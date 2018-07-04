#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include "QEModel.h"
#include "QEShader.h"

namespace QuoteEngine
{
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

		std::vector<QEModel*> m_Models;
		std::vector<QuoteEngine::QEShader*> m_Shaders;
		std::vector<QuoteEngine::QEShaderProgram*> m_ShaderPrograms;


		HRESULT createDirect3DContext(HWND wndHandle);
		void createViewport();
	};

	class QEGUI
	{
	public:
		QEGUI();
		~QEGUI();

	private:

	};
}

