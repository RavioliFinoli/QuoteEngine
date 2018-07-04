#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include <wrl.h>
#include <vector>
#include "QEModel.h"
#include "QEShader.h"

namespace QuoteEngine
{
	class Camera
	{
	public:
		Camera(DirectX::XMVECTOR EyePosition = {0.f, 0.f, 0.f, 1.0f}, DirectX::XMVECTOR Focus = { 0.f, 0.f, 1.f, 1.f }, DirectX::XMVECTOR UpVector = { 0.f, 1.f, 0.f, 0.f });
		~Camera();

		DirectX::XMMATRIX getViewMatrix();

		void update(DirectX::XMVECTOR EyePosition, DirectX::XMVECTOR Focus, DirectX::XMVECTOR UpVector = {0.f, 1.f, 0.f, 0.f});

	private:
		DirectX::XMMATRIX m_ViewMatrix;
	};

	class QEGUI
	{
	public:
		QEGUI();
		~QEGUI();

		void updateAndDraw();
	private:
		bool init();
	};

	class QERenderingModule
	{
	public:
		static Microsoft::WRL::ComPtr<ID3D11Device> gDevice;
		static Microsoft::WRL::ComPtr<ID3D11DeviceContext> gDeviceContext;
		static Microsoft::WRL::ComPtr<IDXGISwapChain> gSwapChain;
		static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> gBackbufferRTV;


		void render();

		QERenderingModule(HWND WindowHandle);

		HRESULT compileShadersAndCreateShaderPrograms();
		void createModels();

		~QERenderingModule();

	private:
		/*
		*Vectors for models, shaders and shader programs.
		*These will later be replaced by maps.
		*/
		QEGUI m_gui;

		std::vector<QEModel*> m_Models;
		std::vector<QuoteEngine::QEShader*> m_Shaders;
		std::vector<QuoteEngine::QEShaderProgram*> m_ShaderPrograms;


		HRESULT createDirect3DContext(HWND wndHandle);
		void createViewport();
	};

}

