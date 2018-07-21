#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <vector>
#include "QEModel.h"
#include "QEShader.h"

//using QuoteEngine::QEShaderProgram;
//using QuoteEngine::QEShader;

namespace QuoteEngine
{
	#define DEFAULT_FORWARD {0.0, 0.0, 1.0, 0.0}
	#define DEFAULT_RIGHT {1.0, 0.0, 0.0, 0.0}

	class Camera
	{
	public:
		struct CameraData
		{
			float camPitch = 0;
			float camYaw = 0;
			DirectX::XMVECTOR camPosition = { 0.0, 0.0, 0.0, 1.0 };
		};

		struct CameraUpdateData 
		{
			DirectX::XMVECTOR camPosition = { 0.0, 0.0, 0.0, 1.0 };
			DirectX::XMVECTOR camTarget = { 0.0, 0.0, 0.0, 1.0 };
			DirectX::XMVECTOR camUp = { 0.0, 1.0, 0.0, 1.0 };
		};

		Camera();
		Camera(DirectX::XMVECTOR, DirectX::XMVECTOR, DirectX::XMVECTOR);
		~Camera();

		DirectX::XMMATRIX getViewMatrix();
		DirectX::XMVECTOR getCameraPosition();
		DirectX::XMVECTOR getCameraTarget();
		void setViewMatrix(DirectX::XMMATRIX matrix);

		void update(DirectX::XMVECTOR EyePosition, DirectX::XMVECTOR Focus, DirectX::XMVECTOR UpVector = {0.0, 1.0, 0.0, 0.0});
		void update(CameraUpdateData data);
		CameraData getCameraData();
		void updateCameraInformation(CameraData info);
	private:
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMVECTOR m_CamPosition = {0.0, 0.0, -1.0, 1.0};
		DirectX::XMVECTOR m_CamTarget = {0.0, 0.0, 0.0, 0.0};

		float m_CamYaw = 0;
		float m_CamPitch = 0;
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
		static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> gDepthStencilView;
		static QuoteEngine::Camera gCamera;

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

		std::vector<std::unique_ptr<QEModel>> m_Models;
		std::vector<QuoteEngine::QEShader*> m_Shaders;
		std::unordered_map < std::string, std::unique_ptr<QuoteEngine::QEShaderProgram >> m_ShaderPrograms;

		std::unique_ptr<QuoteEngine::QEShaderProgram> createProgram(const std::string name, const std::vector<QEShader*>& shaders, D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions, const size_t numElements);


		HRESULT createDirect3DContext(HWND wndHandle);
		HRESULT createDepthStencilView();
		void createViewport();
	};

}

