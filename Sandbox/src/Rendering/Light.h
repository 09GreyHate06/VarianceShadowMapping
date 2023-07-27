#pragma once
#include "Components.h"

namespace VSM
{
	struct DirectionalLight
	{
		DirectX::XMFLOAT3 color;
		float intensity;
		float ambientIntensity;
		float shadowNearZ;
		float shadowFarZ;

		DirectX::XMFLOAT3 rotation;

		DirectX::XMMATRIX GetLightSpace()
		{
			using namespace DirectX;
			m_tranform.rotation = rotation;
			return XMMatrixLookAtLH(-m_tranform.GetForward() * (shadowFarZ * 0.5f - shadowNearZ), m_tranform.GetForward(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)) * 
				XMMatrixOrthographicLH(40.0f, 40.0f, shadowNearZ, shadowFarZ);
		}

		DirectX::XMVECTOR GetDirection()
		{
			m_tranform.rotation = rotation;
			return m_tranform.GetForward();
		}

	private:
		Transform m_tranform;
	};
}