#include "Light.h"
#include "ResourceKeys.h"

using namespace GDX11;
using namespace DirectX;

namespace VSM
{
	DirectionalLight::DirectionalLight(GDX11::GDX11Context* context, const DirectX::XMFLOAT3& color, float intensity, float ambientIntensity, float shadowWidth, float shadowHeight, float shadowNearZ, float shadowFarZ, const DirectX::XMFLOAT3& rotation)
	{
		Set(context, color, intensity, ambientIntensity, shadowWidth, shadowHeight, shadowNearZ, shadowFarZ, rotation);
	}

	DirectionalLight::DirectionalLight()
	{
		Set(nullptr, { 1.0f, 1.0f, 1.0f }, 1.0f, 0.2f, 40.0f, 40.0f, 0.1f, 500.0f, { 50.0f, -30.0f, 0.0f });
	}

	void DirectionalLight::Set(GDX11::GDX11Context* context, const DirectX::XMFLOAT3& color, float intensity, float ambientIntensity, float shadowWidth, float shadowHeight, float shadowNearZ, float shadowFarZ, const DirectX::XMFLOAT3& rotation)
	{
		m_context = context;
		m_color = color;
		m_intensity = intensity;
		m_ambientIntensity = ambientIntensity;
		m_shadowWidth = shadowWidth;
		m_shadowHeight = shadowHeight;
		m_shadowNearZ = shadowNearZ;
		m_shadowFarZ = shadowFarZ;
		m_rotation = rotation;

		UpdateDirection();
		UpdatePosition();
		UpdateLightSpace();

		if (m_context)
			SetOrthoFrustumBuffer();
	}

	void DirectionalLight::SetColor(const DirectX::XMFLOAT3& color)
	{
		m_color = color;
	}

	void DirectionalLight::SetIntensity(float intensity)
	{
		m_intensity = intensity;
	}

	void DirectionalLight::SetAmbientIntensity(float ambientIntensity)
	{
		m_ambientIntensity = ambientIntensity;
	}

	void DirectionalLight::SetShadowWidth(float shadowWidth)
	{
		m_shadowWidth = shadowWidth;

		UpdateLightSpace();
		SetOrthoFrustumBuffer();
	}

	void DirectionalLight::SetShadowHeight(float shadowHeight)
	{
		m_shadowHeight = shadowHeight;

		UpdateLightSpace();
		SetOrthoFrustumBuffer();
	}

	void DirectionalLight::SetShadowNearZ(float shadowNearZ)
	{
		m_shadowNearZ = shadowNearZ;
		UpdateLightSpace();
		if (m_context)
			SetOrthoFrustumBuffer();
	}

	void DirectionalLight::SetShadowFarZ(float shadowFarZ)
	{
		m_shadowFarZ = shadowFarZ;
		UpdateLightSpace();
		if (m_context)
			SetOrthoFrustumBuffer();
	}

	void DirectionalLight::SetRotation(const DirectX::XMFLOAT3& rotation)
	{
		m_rotation = rotation;
		UpdateDirection();
		UpdatePosition();
		UpdateLightSpace();
	}

	const DirectX::XMFLOAT3& DirectionalLight::GetColor() const
	{
		return m_color;
	}

	float DirectionalLight::GetIntensity() const
	{
		return m_intensity;
	}

	float DirectionalLight::GetAmbientIntensity() const
	{
		return m_ambientIntensity;
	}

	float DirectionalLight::GetShadowWidth() const
	{
		return m_shadowWidth;
	}

	float DirectionalLight::GetShadowHeight() const
	{
		return m_shadowHeight;
	}

	float DirectionalLight::GetShadowNearZ() const
	{
		return m_shadowNearZ;
	}

	float DirectionalLight::GetShadowFarZ() const
	{
		return m_shadowFarZ;
	}

	const DirectX::XMFLOAT3& DirectionalLight::GetRotation() const
	{
		return m_rotation;
	}


	DirectX::XMMATRIX DirectionalLight::GetLightSpace()
	{
		return XMLoadFloat4x4(&m_lightSpace);
	}

	DirectX::XMVECTOR DirectionalLight::GetDirection()
	{
		return XMLoadFloat3(&m_direction);
	}

	void DirectionalLight::RenderOrthoFrustum(const GDX11::Utils::ResourceLibrary* m_resLib)
	{
		auto vs = m_resLib->Get<VertexShader>(VS_BASIC);
		auto ps = m_resLib->Get<PixelShader>(PS_BASIC);
		vs->Bind();
		ps->Bind();
		m_resLib->Get<InputLayout>(IL_BASIC)->Bind();

		{
			
			Transform t(m_position, m_rotation, { 1.0f, 1.0f, 1.0f });
			XMFLOAT4X4 transform;
			XMStoreFloat4x4(&transform, XMMatrixTranspose(t.GetTransform()));
			auto cbuf = m_resLib->Get<Buffer>(CB_VS_BASIC_ENTITY);
			cbuf->SetData(&transform);
			cbuf->VSBindAsCBuf(vs->GetResBinding("EntityCBuf"));
		}

		{
			DirectX::XMFLOAT4 color = { m_color.x, m_color.y, m_color.z, 1.0f };
			auto cbuf = m_resLib->Get<Buffer>(CB_PS_BASIC_ENTITY);
			cbuf->SetData(&color);
			cbuf->PSBindAsCBuf(ps->GetResBinding("EntityCBuf"));
		}

		m_vb->BindAsVB();
		m_ib->BindAsIB(DXGI_FORMAT_R32_UINT);
		m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(m_ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));

		m_resLib->Get<VertexShader>(NULL_SHADER)->Bind();
		m_resLib->Get<PixelShader>(NULL_SHADER)->Bind();
	}

	void DirectionalLight::SetOrthoFrustumBuffer()
	{
		auto halfW = m_shadowWidth * 0.5f;
		auto halfH = m_shadowHeight * 0.5f;
		{
			float vertices[] =
			{
				-halfW,  halfH, m_shadowNearZ,
				 halfW,  halfH, m_shadowNearZ,
				 halfW, -halfH, m_shadowNearZ,
				-halfW, -halfH, m_shadowNearZ,

				-halfW,  halfH, m_shadowFarZ,
				 halfW,  halfH, m_shadowFarZ,
				 halfW, -halfH, m_shadowFarZ,
				-halfW, -halfH, m_shadowFarZ,
			};

			D3D11_BUFFER_DESC desc = {};
			desc.ByteWidth = 24 * sizeof(float);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			desc.StructureByteStride = 3 * sizeof(float);
			m_vb = Buffer::Create(m_context, desc, vertices);
		}

		{
			uint32_t indices[] =
			{
				0, 1,
				1, 2,
				2, 3,
				3, 0,

				4, 5,
				5, 6,
				6, 7,
				7, 4,

				0, 4,
				1, 5,
				2, 6,
				3, 7
			};

			D3D11_BUFFER_DESC desc = {};
			desc.ByteWidth = 24 * sizeof(uint32_t);
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;
			desc.StructureByteStride = sizeof(uint32_t);
			m_ib = Buffer::Create(m_context, desc, indices);
		}
	}

	void DirectionalLight::UpdatePosition()
	{
		XMStoreFloat3(&m_position, -GetDirection() * (m_shadowFarZ * 0.5f - m_shadowNearZ));
	}

	void DirectionalLight::UpdateDirection()
	{
		Transform transform({ 0.0f, 0.0f, 0.0f }, m_rotation, { 1.0f, 1.0f, 1.0f });
		XMStoreFloat3(&m_direction, transform.GetForward());
	}

	void DirectionalLight::UpdateLightSpace()
	{
		XMStoreFloat4x4(&m_lightSpace, XMMatrixLookAtLH(XMLoadFloat3(&m_position), GetDirection(), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)) *
			XMMatrixOrthographicLH(m_shadowWidth, m_shadowHeight, m_shadowNearZ, m_shadowFarZ));
	}
}