#include "Mesh.h"
#include "ResourceKeys.h"
#include "ShaderCBuf.h"
#include <imgui.h>

using namespace GDX11;
using namespace DirectX;

namespace VSM
{
	Mesh::Mesh(const GDX11::Utils::ResourceLibrary* resLib, const std::shared_ptr<GDX11::Buffer>& vb, const std::shared_ptr<GDX11::Buffer>& ib, D3D11_PRIMITIVE_TOPOLOGY topology, 
		const Transform& transform, const Material& material)
		: m_context(vb->GetContext()), m_resLib(resLib), m_vb(vb), m_ib(ib), topology(topology), transform(transform), material(material)
	{
	}

    void Mesh::Set(const GDX11::Utils::ResourceLibrary* resLib, const std::shared_ptr<GDX11::Buffer>& vb, const std::shared_ptr<GDX11::Buffer>& ib, D3D11_PRIMITIVE_TOPOLOGY topology, const Transform& transform, const Material& material)
    {
        m_context = vb->GetContext();
        m_resLib = resLib;
        m_vb = vb;
        m_ib = ib;
        this->topology = topology;
        this->transform = transform;
        this->material = material;
    }

    void Mesh::SetResourceLib(const GDX11::Utils::ResourceLibrary* resLib)
    {
        m_resLib = resLib;
    }

    void Mesh::SetVB(const std::shared_ptr<GDX11::Buffer>& vb)
    {
        m_vb = vb;
        m_context = vb->GetContext();
    }

    void Mesh::SetIB(const std::shared_ptr<GDX11::Buffer>& ib)
    {
        m_ib = ib;
        m_context = m_ib->GetContext();
    }

    void Mesh::Render()
	{
        auto vs = m_resLib->Get<VertexShader>(VS_PHONG);
        auto ps = m_resLib->Get<PixelShader>(PS_PHONG);
        vs->Bind();
        ps->Bind();
        m_resLib->Get<InputLayout>(IL_PHONG)->Bind();

        XMMATRIX xmTransform = transform.GetTransform();

        {
            PhongCBuf::VSEntity data = {};
            XMStoreFloat4x4(&data.transform, XMMatrixTranspose(xmTransform));
            XMStoreFloat4x4(&data.normalMatrix, XMMatrixInverse(nullptr, xmTransform));
            auto cbuf = m_resLib->Get<Buffer>(CB_VS_PHONG_ENTITY);
            cbuf->SetData(&data);
            cbuf->VSBindAsCBuf(vs->GetResBinding("EntityCBuf"));
        }

        {
            PhongCBuf::PSEntity data = {};
            data.mat.color = material.color;
            data.mat.tiling = material.tiling;
            data.mat.shininess = material.shininess;
            data.mat.enableNormalMapping = material.normalMap ? TRUE : FALSE;
            data.mat.enableParallaxMapping = material.normalMap && material.depthMap ? TRUE : FALSE;
            data.mat.depthMapScale = material.depthMapScale;
            auto cbuf = m_resLib->Get<Buffer>(CB_PS_PHONG_ENTITY);
            cbuf->SetData(&data);
            cbuf->PSBindAsCBuf(ps->GetResBinding("EntityCBuf"));
        }

        material.diffuseMap->PSBind(ps->GetResBinding("diffuseMap"));
        if (material.normalMap)
        {
            material.normalMap->PSBind(ps->GetResBinding("normalMap"));
            if (material.depthMap)
                material.depthMap->PSBind(ps->GetResBinding("depthMap"));
        }

        material.samplerState->PSBind(ps->GetResBinding("matSampler"));

        m_vb->BindAsVB();
        m_ib->BindAsIB(DXGI_FORMAT_R32_UINT);
        m_context->GetDeviceContext()->IASetPrimitiveTopology(topology);
        GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(m_ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));

        m_resLib->Get<VertexShader>(NULL_SHADER)->Bind();
        m_resLib->Get<PixelShader>(NULL_SHADER)->Bind();
	}

    void Mesh::RenderVSM()
    {
        auto vs = m_resLib->Get<VertexShader>(VS_BASIC);
        m_resLib->Get<PixelShader>(PS_VSM)->Bind();
        vs->Bind();
        m_resLib->Get<InputLayout>(IL_BASIC)->Bind();

        {
            XMFLOAT4X4 transform_;
            XMStoreFloat4x4(&transform_, XMMatrixTranspose(transform.GetTransform()));
            auto cbuf = m_resLib->Get<Buffer>(CB_VS_BASIC_ENTITY);
            cbuf->SetData(&transform_);
            cbuf->VSBindAsCBuf(vs->GetResBinding("EntityCBuf"));
        }

        m_vb->BindAsVB();
        m_ib->BindAsIB(DXGI_FORMAT_R32_UINT);
        m_context->GetDeviceContext()->IASetPrimitiveTopology(topology);
        GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(m_ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));

        m_resLib->Get<VertexShader>(NULL_SHADER)->Bind();
        m_resLib->Get<PixelShader>(NULL_SHADER)->Bind();
    }

    void Mesh::RenderBasicSMap()
    {
        auto vs = m_resLib->Get<VertexShader>(VS_BASIC);
        m_resLib->Get<PixelShader>(NULL_SHADER)->Bind();
        vs->Bind();
        m_resLib->Get<InputLayout>(IL_BASIC)->Bind();

        {
            XMFLOAT4X4 transform_;
            XMStoreFloat4x4(&transform_, XMMatrixTranspose(transform.GetTransform()));
            auto cbuf = m_resLib->Get<Buffer>(CB_VS_BASIC_ENTITY);
            cbuf->SetData(&transform_);
            cbuf->VSBindAsCBuf(vs->GetResBinding("EntityCBuf"));
        }

        m_vb->BindAsVB();
        m_ib->BindAsIB(DXGI_FORMAT_R32_UINT);
        m_context->GetDeviceContext()->IASetPrimitiveTopology(topology);
        GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(m_ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));

        m_resLib->Get<VertexShader>(NULL_SHADER)->Bind();
    }

    void Mesh::ShowImGuiControl(const std::string& label)
    {

        ImGui::Begin(label.c_str());
        ImGui::PushItemWidth(210.0f);
        ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);
        ImGui::PopItemWidth();
        ImGui::End();

    }
}