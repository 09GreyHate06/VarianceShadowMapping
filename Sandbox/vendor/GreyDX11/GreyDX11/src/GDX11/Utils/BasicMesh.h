#pragma once
#include <DirectXMath.h>
#include <array>

namespace GDX11::Utils
{
	struct VertexPUV
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;

		VertexPUV(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& uv)
			: position(position), uv(uv)
		{
		}

		VertexPUV(float p0, float p1, float p2, float uv0, float uv1)
			: VertexPUV({ p0, p1, p2 }, { uv0, uv1 })
		{
		}

		VertexPUV() = default;
	};

	struct VertexPUVN
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;

		VertexPUVN(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT3& normal)
			: position(position), uv(uv), normal(normal)
		{
		}

		VertexPUVN(float p0, float p1, float p2, float uv0, float uv1, float n0, float n1, float n2)
			: VertexPUVN({ p0, p1, p2 }, { uv0, uv1 }, { n0, n1, n2 })
		{
		}

		VertexPUVN() = default;
	};

	struct VertexEx
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
		DirectX::XMFLOAT3 normal;

		VertexEx(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT2& uv, const DirectX::XMFLOAT3& tangent, const DirectX::XMFLOAT3& bitangent, const DirectX::XMFLOAT3& normal)
			: position(position), uv(uv), tangent(tangent), bitangent(bitangent), normal(normal)
		{
		}

		VertexEx(float p0, float p1, float p2, float uv0, float uv1, float t0, float t1, float t2, float bt0, float bt1, float bt2, float n0, float n1, float n2)
			: VertexEx({ p0, p1, p2 }, { uv0, uv1 }, { t0, t1, t2 }, { bt0, bt1, bt2 }, { n0, n1, n2 })
		{
		}

		VertexEx() = default;
	};

	std::array<DirectX::XMFLOAT3, 8> CreateCubeVertices(float minPos = -0.5f, float maxPos = 0.5f);
	std::array<uint32_t, 36> CreateCubeIndices();

	std::array<VertexPUV, 24> CreateCubeVerticesPUV(float minPos = -0.5f, float maxPos = 0.5f, float minUV = 0.0f, float maxUV = 1.0f);
	std::array<uint32_t, 36> CreateCubeIndicesPUV();

	std::array<VertexPUVN, 24> CreateCubeVerticesPUVN(float minPos = -0.5f, float maxPos = 0.5f, float minUV = 0.0f, float maxUV = 1.0f);
	std::array<uint32_t, 36> CreateCubeIndicesPUVN();

	std::array<VertexEx, 36> CreateCubeVerticesEx(float minPos = -0.5f, float maxPos = 0.5f, float minUV = 0.0f, float maxUV = 1.0f);
	std::array<uint32_t, 36> CreateCubeIndicesEx();



	std::array<DirectX::XMFLOAT3, 4> CreatePlaneVertices(float minPos = -0.5f, float maxPos = 0.5f);
	std::array<uint32_t, 6> CreatePlaneIndices();

	std::array<VertexPUV, 4> CreatePlaneVerticesPUV(float minPos = -0.5f, float maxPos = 0.5f, float minUV = 0.0f, float maxUV = 1.0f);
	std::array<uint32_t, 6> CreatePlaneIndicesPUV();

	std::array<VertexPUVN, 4> CreatePlaneVerticesPUVN(float minPos = -0.5f, float maxPos = 0.5f, float minUV = 0.0f, float maxUV = 1.0f);
	std::array<uint32_t, 6> CreatePlaneIndicesPUVN();

	std::array<VertexEx, 6> CreatePlaneVerticesEx(float minPos = -0.5f, float maxPos = 0.5f, float minUV = 0.0f, float maxUV = 1.0f);
	std::array<uint32_t, 6> CreatePlaneIndicesEx();
}