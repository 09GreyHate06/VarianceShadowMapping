#pragma once
#include "Components.h"
#include <GDX11/Utils/ResourceLibrary.h>

namespace VSM
{
	class DirectionalLight
	{
	public:
		DirectionalLight(GDX11::GDX11Context* context, const DirectX::XMFLOAT3& color, float intensity, float ambientIntensity, float shadowWidth, float shadowHeight, float shadowNearZ, float shadowFarZ, const DirectX::XMFLOAT3& rotation);
		DirectionalLight();

		void Set(GDX11::GDX11Context* context, const DirectX::XMFLOAT3& color, float intensity, float ambientIntensity, float shadowWidth, float shadowHeight, float shadowNearZ, float shadowFarZ, const DirectX::XMFLOAT3& rotation);
		void SetColor(const DirectX::XMFLOAT3& color);
		void SetIntensity(float intensity);
		void SetAmbientIntensity(float ambientIntensity);
		void SetShadowWidth(float shadowWidth);
		void SetShadowHeight(float shadowHeight);
		void SetShadowNearZ(float shadowNearZ);
		void SetShadowFarZ(float shadowFarZ);
		void SetRotation(const DirectX::XMFLOAT3& rotation);

		const DirectX::XMFLOAT3& GetColor() const;
		float GetIntensity() const;
		float GetAmbientIntensity() const;
		float GetShadowWidth() const;
		float GetShadowHeight() const;
		float GetShadowNearZ() const;
		float GetShadowFarZ() const;
		const DirectX::XMFLOAT3& GetRotation() const;

		DirectX::XMMATRIX GetLightSpace();
		DirectX::XMVECTOR GetDirection();

		void RenderOrthoFrustum(const GDX11::Utils::ResourceLibrary* m_resLib);

	private:
		void SetOrthoFrustumBuffer();
		void UpdateLightSpace();
		void UpdateDirection();
		void UpdatePosition();

		GDX11::GDX11Context* m_context;
		DirectX::XMFLOAT3 m_color;
		float m_intensity;
		float m_ambientIntensity;
		float m_shadowWidth;
		float m_shadowHeight;
		float m_shadowNearZ;
		float m_shadowFarZ;

		DirectX::XMFLOAT3 m_rotation;
		DirectX::XMFLOAT3 m_position; 
		DirectX::XMFLOAT4X4 m_lightSpace;
		DirectX::XMFLOAT3 m_direction;

		std::shared_ptr<GDX11::Buffer> m_vb;
		std::shared_ptr<GDX11::Buffer> m_ib;
	};
}