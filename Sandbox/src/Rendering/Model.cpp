#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <GDX11/Utils/BasicMesh.h>
#include <GDX11/Utils/Image.h>
#include <imgui.h>

#include "ResourceKeys.h"

using namespace GDX11;
using namespace DirectX;

namespace VSM
{
	Model::Model(GDX11::GDX11Context* context, const GDX11::Utils::ResourceLibrary* resLib, const std::string& filename, const Transform& transform)
		: m_directory(filename), transform(transform)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename,
			aiProcess_JoinIdenticalVertices | aiProcess_MakeLeftHanded | aiProcess_Triangulate | aiProcess_FlipWindingOrder |
			aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_CalcTangentSpace);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			GDX11_ASSERT("Failed to load model: {0}'\n'[Description] {1}", filename, importer.GetErrorString());

		m_directory = m_directory.lexically_normal();
		m_directory = m_directory.parent_path();

		ProcessNode(context, resLib, scene->mRootNode, scene);
	}

	void Model::Render()
	{
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].transform = transform;
			m_meshes[i].Render();
		}
	}

	void Model::RenderBasicSMap()
	{
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].transform = transform;
			m_meshes[i].RenderBasicSMap();
		}
	}

	void Model::ShowImGuiControl(const std::string& label)
	{

		ImGui::Begin(label.c_str());
		ImGui::PushItemWidth(210.0f);
		ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
		ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
		ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
		ImGui::PopItemWidth();
		ImGui::End();

	}

	void Model::RenderVSM()
	{
		for (int i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].transform = transform;
			m_meshes[i].RenderVSM();
		}
	}

	void Model::ProcessNode(GDX11::GDX11Context* context, const GDX11::Utils::ResourceLibrary* resLib, aiNode* node, const aiScene* scene)
	{
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(context, resLib, mesh, scene);
		}
		for (uint32_t i = 0; i < node->mNumChildren; i++)
			ProcessNode(context, resLib, node->mChildren[i], scene);
	}

	void Model::ProcessMesh(GDX11::GDX11Context* context, const GDX11::Utils::ResourceLibrary* resLib, aiMesh* mesh, const aiScene* scene)
	{
		// mesh buffers
		std::shared_ptr<Buffer> vb;
		std::shared_ptr<Buffer> ib;
		{
			std::vector<Utils::VertexEx> vertices;
			std::vector<uint32_t> indices;
			vertices.reserve(mesh->mNumVertices);
			for (uint32_t i = 0; i < mesh->mNumVertices; i++)
			{
				Utils::VertexEx vertex = {};
				vertex.position = *reinterpret_cast<XMFLOAT3*>(&mesh->mVertices[i]);
				vertex.normal = *reinterpret_cast<XMFLOAT3*>(&mesh->mNormals[i]);

				if (mesh->mTextureCoords[0])
					vertex.uv = *reinterpret_cast<XMFLOAT2*>(&mesh->mTextureCoords[0][i]);
				else
					vertex.uv = { 0.0f, 0.0f };

				if (mesh->HasTangentsAndBitangents())
				{
					vertex.tangent = *reinterpret_cast<XMFLOAT3*>(&mesh->mTangents[i]);
					vertex.bitangent = *reinterpret_cast<XMFLOAT3*>(&mesh->mBitangents[i]);
				}
				else
				{
					vertex.tangent = { 1.0f, 0.0f, 0.0f };
					vertex.bitangent = { 0.0f, 1.0f, 0.0f };
				}

				vertices.emplace_back(vertex);
			}

			indices.reserve(mesh->mNumFaces * 3);
			for (uint32_t i = 0; i < mesh->mNumFaces; i++)
			{
				const aiFace& face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; j++)
					indices.emplace_back(face.mIndices[j]);
			}

			
			{
				D3D11_BUFFER_DESC desc = {};
				desc.ByteWidth = (uint32_t)vertices.size() * sizeof(Utils::VertexEx);
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = sizeof(Utils::VertexEx);
				vb = Buffer::Create(context, desc, vertices.data());

				desc = {};
				desc.ByteWidth = (uint32_t)indices.size() * sizeof(uint32_t);
				desc.Usage = D3D11_USAGE_DEFAULT;
				desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = sizeof(uint32_t);
				ib = Buffer::Create(context, desc, indices.data());
			}
		}

		// material
		Material material = {};
		{
			std::shared_ptr<ShaderResourceView> diffuseMap = nullptr;
			std::shared_ptr<ShaderResourceView> normalMap = nullptr;
			std::shared_ptr<ShaderResourceView> depthMap = nullptr;
			XMFLOAT3 diffuseCol = { 1.0f, 1.0f, 1.0f };
			float shininess = 32.0f;
			float depthMapScale = 0.1f;

			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
				diffuseMap = LoadTextureMap(context, mat, aiTextureType_DIFFUSE);
				normalMap = LoadTextureMap(context, mat, aiTextureType_HEIGHT);
				depthMap = LoadTextureMap(context, mat, aiTextureType_DISPLACEMENT);
				if (!normalMap)
					normalMap = LoadTextureMap(context, mat, aiTextureType_NORMALS);

				diffuseCol = LoadSolidColorMap(mat, AI_MATKEY_COLOR_DIFFUSE);

				if (aiGetMaterialFloat(mat, AI_MATKEY_SHININESS, &shininess) != AI_SUCCESS)
					shininess = 32.0f;

				if (aiGetMaterialFloat(mat, AI_MATKEY_BUMPSCALING, &depthMapScale) != AI_SUCCESS)
					depthMapScale = 0.1f;
			}
			
			material.color = XMFLOAT4(diffuseCol.x, diffuseCol.y, diffuseCol.z, 1.0f);
			material.shininess = shininess;
			material.tiling = { 1.0f, 1.0f };
			material.diffuseMap = diffuseMap ? diffuseMap : resLib->Get<ShaderResourceView>(RES_DEFAULT);
			material.normalMap = normalMap;
			material.depthMap = depthMap;
			material.samplerState = resLib->Get<SamplerState>(SS_ANISO_WRAP);
			material.depthMapScale = depthMapScale;
		}

		m_meshes.emplace_back(resLib, vb, ib, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, transform, material);
	}

	std::shared_ptr<GDX11::ShaderResourceView> Model::LoadTextureMap(GDX11::GDX11Context* context, aiMaterial* mat, aiTextureType type)
	{
		if (mat->GetTextureCount(type) <= 0) return nullptr;

		aiString pathBuffer;
		mat->GetTexture(type, 0, &pathBuffer);
		std::string filename = (m_directory / pathBuffer.C_Str()).lexically_normal().string();

		if (m_textures.find(filename) != m_textures.end())
			return m_textures[filename];

		auto image = Utils::LoadImageFile(filename, false, 4);

		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = image.width;
		texDesc.Height = image.height;
		texDesc.MipLevels = 0;
		texDesc.ArraySize = 1;
		texDesc.Format = type == aiTextureType_DIFFUSE ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		auto tex = ShaderResourceView::Create(context, srvDesc, Texture2D::Create(context, texDesc, image.pixels));

		Utils::FreeImageData(&image);

		m_textures[filename] = tex;
		return tex;
	}

	DirectX::XMFLOAT3 Model::LoadSolidColorMap(aiMaterial* mat, const char* key, uint32_t type, uint32_t index)
	{
		aiColor3D color;
		if (mat->Get(key, type, index, color) == AI_SUCCESS)
			return { color.r, color.g, color.b };
		return { 1.0f, 1.0f, 1.0f };
	}
}