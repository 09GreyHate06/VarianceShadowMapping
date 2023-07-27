#pragma once
#include <GDX11.h>
#include <GDX11/Utils/ResourceLibrary.h>
#include "Components.h"

namespace VSM
{
	class Mesh
	{
	public:
		Mesh(const GDX11::Utils::ResourceLibrary* resLib, const std::shared_ptr<GDX11::Buffer>& vb, const std::shared_ptr<GDX11::Buffer>& ib, D3D11_PRIMITIVE_TOPOLOGY topology, 
			const Transform& transform, const Material& material);
		Mesh() = default;

		void Set(const GDX11::Utils::ResourceLibrary* resLib, const std::shared_ptr<GDX11::Buffer>& vb, const std::shared_ptr<GDX11::Buffer>& ib, D3D11_PRIMITIVE_TOPOLOGY topology,
			const Transform& transform, const Material& material);
		void SetResourceLib(const GDX11::Utils::ResourceLibrary* resLib);
		void SetVB(const std::shared_ptr<GDX11::Buffer>& vb);
		void SetIB(const std::shared_ptr<GDX11::Buffer>& ib);

		void Render();
		void RenderVSM();
		void RenderBasicSMap();

		void ShowImGuiControl(const std::string& label);

	public:
		Transform transform = {};
		Material material = {};
		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	private:
		GDX11::GDX11Context* m_context;
		const GDX11::Utils::ResourceLibrary* m_resLib;

		std::shared_ptr<GDX11::Buffer> m_vb;
		std::shared_ptr<GDX11::Buffer> m_ib;
	};
}