#pragma once
#include <DirectXMath.h>
#include <GDX11.h>

namespace VSM
{
	struct Transform
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 scale;

		Transform() = default;
		Transform(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation, const DirectX::XMFLOAT3& scale)
			: position(position), rotation(rotation), scale(scale)
		{
		}

		DirectX::XMVECTOR GetOrientation()
		{
			using namespace DirectX;
			return XMQuaternionRotationRollPitchYaw(XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z));
		}

		DirectX::XMVECTOR GetRight()
		{
			using namespace DirectX;
			return XMVector3Rotate(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), GetOrientation());
		}

		DirectX::XMVECTOR GetUp()
		{
			using namespace DirectX;
			return XMVector3Rotate(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), GetOrientation());
		}

		DirectX::XMVECTOR GetForward()
		{
			using namespace DirectX;
			return XMVector3Rotate(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), GetOrientation());
		}

		DirectX::XMMATRIX GetTransform()
		{
			using namespace DirectX;
			return XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationQuaternion(GetOrientation()) * XMMatrixTranslation(position.x, position.y, position.z);
		}
	};

	struct Material
	{
		std::shared_ptr<GDX11::ShaderResourceView> diffuseMap;
		std::shared_ptr<GDX11::ShaderResourceView> normalMap;
		std::shared_ptr<GDX11::ShaderResourceView> depthMap; // for parallax mapping
		std::shared_ptr<GDX11::SamplerState> samplerState;

		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 tiling;
		float shininess;
		float depthMapScale; // for parallax mapping
	};
}