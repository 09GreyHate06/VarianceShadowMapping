#pragma once
#include "RenderingResource.h"
#include "Texture2D.h"
#include "DepthStencilView.h"
#include <vector>

namespace GDX11
{
	class RenderTargetView;
	using RenderTargetViewArray = std::vector<std::shared_ptr<RenderTargetView>>;


	class RenderTargetView : public RenderingResource<ID3D11RenderTargetView>
	{
	public:
		virtual ~RenderTargetView() = default;
		
		void Clear(float r, float g, float b, float a);
		void Bind(const DepthStencilView* ds) const;


		virtual ID3D11RenderTargetView* GetNative() const override { return m_rtv.Get(); }
		const std::shared_ptr<Texture2D>& GetTexture2D() const { return m_rtTexture; }

		static void Bind(uint32_t numViews, const std::shared_ptr<RenderTargetView>* rtvs, const DepthStencilView* ds);
		static void Bind(const RenderTargetViewArray& rtva, const DepthStencilView* ds);

		static std::shared_ptr<RenderTargetView> Create(GDX11Context* context, const D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc, const std::shared_ptr<Texture2D>& tex);
		static std::shared_ptr<RenderTargetView> Create(GDX11Context* context, ID3D11RenderTargetView* rtv, const std::shared_ptr<Texture2D>& tex);

	private:
		RenderTargetView(GDX11Context* context, const D3D11_RENDER_TARGET_VIEW_DESC& rtvDesc, const std::shared_ptr<Texture2D>& tex);
		RenderTargetView(GDX11Context* context, ID3D11RenderTargetView* rtv, const std::shared_ptr<Texture2D>& tex);
		
		std::shared_ptr<Texture2D> m_rtTexture = nullptr;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv = nullptr;
	};

}