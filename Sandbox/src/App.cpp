#include "App.h"
#include "ResourceKeys.h"
#include "ShaderCBuf.h"
#include <GDX11/Utils/BasicMesh.h>
#include <GDX11/Utils/Image.h>
#include "Utils/Math.h"

using namespace GDX11;
using namespace DirectX;
using namespace Microsoft::WRL;

#define SCENE_SAMPLE_COUNT 4

namespace VSM
{
    App::App()
    {
        WindowDesc wd = {};
        wd.name = "VSM";
        wd.className = "VSM";
        wd.width = 1280;
        wd.height = 720;
        m_window = std::make_unique<Window>(wd);
        m_window->SetEventCallback(GDX11_BIND_EVENT_FN(OnEvent));

        m_context = std::make_unique<GDX11Context>();

        SetSwapChain();
        ResizeViews(m_window->GetDesc().width, m_window->GetDesc().height);
        SetShaders();
        SetStates();
        SetBuffers();
        SetTextures();
        ResizeVSM(m_vsmSize.x, m_vsmSize.y);
        ResizeVSMMS(m_vsmSize.x, m_vsmSize.y, m_vsmSampleCount);
        SetVSMBlurKernel(m_vsmGaussBlurRadius, m_vsmGaussBlurSigma);
        ResizeBasicSMap(m_basicSMapSize.x, m_basicSMapSize.y);
        m_vsmSampler = SS_ANISO_CLAMP;

        m_imguiManager.Set(m_window.get(), m_context.get());

        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = (float)m_window->GetDesc().width;
        vp.Height = (float)m_window->GetDesc().height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->GetDeviceContext()->RSSetViewports(1, &vp);

        CameraDesc cd = {};
        cd.aspect = (float)m_window->GetDesc().width / m_window->GetDesc().height;
        cd.fov = 60.0f;
        cd.nearZ = 0.1f;
        cd.farZ = 500.0f;
        cd.position = { 0.0f, 0.0f, 0.0f };
        cd.rotation = { 0.0f, 0.0f, 0.0f };
        m_camera.Set(cd);
        m_cameraController.Set(&m_camera, XMFLOAT3(0.0f, 0.0f, 0.0f), 3.0f, 0.8f, 5.0f, 15.0f);

        {
            Transform transform({ 0.0f, 5.5f, -3.0f }, { 45.0f, 45.0f, 45.0f }, { 1.0f, 1.0f, 1.0f });
            Material mat = {};
            mat.color = { 1.0f, 1.0f, 1.0f, 1.0f };
            mat.tiling = { 1.0f, 1.0f };
            mat.shininess = 180.0f;
            mat.diffuseMap = m_resLib.Get<ShaderResourceView>("wood");
            mat.normalMap = m_resLib.Get<ShaderResourceView>("bump_normal");
            mat.depthMap = m_resLib.Get<ShaderResourceView>("bump_depth");
            mat.samplerState = m_resLib.Get<SamplerState>(SS_ANISO_WRAP);
            mat.depthMapScale = 0.1f;
            m_cube.Set(&m_resLib, m_resLib.Get<Buffer>(VB_CUBE), m_resLib.Get<Buffer>(IB_CUBE), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, transform, mat);
        }

        {
            Transform transform({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 30.0f, 1.0f, 30.0f });
            Material mat = {};
            mat.color = { 1.0f, 1.0f, 1.0f, 1.0f };
            mat.tiling = { 20.0f, 20.0f };
            mat.shininess = 260.0f;
            mat.diffuseMap = m_resLib.Get<ShaderResourceView>("wood");
            mat.normalMap = nullptr;
            mat.depthMap = nullptr;
            mat.samplerState = m_resLib.Get<SamplerState>(SS_ANISO_WRAP);
            mat.depthMapScale = 0.1f;
            m_plane.Set(&m_resLib, m_resLib.Get<Buffer>(VB_PLANE), m_resLib.Get<Buffer>(IB_PLANE), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, transform, mat);
        }

        {
            Transform transform({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
            m_model = std::make_unique<Model>(m_context.get(), &m_resLib, "D:/Utilities/3D_Models/armor-set/source/armor 2021.obj", transform);
        }

        {
            m_dirLight.color = { 1.0f, 1.0f, 1.0f };
            m_dirLight.ambientIntensity = 0.2f;
            m_dirLight.intensity = 1.0f;
            m_dirLight.shadowNearZ = 0.1f;
            m_dirLight.shadowFarZ = 500.0f;
            m_dirLight.rotation = { 50.0f, -30.0f, 0.0f };
        }
    }

    void App::Run()
    {
        while (!m_window->GetState().shouldClose)
        {
            m_time.UpdateDeltaTime();

            OnUpdate();
            OnRender();
            OnImGuiRender();

            Window::PollEvents();

            HRESULT hr;
            if (FAILED(hr = m_context->GetSwapChain()->Present(1, 0)))
            {
                if (hr == DXGI_ERROR_DEVICE_REMOVED)
                    throw GDX11_CONTEXT_DEVICE_REMOVED_EXCEPT(hr);
                else
                    throw GDX11_CONTEXT_EXCEPT(hr);
            }
        }
    }

    void App::OnEvent(GDX11::Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<WindowResizeEvent>(GDX11_BIND_EVENT_FN(OnWindowResizedEvent));

        m_cameraController.OnEvent(event, m_time.GetDeltaTime());
    }

    void App::OnUpdate()
    {
        m_cameraController.ProcessInput(m_window.get(), m_time.GetDeltaTime());
        //m_dirLight.rotation.y += m_time.GetDeltaTime() * 20.0f;
    }

    void App::OnRender()
    {
        if (m_useVSM)
        {
            VSMPass();
            if (m_blurVSM) VSMBlurPass();
        }
        else
        {
            BasicSMapPass();
        }

        RenderPass();
        GammaCorrectionPass();
    }

    void App::OnImGuiRender()
    {
        m_imguiManager.Begin();
        
        ImGui::Begin("Directional Light");
        ImGui::PushItemWidth(210.0f);
        ImGui::DragFloat3("Rotation", &m_dirLight.rotation.x, 0.1f);
        ImGui::PopItemWidth();
        ImGui::End();

        m_cube.ShowImGuiControl("Cube");
        m_plane.ShowImGuiControl("Plane");
        m_model->ShowImGuiControl("Model");

        if (ImGui::Begin("Basic Shadow Map"))
        {
            ImGui::PushItemWidth(110.0f);

            if (ImGui::DragInt2("Size", (int*)&m_basicSMapSize.x, 0.1f, 0))
                ResizeBasicSMap(m_basicSMapSize.x, m_basicSMapSize.y);

            static const char* pcfItems[] = { "No PCF", "3x3", "5x5", "7x7", "9x9" };
            ImGui::Combo("PCF", &m_basicSMapPCF, pcfItems, 5);

            ImGui::PopItemWidth();
            ImGui::End();
        }


        if (ImGui::Begin("Variance Shadow Map"))
        {
            ImGui::PushItemWidth(110.0f);

            ImGui::Checkbox("Enable", &m_useVSM);

            if (ImGui::DragInt2("Size", (int*)&m_vsmSize.x, 1.0f, 0))
            {
                ResizeVSM(m_vsmSize.x, m_vsmSize.y);
                ResizeVSMMS(m_vsmSize.x, m_vsmSize.y, m_vsmSampleCount);
            }

            ImGui::DragFloat("Min. variance", &m_vsmMinVariance, 0.0000001f, 0.0f, 1.0f, "%.7f");
            ImGui::DragFloat("Light bleed reduction", &m_vsmLightBleedReduction, 0.01f, 0.0f, 1.0f);

            static const char* sampleItems[] = { "MSAA 2x", "MSAA 4x", "MSAA 8x" };
            if (ImGui::Combo("Sample Count", &m_vsmSampleCountArrayIndex, sampleItems, 3))
            {
                switch (m_vsmSampleCountArrayIndex)
                {
                case 0:
                    m_vsmSampleCount = 2;
                    break;
                case 1:
                    m_vsmSampleCount = 4;
                    break;
                case 2:
                    m_vsmSampleCount = 8;
                    break;
                }

                ResizeVSMMS(m_vsmSize.x, m_vsmSize.y, m_vsmSampleCount);
            }

            static const char* samplerItems[] = { "Point", "Linear", "Anisotropic" };
            if (ImGui::Combo("Sampler", &m_vsmSamplerArrayIndex, samplerItems, 3))
            {
                switch (m_vsmSamplerArrayIndex)
                {
                case 0:
                    m_vsmSampler = SS_POINT_CLAMP;
                    break;
                case 1:
                    m_vsmSampler = SS_LINEAR_CLAMP;
                    break;
                case 2:
                    m_vsmSampler = SS_ANISO_CLAMP;
                    break;
                }
            }

            ImGui::PopItemWidth();
            ImGui::End();
        }

        if (ImGui::Begin("VSM Gaussian blur"))
        {
            ImGui::PushItemWidth(110.0f);

            ImGui::Checkbox("Enable", &m_blurVSM);
            if (ImGui::DragInt("Radius", &m_vsmGaussBlurRadius, 0.1f, 0, 17))
            {
                m_vsmGaussBlurRadius = std::max(0, m_vsmGaussBlurRadius);
                SetVSMBlurKernel(m_vsmGaussBlurRadius, m_vsmGaussBlurSigma);
            }
            if (ImGui::DragFloat("Sigma", &m_vsmGaussBlurSigma, 0.01f))
            {
                m_vsmGaussBlurSigma = std::max(0.0f, m_vsmGaussBlurSigma);
                SetVSMBlurKernel(m_vsmGaussBlurRadius, m_vsmGaussBlurSigma);
            }

            ImGui::PopItemWidth();
            ImGui::End();
        }


        ImGui::Begin("FPS");
        ImGui::Text(std::to_string(m_time.GetDeltaTime()).c_str());
        ImGui::End();

        m_imguiManager.End();
    }

    void App::BasicSMapPass()
    {
        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = (float)m_basicSMapSize.x;
        vp.Height = (float)m_basicSMapSize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->GetDeviceContext()->RSSetViewports(1, &vp);

        m_resLib.Get<RasterizerState>(RS_DEPTH_SLOPE_SCALED_BIAS)->Bind();
        m_resLib.Get<BlendState>(RES_DEFAULT)->Bind(nullptr, 0xff);
        m_resLib.Get<DepthStencilState>(RES_DEFAULT)->Bind(0xff);

        auto dsv = m_resLib.Get<DepthStencilView>(DSV_BASIC_SMAP);
        dsv->Clear(D3D11_CLEAR_DEPTH, 1.0f, 0xff);
        dsv->Bind();

        auto vs = m_resLib.Get<VertexShader>(VS_BASIC);

        {
            XMFLOAT4X4 lightSpace;
            XMStoreFloat4x4(&lightSpace, XMMatrixTranspose(m_dirLight.GetLightSpace()));
            auto cbuf = m_resLib.Get<Buffer>(CB_VS_BASIC_SYSTEM);
            cbuf->SetData(&lightSpace);
            cbuf->VSBindAsCBuf(vs->GetResBinding("SystemCBuf"));
        }

        m_cube.RenderBasicSMap();
        m_plane.RenderBasicSMap();
        m_model->RenderBasicSMap();
    }

    void App::VSMPass()
    {
        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = (float)m_vsmSize.x;
        vp.Height = (float)m_vsmSize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->GetDeviceContext()->RSSetViewports(1, &vp);

        m_resLib.Get<RasterizerState>(RS_DEPTH_SLOPE_SCALED_BIAS)->Bind();
        m_resLib.Get<BlendState>(RES_DEFAULT)->Bind(nullptr, 0xff);
        m_resLib.Get<DepthStencilState>(RES_DEFAULT)->Bind(0xff);

        auto rtv = m_resLib.Get<RenderTargetView>(RTV_VSM_MS);
        auto dsv = m_resLib.Get<DepthStencilView>(DSV_VSM_MS);
        rtv->Clear(1.0f, 1.0f, 1.0f, 1.0f);
        dsv->Clear(D3D11_CLEAR_DEPTH, 1.0f, 0xff);
        rtv->Bind(dsv.get());

        auto vs = m_resLib.Get<VertexShader>(VS_BASIC);

        {
            XMFLOAT4X4 lightSpace;
            XMStoreFloat4x4(&lightSpace, XMMatrixTranspose(m_dirLight.GetLightSpace()));
            auto cbuf = m_resLib.Get<Buffer>(CB_VS_BASIC_SYSTEM);
            cbuf->SetData(&lightSpace);
            cbuf->VSBindAsCBuf(vs->GetResBinding("SystemCBuf"));
        }

        m_cube.RenderVSM();
        m_plane.RenderVSM();
        m_model->RenderVSM();

        // temporary resolve the ms smap to final srv
        auto smap = m_resLib.Get<ShaderResourceView>(SRV_VSM);
        {
            auto rtvMS = m_resLib.Get<RenderTargetView>(RTV_VSM_MS);
            GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->ResolveSubresource(smap->GetTexture2D()->GetNative(), D3D11CalcSubresource(0, 0, 1),
                rtvMS->GetTexture2D()->GetNative(), D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R32G32_FLOAT));

            if (!m_blurVSM)
                m_context->GetDeviceContext()->GenerateMips(m_resLib.Get<ShaderResourceView>(SRV_VSM)->GetNative());
        }
    }

    void App::VSMBlurPass()
    {
        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = (float)m_vsmSize.x;
        vp.Height = (float)m_vsmSize.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->GetDeviceContext()->RSSetViewports(1, &vp);

        m_resLib.Get<RasterizerState>(RES_DEFAULT)->Bind();
        m_resLib.Get<BlendState>(RES_DEFAULT)->Bind(nullptr, 0xff);
        m_resLib.Get<DepthStencilState>(RES_DEFAULT)->Bind(0xff);

        m_resLib.Get<VertexShader>(VS_FS_OUT_TC_POS)->Bind();
        auto ps = m_resLib.Get<PixelShader>(PS_BLUR);
        ps->Bind();
        m_resLib.Get<InputLayout>(IL_FS_OUT_TC_POS)->Bind();

        // vertical blur
        {
            auto rtv = m_resLib.Get<RenderTargetView>(RTV_VSM_TEMP);
            rtv->Clear(1.0f, 1.0f, 1.0f, 1.0f);
            rtv->Bind(nullptr);

            // bind smap
            m_resLib.Get<ShaderResourceView>(SRV_VSM)->PSBind(ps->GetResBinding("tex"));
            m_resLib.Get<SamplerState>(SS_POINT_CLAMP)->PSBind(ps->GetResBinding("samplerState"));

            // set cbufs
            {
                m_resLib.Get<Buffer>(CB_PS_BLUR_KERNEL)->PSBindAsCBuf(ps->GetResBinding("KernelCBuf"));

                BlurCBuf::PSControl data = {};
                data.texelStep = { 0.0f, 1.0f / m_vsmSize.y };
                auto cbuf = m_resLib.Get<Buffer>(CB_PS_BLUR_CONTROL);
                cbuf->SetData(&data);
                cbuf->PSBindAsCBuf(ps->GetResBinding("ControlCBuf"));
            }

            m_resLib.Get<Buffer>(VB_FS_QUAD)->BindAsVB();
            auto ib = m_resLib.Get<Buffer>(IB_FS_QUAD);
            ib->BindAsIB(DXGI_FORMAT_R32_UINT);
            m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));
        }

        // horizontal blur
        {
            auto rtv = m_resLib.Get<RenderTargetView>(RTV_VSM);
            rtv->Clear(1.0f, 1.0f, 1.0f, 1.0f);
            rtv->Bind(nullptr);

            // bind smap
            m_resLib.Get<ShaderResourceView>(SRV_VSM_TEMP)->PSBind(ps->GetResBinding("tex"));
            m_resLib.Get<SamplerState>(SS_POINT_CLAMP)->PSBind(ps->GetResBinding("samplerState"));

            // set cbufs
            {
                m_resLib.Get<Buffer>(CB_PS_BLUR_KERNEL)->PSBindAsCBuf(ps->GetResBinding("KernelCBuf"));

                BlurCBuf::PSControl data = {};
                data.texelStep = { 1.0f / m_vsmSize.x, 0.0f };
                auto cbuf = m_resLib.Get<Buffer>(CB_PS_BLUR_CONTROL);
                cbuf->SetData(&data);
                cbuf->PSBindAsCBuf(ps->GetResBinding("ControlCBuf"));
            }

            m_resLib.Get<Buffer>(VB_FS_QUAD)->BindAsVB();
            auto ib = m_resLib.Get<Buffer>(IB_FS_QUAD);
            ib->BindAsIB(DXGI_FORMAT_R32_UINT);
            m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));
        }

        m_resLib.Get<VertexShader>(NULL_SHADER)->Bind();
        m_resLib.Get<PixelShader>(NULL_SHADER)->Bind();
        m_context->GetDeviceContext()->GenerateMips(m_resLib.Get<ShaderResourceView>(SRV_VSM)->GetNative());
    }

    void App::RenderPass()
    {
        D3D11_VIEWPORT vp = {};
        vp.TopLeftX = 0.0f;
        vp.TopLeftY = 0.0f;
        vp.Width = (float)m_window->GetDesc().width;
        vp.Height = (float)m_window->GetDesc().height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_context->GetDeviceContext()->RSSetViewports(1, &vp);

        m_resLib.Get<RasterizerState>(RES_DEFAULT)->Bind();
        m_resLib.Get<BlendState>(RES_DEFAULT)->Bind(nullptr, 0xff);
        m_resLib.Get<DepthStencilState>(RES_DEFAULT)->Bind(0xff);

        auto rtv = m_resLib.Get<RenderTargetView>(RTV_SCENE_MS);
        auto dsv = m_resLib.Get<DepthStencilView>(DSV_SCENE_MS);

        rtv->Clear(0.0f, 0.0f, 0.0f, 0.0f);
        dsv->Clear(D3D11_CLEAR_DEPTH, 1.0f, 0xff);
        rtv->Bind(dsv.get());

        auto vs = m_resLib.Get<VertexShader>(VS_PHONG);
        auto ps = m_resLib.Get<PixelShader>(PS_PHONG);

        m_resLib.Get<SamplerState>(m_vsmSampler)->PSBind(ps->GetResBinding("vsmSampler"));
        m_resLib.Get<SamplerState>(SS_CMP_LESS_EQUAL_LINEAR_CLAMP)->PSBind(ps->GetResBinding("smapSampler"));

        if (m_useVSM)
            m_resLib.Get<ShaderResourceView>(SRV_VSM)->PSBind(ps->GetResBinding("vsm"));
        else
            m_resLib.Get<ShaderResourceView>(SRV_BASIC_SMAP)->PSBind(ps->GetResBinding("smap"));

        XMFLOAT4X4 viewProj;
        XMStoreFloat4x4(&viewProj, XMMatrixTranspose(m_camera.GetViewMatrix() * m_camera.GetProjectionMatrix()));
        {
            PhongCBuf::VSSystem data = {};
            data.viewProjection = viewProj;
            data.viewPos = m_camera.GetDesc().position;
            auto cbuf = m_resLib.Get<Buffer>(CB_VS_PHONG_SYSTEM);
            cbuf->SetData(&data);
            cbuf->VSBindAsCBuf(vs->GetResBinding("SystemCBuf"));
        }

        {
            XMFLOAT4X4 lightSpace;
            XMStoreFloat4x4(&lightSpace, XMMatrixTranspose(m_dirLight.GetLightSpace()));

            PhongCBuf::PSSystem data = {};
            data.dirLight.color = m_dirLight.color;
            data.dirLight.intensity = m_dirLight.intensity;
            data.dirLight.ambientIntensity = m_dirLight.ambientIntensity;
            data.dirLight.lightSpace = lightSpace;
            data.vsmControl.enabled = m_useVSM;
            data.vsmControl.minVariance = m_vsmMinVariance;
            data.vsmControl.lightBleedReduction = m_vsmLightBleedReduction;
            data.basicSMapControl.pcfLevel = (uint32_t)m_basicSMapPCF;
            XMStoreFloat3(&data.dirLight.direction, m_dirLight.GetDirection());
            auto cbuf = m_resLib.Get<Buffer>(CB_PS_PHONG_SYSTEM);
            cbuf->SetData(&data);
            cbuf->PSBindAsCBuf(ps->GetResBinding("SystemCBuf"));
        }

        m_cube.Render();
        m_plane.Render();
        m_model->Render();
    }

    void App::GammaCorrectionPass()
    {
        // resolve scene msaa
        auto sceneSrv = m_resLib.Get<ShaderResourceView>(SRV_SCENE);
        {
            auto rtvMS = m_resLib.Get<RenderTargetView>(RTV_SCENE_MS);
            GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->ResolveSubresource(sceneSrv->GetTexture2D()->GetNative(), D3D11CalcSubresource(0, 0, 1),
                rtvMS->GetTexture2D()->GetNative(), D3D11CalcSubresource(0, 0, 1), DXGI_FORMAT_R8G8B8A8_UNORM));
        }

        m_resLib.Get<RenderTargetView>(RTV_MAIN)->Bind(nullptr);
        m_resLib.Get<RenderTargetView>(RTV_MAIN)->Clear(0.0f, 0.0f, 0.0f, 0.0f);

        m_resLib.Get<VertexShader>(VS_FS_OUT_TC_POS)->Bind();
        auto ps = m_resLib.Get<PixelShader>(PS_GAMMA_CORRECTION);
        ps->Bind();
        m_resLib.Get<InputLayout>(IL_FS_OUT_TC_POS)->Bind();

        {
            XMFLOAT4 data = { 2.2f, 0.0f, 0.0f, 0.0f };
            auto cbuf = m_resLib.Get<Buffer>(CB_PS_GAMMA_CORRECTION_SYSTEM);
            cbuf->SetData(&data);
            cbuf->PSBindAsCBuf(ps->GetResBinding("SystemCBuf"));
        }

        sceneSrv->PSBind(ps->GetResBinding("tex"));
        m_resLib.Get<SamplerState>(SS_POINT_CLAMP)->PSBind(ps->GetResBinding("samplerState"));

        m_resLib.Get<Buffer>(VB_FS_QUAD)->BindAsVB();
        auto ib = m_resLib.Get<Buffer>(IB_FS_QUAD);
        ib->BindAsIB(DXGI_FORMAT_R32_UINT);
        m_context->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        GDX11_CONTEXT_THROW_INFO_ONLY(m_context->GetDeviceContext()->DrawIndexed(ib->GetDesc().ByteWidth / sizeof(uint32_t), 0, 0));

        m_resLib.Get<VertexShader>(NULL_SHADER)->Bind();
        m_resLib.Get<PixelShader>(NULL_SHADER)->Bind();
    }

    bool App::OnWindowResizedEvent(GDX11::WindowResizeEvent& event)
    {
        uint32_t width = event.GetWidth();
        uint32_t height = event.GetHeight();

        m_camera.SetAspect((float)width / height);

        ResizeViews(width, height);

        return false;
    }






























#pragma region Resources
    void App::ResizeViews(uint32_t width, uint32_t height)
    {
        // main rtv
        {
            HRESULT hr;

            if (m_resLib.Exist<RenderTargetView>(RTV_MAIN))
            {
                m_resLib.Remove<RenderTargetView>(RTV_MAIN);
                GDX11_CONTEXT_THROW_INFO(m_context->GetSwapChain()->ResizeBuffers(1, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
            }

            D3D11_RENDER_TARGET_VIEW_DESC desc = {};
            desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            desc.Texture2D.MipSlice = 0;
            ComPtr<ID3D11Texture2D> backBuffer;
            GDX11_CONTEXT_THROW_INFO(m_context->GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer));
            m_resLib.Add<RenderTargetView>(RTV_MAIN, RenderTargetView::Create(m_context.get(), desc, Texture2D::Create(m_context.get(), backBuffer.Get())));
        }

        // scene rtv
        {
            if (m_resLib.Exist<RenderTargetView>(RTV_SCENE_MS))
            {
                m_resLib.Remove<RenderTargetView>(RTV_SCENE_MS);
                m_resLib.Remove<ShaderResourceView>(SRV_SCENE);
            }

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = SCENE_SAMPLE_COUNT;
            texDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = texDesc.Format;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            m_resLib.Add(RTV_SCENE_MS, RenderTargetView::Create(m_context.get(), rtvDesc, Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr)));

            // non msaa buffer
            texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            m_resLib.Add(SRV_SCENE, ShaderResourceView::Create(m_context.get(), srvDesc, Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr)));
        }

        // scene dsv
        {
            if (m_resLib.Exist<DepthStencilView>(DSV_SCENE_MS))
            {
                m_resLib.Remove<DepthStencilView>(DSV_SCENE_MS);
            }

            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_D32_FLOAT;
            texDesc.SampleDesc.Count = SCENE_SAMPLE_COUNT;
            texDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = texDesc.Format;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            m_resLib.Add(DSV_SCENE_MS, DepthStencilView::Create(m_context.get(), dsvDesc, Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr)));
        }
    }

    void App::SetShaders()
    {
        m_resLib.Add(NULL_SHADER, VertexShader::Create(m_context.get()));
        m_resLib.Add(NULL_SHADER, PixelShader::Create(m_context.get()));

        m_resLib.Add(VS_PHONG, VertexShader::Create(m_context.get(), "res/cso/phong.vs.cso"));
        m_resLib.Add(PS_PHONG, PixelShader::Create(m_context.get(), "res/cso/phong.ps.cso"));
        m_resLib.Add(IL_PHONG, InputLayout::Create(m_context.get(), m_resLib.Get<VertexShader>(VS_PHONG)));

        m_resLib.Add(VS_BASIC, VertexShader::Create(m_context.get(), "res/cso/basic.vs.cso"));
        m_resLib.Add(PS_VSM, PixelShader::Create(m_context.get(), "res/cso/vsm.ps.cso"));
        m_resLib.Add(IL_BASIC, InputLayout::Create(m_context.get(), m_resLib.Get<VertexShader>(VS_BASIC)));

        m_resLib.Add(VS_FS_OUT_TC_POS, VertexShader::Create(m_context.get(), "res/cso/fullscreen_out_tc_pos.vs.cso"));
        m_resLib.Add(PS_GAMMA_CORRECTION, PixelShader::Create(m_context.get(), "res/cso/gamma_correction.ps.cso"));
        m_resLib.Add(IL_FS_OUT_TC_POS, InputLayout::Create(m_context.get(), m_resLib.Get<VertexShader>(VS_FS_OUT_TC_POS)));

        m_resLib.Add(PS_BLUR, PixelShader::Create(m_context.get(), "res/cso/blur.ps.cso"));
    }

    void App::SetStates()
    {
        {
            m_resLib.Add(RES_DEFAULT, RasterizerState::Create(m_context.get(), CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT())));

            D3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
            desc.DepthBias = 40;
            desc.SlopeScaledDepthBias = 6.0f;
            desc.DepthBiasClamp = 1.0f;
            m_resLib.Add(RS_DEPTH_SLOPE_SCALED_BIAS, RasterizerState::Create(m_context.get(), desc));
        }

        {
            m_resLib.Add(RES_DEFAULT, BlendState::Create(m_context.get(), CD3D11_BLEND_DESC(CD3D11_DEFAULT())));
        }

        {
            m_resLib.Add(RES_DEFAULT, DepthStencilState::Create(m_context.get(), CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT())));
        }

        {
            D3D11_SAMPLER_DESC desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
            desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
            desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
            for (int i = 0; i < 4; i++)
                desc.BorderColor[i] = 0.0f;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = D3D11_FLOAT32_MAX;
            m_resLib.Add(SS_CMP_LESS_EQUAL_LINEAR_CLAMP, SamplerState::Create(m_context.get(), desc));

            desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
            desc.Filter = D3D11_FILTER_ANISOTROPIC;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            for (int i = 0; i < 4; i++)
                desc.BorderColor[i] = 0.0f;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = D3D11_FLOAT32_MAX;
            m_resLib.Add(SS_ANISO_WRAP, SamplerState::Create(m_context.get(), desc));

            desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
            desc.Filter = D3D11_FILTER_ANISOTROPIC;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            for (int i = 0; i < 4; i++)
                desc.BorderColor[i] = 0.0f;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = D3D11_FLOAT32_MAX;
            m_resLib.Add(SS_ANISO_CLAMP, SamplerState::Create(m_context.get(), desc));

            desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            for (int i = 0; i < 4; i++)
                desc.BorderColor[i] = 0.0f;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = D3D11_FLOAT32_MAX;
            m_resLib.Add(SS_LINEAR_CLAMP, SamplerState::Create(m_context.get(), desc));

            desc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
            desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
            desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
            desc.MipLODBias = 0.0f;
            desc.MaxAnisotropy = D3D11_DEFAULT_MAX_ANISOTROPY;
            desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
            for (int i = 0; i < 4; i++)
                desc.BorderColor[i] = 0.0f;
            desc.MinLOD = 0.0f;
            desc.MaxLOD = D3D11_FLOAT32_MAX;
            m_resLib.Add(SS_POINT_CLAMP, SamplerState::Create(m_context.get(), desc));
        }
    }

    void App::SetBuffers()
    {
        // fs quad
        {
            float vert[] =
            {
                -1.0f,  1.0f,
                 1.0f,  1.0f,
                 1.0f, -1.0f,
                -1.0f, -1.0f
            };

            uint32_t ind[] =
            {
                0, 1, 2,
                2, 3, 0
            };

            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(vert);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 2 * sizeof(float);
            m_resLib.Add(VB_FS_QUAD, Buffer::Create(m_context.get(), desc, vert));

            desc = {};
            desc.ByteWidth = sizeof(ind);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = sizeof(uint32_t);
            m_resLib.Add(IB_FS_QUAD, Buffer::Create(m_context.get(), desc, ind));
        }

        //cube
        {
            auto vert = Utils::CreateCubeVerticesEx();
            auto ind = Utils::CreateCubeIndicesEx();

            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = (uint32_t)vert.size() * sizeof(Utils::VertexEx);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = sizeof(Utils::VertexEx);
            m_resLib.Add(VB_CUBE, Buffer::Create(m_context.get(), desc, vert.data()));

            desc = {};
            desc.ByteWidth = (uint32_t)ind.size() * sizeof(uint32_t);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = sizeof(uint32_t);
            m_resLib.Add(IB_CUBE, Buffer::Create(m_context.get(), desc, ind.data()));
        }

        // plane
        {
            auto vert = Utils::CreatePlaneVerticesEx();
            auto ind = Utils::CreatePlaneIndicesEx();

            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = (uint32_t)vert.size() * sizeof(Utils::VertexEx);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = sizeof(Utils::VertexEx);
            m_resLib.Add(VB_PLANE, Buffer::Create(m_context.get(), desc, vert.data()));

            desc = {};
            desc.ByteWidth = (uint32_t)ind.size() * sizeof(uint32_t);
            desc.Usage = D3D11_USAGE_DEFAULT;
            desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            desc.CPUAccessFlags = 0;
            desc.MiscFlags = 0;
            desc.StructureByteStride = sizeof(uint32_t);
            m_resLib.Add(IB_PLANE, Buffer::Create(m_context.get(), desc, ind.data()));
        }

        // phong cbufs
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(PhongCBuf::VSSystem);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_VS_PHONG_SYSTEM, Buffer::Create(m_context.get(), desc, nullptr));

            desc = {};
            desc.ByteWidth = sizeof(PhongCBuf::VSEntity);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_VS_PHONG_ENTITY, Buffer::Create(m_context.get(), desc, nullptr));

            desc = {};
            desc.ByteWidth = sizeof(PhongCBuf::PSSystem);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_PS_PHONG_SYSTEM, Buffer::Create(m_context.get(), desc, nullptr));

            desc = {};
            desc.ByteWidth = sizeof(PhongCBuf::PSEntity);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_PS_PHONG_ENTITY, Buffer::Create(m_context.get(), desc, nullptr));
        }

        // basic cbufs
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(XMFLOAT4X4);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_VS_BASIC_SYSTEM, Buffer::Create(m_context.get(), desc, nullptr));

            desc = {};
            desc.ByteWidth = sizeof(XMFLOAT4X4);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_VS_BASIC_ENTITY, Buffer::Create(m_context.get(), desc, nullptr));
        }

        // blur cbuf
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(BlurCBuf::PSKernel);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_PS_BLUR_KERNEL, Buffer::Create(m_context.get(), desc, nullptr));

            desc = {};
            desc.ByteWidth = sizeof(BlurCBuf::PSControl);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_PS_BLUR_CONTROL, Buffer::Create(m_context.get(), desc, nullptr));
        }

        // gamma_correction cbufs
        {
            D3D11_BUFFER_DESC desc = {};
            desc.ByteWidth = sizeof(XMFLOAT4);
            desc.Usage = D3D11_USAGE_DYNAMIC;
            desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            desc.MiscFlags = 0;
            desc.StructureByteStride = 0;
            m_resLib.Add(CB_PS_GAMMA_CORRECTION_SYSTEM, Buffer::Create(m_context.get(), desc, nullptr));
        }
    }

    void App::SetTextures()
    {
        {
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = 1;
            texDesc.Height = 1;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;

            uint32_t pixels = 0xffffffff;
            m_resLib.Add(RES_DEFAULT, ShaderResourceView::Create(m_context.get(), srvDesc, Texture2D::Create(m_context.get(), texDesc, &pixels)));
        }

        {
            auto image = Utils::LoadImageFile("res/textures/wood.png", false, 4);
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = image.width;
            texDesc.Height = image.height;
            texDesc.MipLevels = 0;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = -1;
            srvDesc.Texture2D.MostDetailedMip = 0;
            m_resLib.Add("wood", ShaderResourceView::Create(m_context.get(), srvDesc, Texture2D::Create(m_context.get(), texDesc, image.pixels)));

            Utils::FreeImageData(&image);
        }

        {
            auto image = GDX11::Utils::LoadImageFile("res/textures/bump_normal.png", false, 4);
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = image.width;
            texDesc.Height = image.height;
            texDesc.MipLevels = 0;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = -1;

            m_resLib.Add("bump_normal", ShaderResourceView::Create(m_context.get(), srvDesc, Texture2D::Create(m_context.get(), texDesc, image.pixels)));
            Utils::FreeImageData(&image);
        }


        {
            auto image = GDX11::Utils::LoadImageFile("res/textures/bump_depth.png", false, 4);
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = image.width;
            texDesc.Height = image.height;
            texDesc.MipLevels = 0;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            texDesc.SampleDesc.Count = 1;
            texDesc.SampleDesc.Quality = 0;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = texDesc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MostDetailedMip = 0;
            srvDesc.Texture2D.MipLevels = -1;

            m_resLib.Add("bump_depth", ShaderResourceView::Create(m_context.get(), srvDesc, Texture2D::Create(m_context.get(), texDesc, image.pixels)));
            Utils::FreeImageData(&image);
        }
    }

    void App::SetSwapChain()
    {
        DXGI_SWAP_CHAIN_DESC scDesc = {};
        scDesc.BufferDesc.Width = 0;
        scDesc.BufferDesc.Height = 0;
        scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferDesc.RefreshRate.Numerator = 0;
        scDesc.BufferDesc.RefreshRate.Denominator = 0;
        scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        scDesc.SampleDesc.Count = 1;
        scDesc.SampleDesc.Quality = 0;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.BufferCount = 1;
        scDesc.OutputWindow = m_window->GetNativeWindow();
        scDesc.Windowed = TRUE;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        scDesc.Flags = 0;
        m_context->SetSwapChain(scDesc);
    }

    void App::SetVSMBlurKernel(int radius, float sigma)
    {
        BlurCBuf::PSKernel k = {};
        k.nTaps = radius * 2u + 1u;
        float sum = 0.0f;
        for (int i = 0; i < (int)k.nTaps; i++)
        {
            const float x = (float)i - radius;
            const float gauss = Gauss(x, sigma);
            sum += gauss;
            k.coefficients[i].x = gauss;
        }
        for (int i = 0; i < (int)k.nTaps; i++)
        {
            k.coefficients[i].x /= sum;
        }

        //k.nTaps = radius * 2u + 1u;
        //float c = 1.0f / k.nTaps;
        //for (int i = 0; i < (int)k.nTaps; i++)
        //    k.coefficients[i].x = c;

        m_resLib.Get<Buffer>(CB_PS_BLUR_KERNEL)->SetData(&k);
    }

    void App::ResizeVSM(uint32_t width, uint32_t height)
    {
        if (m_resLib.Exist<RenderTargetView>(RTV_VSM))
        {
            m_resLib.Remove<RenderTargetView>(RTV_VSM_TEMP);
            m_resLib.Remove<ShaderResourceView>(SRV_VSM_TEMP);
            m_resLib.Remove<RenderTargetView>(RTV_VSM);
            m_resLib.Remove<ShaderResourceView>(SRV_VSM);
        }

        // blur rtv
        {
            {
                D3D11_TEXTURE2D_DESC texDesc = {};
                texDesc.Width = width;
                texDesc.Height = height;
                texDesc.MipLevels = 1;
                texDesc.ArraySize = 1;
                texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                texDesc.SampleDesc.Count = 1;
                texDesc.SampleDesc.Quality = 0;
                texDesc.Usage = D3D11_USAGE_DEFAULT;
                texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
                texDesc.CPUAccessFlags = 0;
                texDesc.MiscFlags = 0;
                auto tex = Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr);

                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
                m_resLib.Add(RTV_VSM_TEMP, RenderTargetView::Create(m_context.get(), rtvDesc, tex));

                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;
                srvDesc.Texture2D.MostDetailedMip = 0;
                m_resLib.Add(SRV_VSM_TEMP, ShaderResourceView::Create(m_context.get(), srvDesc, tex));
            }

            {
                D3D11_TEXTURE2D_DESC texDesc = {};
                texDesc.Width = width;
                texDesc.Height = height;
                texDesc.MipLevels = 0;
                texDesc.ArraySize = 1;
                texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                texDesc.SampleDesc.Count = 1;
                texDesc.SampleDesc.Quality = 0;
                texDesc.Usage = D3D11_USAGE_DEFAULT;
                texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
                texDesc.CPUAccessFlags = 0;
                texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
                auto tex = Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr);

                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
                rtvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = 0;
                m_resLib.Add(RTV_VSM, RenderTargetView::Create(m_context.get(), rtvDesc, tex));

                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = -1;
                srvDesc.Texture2D.MostDetailedMip = 0;
                m_resLib.Add(SRV_VSM, ShaderResourceView::Create(m_context.get(), srvDesc, tex));
            }
        }
    }

    void App::ResizeVSMMS(uint32_t width, uint32_t height, int sample)
    {
        if (m_resLib.Exist<RenderTargetView>(RTV_VSM_MS))
        {
            m_resLib.Remove<DepthStencilView>(DSV_VSM_MS);
            m_resLib.Remove<RenderTargetView>(RTV_VSM_MS);
        }

        {
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_D32_FLOAT;
            texDesc.SampleDesc.Count = sample;
            texDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;
            auto tex = Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr);

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
            dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
            m_resLib.Add(DSV_VSM_MS, DepthStencilView::Create(m_context.get(), dsvDesc, tex));
        }

        {
            D3D11_TEXTURE2D_DESC texDesc = {};
            texDesc.Width = width;
            texDesc.Height = height;
            texDesc.MipLevels = 1;
            texDesc.ArraySize = 1;
            texDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
            texDesc.SampleDesc.Count = sample;
            texDesc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
            texDesc.Usage = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
            texDesc.CPUAccessFlags = 0;
            texDesc.MiscFlags = 0;

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
            rtvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
            m_resLib.Add(RTV_VSM_MS, RenderTargetView::Create(m_context.get(), rtvDesc, Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr)));
        }
    }

    void App::ResizeBasicSMap(uint32_t width, uint32_t height)
    {
        if (m_resLib.Exist<DepthStencilView>(DSV_BASIC_SMAP))
        {
            m_resLib.Remove<DepthStencilView>(DSV_BASIC_SMAP);
            m_resLib.Remove<ShaderResourceView>(SRV_BASIC_SMAP);
        }

        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = width;
        texDesc.Height = height;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        texDesc.SampleDesc.Count = 1;
        texDesc.SampleDesc.Quality = 0;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        texDesc.CPUAccessFlags = 0;
        texDesc.MiscFlags = 0;
        auto tex = Texture2D::Create(m_context.get(), texDesc, (const void*)nullptr);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        m_resLib.Add(DSV_BASIC_SMAP, DepthStencilView::Create(m_context.get(), dsvDesc, tex));

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;
        srvDesc.Texture2D.MostDetailedMip = 0;
        m_resLib.Add(SRV_BASIC_SMAP, ShaderResourceView::Create(m_context.get(), srvDesc, tex));
    }
#pragma endregion Resources
}