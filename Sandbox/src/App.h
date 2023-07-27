#pragma once
#include <GDX11.h>
#include <GDX11/Utils/ResourceLibrary.h>
#include <GDX11/Utils/Time.h>
#include "Rendering/Camera.h"
#include "Rendering/EditorCameraController.h"
#include "Rendering/Mesh.h"
#include "Rendering/Model.h"
#include "Rendering/Light.h"
#include "ImGui/ImGuiManager.h"

namespace VSM
{
	class App
	{
	public:
		App();

		void Run();

	private:
		void OnEvent(GDX11::Event& event);
		void OnUpdate();
		void OnRender();
		void OnImGuiRender();

		void BasicSMapPass();
		void VSMPass();
		void VSMBlurPass();
		void RenderPass();
		void GammaCorrectionPass();

		bool OnWindowResizedEvent(GDX11::WindowResizeEvent& event);


		
		void ResizeViews(uint32_t width, uint32_t height);
		void SetShaders();
		void SetStates();
		void SetBuffers();
		void SetTextures();
		void SetSwapChain();
		void SetVSMBlurKernel(int radius, float sigma);
		void ResizeBasicSMap(uint32_t width, uint32_t height);
		void ResizeVSM(uint32_t width, uint32_t height);
		void ResizeVSMMS(uint32_t width, uint32_t height, int sample);




		std::unique_ptr<GDX11::Window> m_window;
		std::unique_ptr<GDX11::GDX11Context> m_context;
		GDX11::Utils::ResourceLibrary m_resLib;
		ImGuiManager m_imguiManager;

		Camera m_camera;
		EditorCameraController m_cameraController;

		GDX11::Utils::Time m_time;

		DirectionalLight m_dirLight;
		Mesh m_cube;
		Mesh m_plane;
		std::unique_ptr<Model> m_model;

		// controls
		bool m_useVSM = false;
		bool m_blurVSM = false;
		DirectX::XMUINT2 m_vsmSize = { 512, 512 };
		int m_vsmGaussBlurRadius = 4;
		float m_vsmGaussBlurSigma = 1.5f;
		float m_vsmMinVariance = 0.000001f;
		float m_vsmLightBleedReduction = 0.2f;
		int m_vsmSampleCountArrayIndex = 0;
		int m_vsmSampleCount = 2;
		int m_vsmSamplerArrayIndex = 2;
		std::string m_vsmSampler;

		DirectX::XMUINT2 m_basicSMapSize = { 512, 512 };
		int m_basicSMapPCF = 4;
	};
}