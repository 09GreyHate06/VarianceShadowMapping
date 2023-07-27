#include "BasicMesh.h"

using namespace DirectX;

namespace GDX11::Utils
{
	struct TangentSpace
	{
		DirectX::XMFLOAT3 t, b, n;
	};

	TangentSpace CalcTangentSpace(const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1, const DirectX::XMFLOAT3& p2,
		const DirectX::XMFLOAT2& uv0, const DirectX::XMFLOAT2& uv1, const DirectX::XMFLOAT2& uv2)
	{
		// https://learnopengl.com/Advanced-Lighting/Normal-Mapping
		// [edge] = [deltaUV][tb]
		// [tb] = [deltaUV]^-1 [edge]

		XMVECTOR xmP0 = XMLoadFloat3(&p0);
		XMVECTOR xmP1 = XMLoadFloat3(&p1);
		XMVECTOR xmP2 = XMLoadFloat3(&p2);

		XMVECTOR xmUV0 = XMLoadFloat2(&uv0);
		XMVECTOR xmUV1 = XMLoadFloat2(&uv1);
		XMVECTOR xmUV2 = XMLoadFloat2(&uv2);

		XMVECTOR xmEdge0 = xmP1 - xmP0;
		XMVECTOR xmEdge1 = xmP2 - xmP0;

		XMVECTOR xmDeltaUV0 = xmUV1 - xmUV0;
		XMVECTOR xmDeltaUV1 = xmUV2 - xmUV0;

		XMFLOAT3 edge0;
		XMFLOAT3 edge1;
		XMFLOAT2 deltaUV0;
		XMFLOAT2 deltaUV1;
		XMStoreFloat3(&edge0, xmEdge0);
		XMStoreFloat3(&edge1, xmEdge1);
		XMStoreFloat2(&deltaUV0, xmDeltaUV0);
		XMStoreFloat2(&deltaUV1, xmDeltaUV1);

		float deltaUVInvDet = 1.0f / (deltaUV0.x * deltaUV1.y - deltaUV0.y * deltaUV1.x);

		XMFLOAT3 tangent = {};
		tangent.x = deltaUVInvDet * (deltaUV1.y * edge0.x - deltaUV0.y * edge1.x);
		tangent.y = deltaUVInvDet * (deltaUV1.y * edge0.y - deltaUV0.y * edge1.y);
		tangent.z = deltaUVInvDet * (deltaUV1.y * edge0.z - deltaUV0.y * edge1.z);

		XMFLOAT3 bitangent = {};
		bitangent.x = deltaUVInvDet * (-deltaUV1.x * edge0.x + deltaUV0.x * edge1.x);
		bitangent.y = deltaUVInvDet * (-deltaUV1.x * edge0.y + deltaUV0.x * edge1.y);
		bitangent.z = deltaUVInvDet * (-deltaUV1.x * edge0.z + deltaUV0.x * edge1.z);

		XMFLOAT3 normal;
		XMStoreFloat3(&normal, XMVector3Cross(XMLoadFloat3(&tangent), XMLoadFloat3(&bitangent)));

		return { tangent, bitangent, normal };
	}

	std::array<DirectX::XMFLOAT3, 8> CreateCubeVertices(float minPos, float maxPos)
	{
		return std::array<XMFLOAT3, 8>{
			// front plane
			XMFLOAT3(minPos, maxPos, minPos),
			XMFLOAT3(maxPos, maxPos, minPos),
			XMFLOAT3(maxPos, minPos, minPos),
			XMFLOAT3(minPos, minPos, minPos),

			// back plane
			XMFLOAT3(maxPos, maxPos, maxPos),
			XMFLOAT3(minPos, maxPos, maxPos),
			XMFLOAT3(minPos, minPos, maxPos),
			XMFLOAT3(maxPos, minPos, maxPos)
		};
	}

	std::array<uint32_t, 36> CreateCubeIndices()
	{
		return std::array<uint32_t, 36>{
			// front
			0, 1, 2,
			2, 3, 0,

			// back
			4, 5, 6,
			6, 7, 4,

			// right
			1, 4, 7,
			7, 2, 1,

			// left
			5, 0, 3,
			3, 6, 5,

			// top
			5, 4, 1,
			1, 0, 5,

			// bottom
			3, 2, 7,
			7, 6, 3
		};
	}

	std::array<VertexPUV, 24> CreateCubeVerticesPUV(float minPos, float maxPos, float minUV, float maxUV)
	{
		return std::array<VertexPUV, 24>{
			//  back		             						  
			VertexPUV(maxPos, maxPos, maxPos, minUV, minUV),
			VertexPUV(minPos, maxPos, maxPos, maxUV, minUV),
			VertexPUV(minPos, minPos, maxPos, maxUV, maxUV),
			VertexPUV(maxPos, minPos, maxPos, minUV, maxUV),
												  
			// front							  
			VertexPUV(minPos, maxPos, minPos, minUV, minUV),
			VertexPUV(maxPos, maxPos, minPos, maxUV, minUV),
			VertexPUV(maxPos, minPos, minPos, maxUV, maxUV),
			VertexPUV(minPos, minPos, minPos, minUV, maxUV),

			// bottom			     	      					 
			VertexPUV(minPos, minPos, minPos, minUV, minUV),
			VertexPUV(maxPos, minPos, minPos, maxUV, minUV),
			VertexPUV(maxPos, minPos, maxPos, maxUV, maxUV),
			VertexPUV(minPos, minPos, maxPos, minUV, maxUV),

			// top		      	     					  
			VertexPUV(minPos, maxPos, maxPos, minUV, minUV),
			VertexPUV(maxPos, maxPos, maxPos, maxUV, minUV),
			VertexPUV(maxPos, maxPos, minPos, maxUV, maxUV),
			VertexPUV(minPos, maxPos, minPos, minUV, maxUV),

			// left				     	      
			VertexPUV(minPos, maxPos, maxPos, minUV, minUV),
			VertexPUV(minPos, maxPos, minPos, maxUV, minUV),
			VertexPUV(minPos, minPos, minPos, maxUV, maxUV),
			VertexPUV(minPos, minPos, maxPos, minUV, maxUV),

			// right			             	  
			VertexPUV(maxPos, maxPos, minPos, minUV, minUV),
			VertexPUV(maxPos, maxPos, maxPos, maxUV, minUV),
			VertexPUV(maxPos, minPos, maxPos, maxUV, maxUV),
			VertexPUV(maxPos, minPos, minPos, minUV, maxUV),
		};
	}

	std::array<uint32_t, 36> CreateCubeIndicesPUV()
	{
		return std::array<uint32_t, 36>
		{
			// back
			0, 1, 2,
			2, 3, 0,

			// front
			4, 5, 6,
			6, 7, 4,

			// left
			8, 9, 10,
			10, 11, 8,

			// right
			12, 13, 14,
			14, 15, 12,

			// bottom
			16, 17, 18,
			18, 19, 16,

			// top
			20, 21, 22,
			22, 23, 20
		};
	}

	std::array<VertexPUVN, 24> CreateCubeVerticesPUVN(float minPos, float maxPos, float minUV, float maxUV)
	{
		return std::array<VertexPUVN, 24>{
			//  back		             						  
			VertexPUVN(maxPos, maxPos, maxPos, minUV, minUV, 0.0f, 0.0f, 1.0f),
			VertexPUVN(minPos, maxPos, maxPos, maxUV, minUV, 0.0f, 0.0f, 1.0f),
			VertexPUVN(minPos, minPos, maxPos, maxUV, maxUV, 0.0f, 0.0f, 1.0f),
			VertexPUVN(maxPos, minPos, maxPos, minUV, maxUV, 0.0f, 0.0f, 1.0f),

			// front							  
			VertexPUVN(minPos, maxPos, minPos, minUV, minUV, 0.0f, 0.0f, -1.0f),
			VertexPUVN(maxPos, maxPos, minPos, maxUV, minUV, 0.0f, 0.0f, -1.0f),
			VertexPUVN(maxPos, minPos, minPos, maxUV, maxUV, 0.0f, 0.0f, -1.0f),
			VertexPUVN(minPos, minPos, minPos, minUV, maxUV, 0.0f, 0.0f, -1.0f),

			// bottom			     	      				     	 
			VertexPUVN(minPos, minPos, minPos, minUV, minUV, 0.0f, -1.0f, 0.0),
			VertexPUVN(maxPos, minPos, minPos, maxUV, minUV, 0.0f, -1.0f, 0.0),
			VertexPUVN(maxPos, minPos, maxPos, maxUV, maxUV, 0.0f, -1.0f, 0.0),
			VertexPUVN(minPos, minPos, maxPos, minUV, maxUV, 0.0f, -1.0f, 0.0),

			// top		      	     					  				  
			VertexPUVN(minPos, maxPos, maxPos, minUV, minUV, 0.0f, 1.0f, 0.0f),
			VertexPUVN(maxPos, maxPos, maxPos, maxUV, minUV, 0.0f, 1.0f, 0.0f),
			VertexPUVN(maxPos, maxPos, minPos, maxUV, maxUV, 0.0f, 1.0f, 0.0f),
			VertexPUVN(minPos, maxPos, minPos, minUV, maxUV, 0.0f, 1.0f, 0.0f),

			// left				     	      			      
			VertexPUVN(minPos, maxPos, maxPos, minUV, minUV, -1.0f, 0.0f, 0.0),
			VertexPUVN(minPos, maxPos, minPos, maxUV, minUV, -1.0f, 0.0f, 0.0),
			VertexPUVN(minPos, minPos, minPos, maxUV, maxUV, -1.0f, 0.0f, 0.0),
			VertexPUVN(minPos, minPos, maxPos, minUV, maxUV, -1.0f, 0.0f, 0.0),

			// right			             	  		      
			VertexPUVN(maxPos, maxPos, minPos, minUV, minUV, 1.0f, 0.0f, 0.0f),
			VertexPUVN(maxPos, maxPos, maxPos, maxUV, minUV, 1.0f, 0.0f, 0.0f),
			VertexPUVN(maxPos, minPos, maxPos, maxUV, maxUV, 1.0f, 0.0f, 0.0f),
			VertexPUVN(maxPos, minPos, minPos, minUV, maxUV, 1.0f, 0.0f, 0.0f),
		};
	}

	std::array<uint32_t, 36> CreateCubeIndicesPUVN()
	{
		return CreateCubeIndicesPUV();
	}

	std::array<VertexEx, 36> CreateCubeVerticesEx(float minPos, float maxPos, float minUV, float maxUV)
	{
		XMFLOAT2 uv0 = { minUV, minUV };
		XMFLOAT2 uv1 = { maxUV, minUV };
		XMFLOAT2 uv2 = { maxUV, maxUV };
		XMFLOAT2 uv3 = { minUV, maxUV };

		// front pos
		XMFLOAT3 frontP0 = { minPos, maxPos, minPos };
		XMFLOAT3 frontP1 = { maxPos, maxPos, minPos };
		XMFLOAT3 frontP2 = { maxPos, minPos, minPos };
		XMFLOAT3 frontP3 = { minPos, minPos, minPos };

		TangentSpace frontTS012 = CalcTangentSpace(frontP0, frontP1, frontP2, uv0, uv1, uv2);
		TangentSpace frontTS230 = CalcTangentSpace(frontP2, frontP3, frontP0, uv2, uv3, uv0);

		// back pos
		XMFLOAT3 backP0 = { maxPos, maxPos, maxPos };
		XMFLOAT3 backP1 = { minPos, maxPos, maxPos };
		XMFLOAT3 backP2 = { minPos, minPos, maxPos };
		XMFLOAT3 backP3 = { maxPos, minPos, maxPos };

		TangentSpace backTS012 = CalcTangentSpace(backP0, backP1, backP2, uv0, uv1, uv2);
		TangentSpace backTS230 = CalcTangentSpace(backP2, backP3, backP0, uv2, uv3, uv0);

		// top pos
		XMFLOAT3 topP0 = { minPos, maxPos, maxPos };
		XMFLOAT3 topP1 = { maxPos, maxPos, maxPos };
		XMFLOAT3 topP2 = { maxPos, maxPos, minPos };
		XMFLOAT3 topP3 = { minPos, maxPos, minPos };

		TangentSpace topTS012 = CalcTangentSpace(topP0, topP1, topP2, uv0, uv1, uv2);
		TangentSpace topTS230 = CalcTangentSpace(topP2, topP3, topP0, uv2, uv3, uv0);

		// bottom pos
		XMFLOAT3 bottomP0 = { minPos, minPos, minPos };
		XMFLOAT3 bottomP1 = { maxPos, minPos, minPos };
		XMFLOAT3 bottomP2 = { maxPos, minPos, maxPos };
		XMFLOAT3 bottomP3 = { minPos, minPos, maxPos };

		TangentSpace bottomTS012 = CalcTangentSpace(bottomP0, bottomP1, bottomP2, uv0, uv1, uv2);
		TangentSpace bottomTS230 = CalcTangentSpace(bottomP2, bottomP3, bottomP0, uv2, uv3, uv0);

		// right pos
		XMFLOAT3 rightP0 = { maxPos, maxPos, minPos };
		XMFLOAT3 rightP1 = { maxPos, maxPos, maxPos };
		XMFLOAT3 rightP2 = { maxPos, minPos, maxPos };
		XMFLOAT3 rightP3 = { maxPos, minPos, minPos };

		TangentSpace rightTS012 = CalcTangentSpace(rightP0, rightP1, rightP2, uv0, uv1, uv2);
		TangentSpace rightTS230 = CalcTangentSpace(rightP2, rightP3, rightP0, uv2, uv3, uv0);

		// left pos
		XMFLOAT3 leftP0 = { minPos, maxPos, maxPos };
		XMFLOAT3 leftP1 = { minPos, maxPos, minPos };
		XMFLOAT3 leftP2 = { minPos, minPos, minPos };
		XMFLOAT3 leftP3 = { minPos, minPos, maxPos };

		TangentSpace leftTS012 = CalcTangentSpace(leftP0, leftP1, leftP2, uv0, uv1, uv2);
		TangentSpace leftTS230 = CalcTangentSpace(leftP2, leftP3, leftP0, uv2, uv3, uv0);

		return std::array<VertexEx, 36>
		{
			// front 
			VertexEx(frontP0, uv0, frontTS012.t, frontTS012.b, frontTS012.n),
			VertexEx(frontP1, uv1, frontTS012.t, frontTS012.b, frontTS012.n),
			VertexEx(frontP2, uv2, frontTS012.t, frontTS012.b, frontTS012.n),
			VertexEx(frontP2, uv2, frontTS230.t, frontTS230.b, frontTS230.n),
			VertexEx(frontP3, uv3, frontTS230.t, frontTS230.b, frontTS230.n),
			VertexEx(frontP0, uv0, frontTS230.t, frontTS230.b, frontTS230.n),
			
			// back
			VertexEx(backP0, uv0, backTS012.t, backTS012.b, backTS012.n),
			VertexEx(backP1, uv1, backTS012.t, backTS012.b, backTS012.n),
			VertexEx(backP2, uv2, backTS012.t, backTS012.b, backTS012.n),
			VertexEx(backP2, uv2, backTS230.t, backTS230.b, backTS230.n),
			VertexEx(backP3, uv3, backTS230.t, backTS230.b, backTS230.n),
			VertexEx(backP0, uv0, backTS230.t, backTS230.b, backTS230.n),
			
			// top 
			VertexEx(topP0, uv0, topTS012.t, topTS012.b, topTS012.n),
			VertexEx(topP1, uv1, topTS012.t, topTS012.b, topTS012.n),
			VertexEx(topP2, uv2, topTS012.t, topTS012.b, topTS012.n),
			VertexEx(topP2, uv2, topTS230.t, topTS230.b, topTS230.n),
			VertexEx(topP3, uv3, topTS230.t, topTS230.b, topTS230.n),
			VertexEx(topP0, uv0, topTS230.t, topTS230.b, topTS230.n),
			
			// bottom
			VertexEx(bottomP0, uv0, bottomTS012.t, bottomTS012.b, bottomTS012.n),
			VertexEx(bottomP1, uv1, bottomTS012.t, bottomTS012.b, bottomTS012.n),
			VertexEx(bottomP2, uv2, bottomTS012.t, bottomTS012.b, bottomTS012.n),
			VertexEx(bottomP2, uv2, bottomTS230.t, bottomTS230.b, bottomTS230.n),
			VertexEx(bottomP3, uv3, bottomTS230.t, bottomTS230.b, bottomTS230.n),
			VertexEx(bottomP0, uv0, bottomTS230.t, bottomTS230.b, bottomTS230.n),
			
			// right
			VertexEx(rightP0, uv0, rightTS012.t, rightTS012.b, rightTS012.n),
			VertexEx(rightP1, uv1, rightTS012.t, rightTS012.b, rightTS012.n),
			VertexEx(rightP2, uv2, rightTS012.t, rightTS012.b, rightTS012.n),
			VertexEx(rightP2, uv2, rightTS230.t, rightTS230.b, rightTS230.n),
			VertexEx(rightP3, uv3, rightTS230.t, rightTS230.b, rightTS230.n),
			VertexEx(rightP0, uv0, rightTS230.t, rightTS230.b, rightTS230.n),
			
			// left
			VertexEx(leftP0, uv0, leftTS012.t, leftTS012.b, leftTS012.n),
			VertexEx(leftP1, uv1, leftTS012.t, leftTS012.b, leftTS012.n),
			VertexEx(leftP2, uv2, leftTS012.t, leftTS012.b, leftTS012.n),
			VertexEx(leftP2, uv2, leftTS230.t, leftTS230.b, leftTS230.n),
			VertexEx(leftP3, uv3, leftTS230.t, leftTS230.b, leftTS230.n),
			VertexEx(leftP0, uv0, leftTS230.t, leftTS230.b, leftTS230.n)
		};
	}

	std::array<uint32_t, 36> CreateCubeIndicesEx()
	{
		std::array<uint32_t, 36> ind = {};
		for (int i = 0; i < 36; i++)
			ind[i] = i;

		return ind;
	}




	std::array<DirectX::XMFLOAT3, 4> CreatePlaneVertices(float minPos, float maxPos)
	{
		return std::array<DirectX::XMFLOAT3, 4>{
			XMFLOAT3(minPos, 0.0f, maxPos),
			XMFLOAT3(maxPos, 0.0f, maxPos),
			XMFLOAT3(maxPos, 0.0f, minPos),
			XMFLOAT3(minPos, 0.0f, minPos),
		};
	}

	std::array<uint32_t, 6> CreatePlaneIndices()
	{
		return std::array<uint32_t, 6>{
			0, 1, 2,
			2, 3, 0
		};
	}

	std::array<VertexPUV, 4> CreatePlaneVerticesPUV(float minPos, float maxPos, float minUV, float maxUV)
	{
		return std::array<VertexPUV, 4>{
			VertexPUV(minPos, 0.0f, maxPos, minUV, minUV),
			VertexPUV(maxPos, 0.0f, maxPos, maxUV, minUV),
			VertexPUV(maxPos, 0.0f, minPos, maxUV, maxUV),
			VertexPUV(minPos, 0.0f, minPos, minUV, maxUV),
		};
	}

	std::array<uint32_t, 6> CreatePlaneIndicesPUV()
	{
		return CreatePlaneIndices();
	}

	std::array<VertexPUVN, 4> CreatePlaneVerticesPUVN(float minPos, float maxPos, float minUV, float maxUV)
	{
		return std::array<VertexPUVN, 4>{
			VertexPUVN(minPos, 0.0f, maxPos, minUV, minUV, 0.0f, 1.0f, 0.0f),
			VertexPUVN(maxPos, 0.0f, maxPos, maxUV, minUV, 0.0f, 1.0f, 0.0f),
			VertexPUVN(maxPos, 0.0f, minPos, maxUV, maxUV, 0.0f, 1.0f, 0.0f),
			VertexPUVN(minPos, 0.0f, minPos, minUV, maxUV, 0.0f, 1.0f, 0.0f),
		};
	}

	std::array<uint32_t, 6> CreatePlaneIndicesPUVN()
	{
		return CreatePlaneIndices();
	}

	std::array<VertexEx, 6> CreatePlaneVerticesEx(float minPos, float maxPos, float minUV, float maxUV)
	{
		XMFLOAT2 uv0 = { minUV, minUV };
		XMFLOAT2 uv1 = { maxUV, minUV };
		XMFLOAT2 uv2 = { maxUV, maxUV };
		XMFLOAT2 uv3 = { minUV, maxUV };

		XMFLOAT3 p0 = { minPos, 0.0f, maxPos };
		XMFLOAT3 p1 = { maxPos, 0.0f, maxPos };
		XMFLOAT3 p2 = { maxPos, 0.0f, minPos };
		XMFLOAT3 p3 = { minPos, 0.0f, minPos };

		auto ts0 = CalcTangentSpace(p0, p1, p2, uv0, uv1, uv2);
		auto ts1 = CalcTangentSpace(p2, p3, p0, uv2, uv3, uv0);

		return std::array<VertexEx, 6> {
			VertexEx(p0, uv0, ts0.t, ts0.b, ts0.n),
			VertexEx(p1, uv1, ts0.t, ts0.b, ts0.n),
			VertexEx(p2, uv2, ts0.t, ts0.b, ts0.n),
			VertexEx(p2, uv2, ts1.t, ts1.b, ts1.n),
			VertexEx(p3, uv3, ts1.t, ts1.b, ts1.n),
			VertexEx(p0, uv0, ts1.t, ts1.b, ts1.n),
		};
	}

	std::array<uint32_t, 6> CreatePlaneIndicesEx()
	{
		std::array<uint32_t, 6> ind = {};
		for (int i = 0; i < 6; i++)
			ind[i] = i;

		return ind;
	}
}