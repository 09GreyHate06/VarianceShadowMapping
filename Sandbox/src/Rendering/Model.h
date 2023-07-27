#pragma once
#include "Mesh.h"
#include <vector>
#include <assimp/scene.h>
#include <filesystem>

namespace VSM
{
	class Model
	{
	public:
		Model(GDX11::GDX11Context* context, const GDX11::Utils::ResourceLibrary* resLib, const std::string& filename, const Transform& transform);
		Model(Model&) = delete;
		Model& operator=(Model&) = delete;

		void Render();
		void RenderVSM();
		void RenderBasicSMap();

		void ShowImGuiControl(const std::string& label);

	private:
		void ProcessNode(GDX11::GDX11Context* context, const GDX11::Utils::ResourceLibrary* resLib, aiNode* node, const aiScene* scene);
		void ProcessMesh(GDX11::GDX11Context* context, const GDX11::Utils::ResourceLibrary* resLib, aiMesh* mesh, const aiScene* scene);
		std::shared_ptr<GDX11::ShaderResourceView> LoadTextureMap(GDX11::GDX11Context* context, aiMaterial* mat, aiTextureType type);
		DirectX::XMFLOAT3 LoadSolidColorMap(aiMaterial* mat, const char* key, uint32_t type, uint32_t index);

	public:
		Transform transform;

	private:
		std::filesystem::path m_directory;
		std::vector<Mesh> m_meshes;
		std::unordered_map<std::string, std::shared_ptr<GDX11::ShaderResourceView>> m_textures;
	};
}