#include "QERenderingModule.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"
#include <unordered_map>
#include <d3d11_1.h>
#include "debugCommon.h"

using QuoteEngine::QEShader;
using QuoteEngine::QEShaderProgram;

Microsoft::WRL::ComPtr<ID3D11Device> QuoteEngine::QERenderingModule::gDevice(nullptr);
Microsoft::WRL::ComPtr<ID3D11DeviceContext> QuoteEngine::QERenderingModule::gDeviceContext(nullptr);
Microsoft::WRL::ComPtr<IDXGISwapChain> QuoteEngine::QERenderingModule::gSwapChain(nullptr);
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> QuoteEngine::QERenderingModule::gBackbufferRTV(nullptr);
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> QuoteEngine::QERenderingModule::gDepthStencilView(nullptr);

QuoteEngine::Camera QuoteEngine::QERenderingModule::gCamera;

void QuoteEngine::QERenderingModule::render()
{
	/*
	*Some of these statements will be moved to QEShaderProgram class, since not
	*all shader programs will use only one render target and might need several 
	*passes etc.
	*/

	// clear the back buffer to a deep blue
	float clearColor[] = { 0, 0, 0, 1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV.Get(), clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0.0);

	// specify the topology to use when drawing
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Set shader program (hardcoded for now)
	m_ShaderPrograms[0]->bind();

	//drawModels();
	for (auto model : m_Models)
	{
		{
			//Update cbuffer
			QuoteEngine::CB_PerModel perModel = {};
			auto WVP = model->getWorldMatrix();
			DirectX::XMMATRIX ViewMatrix = QERenderingModule::gCamera.getViewMatrix();
			DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(1.57, (float)640 / (float)480, 0.1f, 100.f);

			WVP = DirectX::XMMatrixMultiply(WVP, DirectX::XMMatrixMultiply(ViewMatrix, ProjectionMatrix));
			DirectX::XMStoreFloat4x4(&perModel._WVP, WVP);
			
			m_ShaderPrograms[0]->updateBuffer("WVP", &perModel);
		}

		//get the buffer, stride and offset
		ID3D11Buffer* buffer = model->getVertexBuffer(); 
		UINT stride = model->getStrideInBytes();
		UINT offset = 0;

		//set the buffer and draw model
		gDeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
		gDeviceContext->Draw(model->getVertexCount(), 0);
	}
	//m_gui.updateAndDraw();

	//all models drawn; present.
	gSwapChain->Present(1, 0);
}

QuoteEngine::QERenderingModule::QERenderingModule(HWND WindowHandle)
{
	createDirect3DContext(WindowHandle);
	createViewport();
	createDepthStencilView();
}

HRESULT QuoteEngine::QERenderingModule::compileShadersAndCreateShaderPrograms()
{
	/*
	*Currently hardcoding shaders and shader programs*
	*/

	//Constant buffers

	//float arr[3] = { 1.0f, 0.0f, 0.0f };
	//QEConstantBuffer* FragmentTest = new QEConstantBuffer(sizeof(float)*3, &arr, 0, QuoteEngine::SHADER_TYPE::PIXEL_SHADER);

	QuoteEngine::CB_PerModel perModel = {};
	DirectX::XMMATRIX WVP;
	DirectX::XMMATRIX ViewMatrix = QERenderingModule::gCamera.getViewMatrix();
	DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(1.57, (float)640/(float)480, 0.1f, 100.f);

	WVP = DirectX::XMMatrixMultiply(ViewMatrix, ProjectionMatrix);
	DirectX::XMStoreFloat4x4(&perModel._WVP, WVP);

	QEConstantBuffer* VSTest = DBG_NEW QEConstantBuffer(sizeof(perModel), &perModel, 0, QuoteEngine::SHADER_TYPE::VERTEX_SHADER);


	HRESULT hr = S_OK;
	QEShader* vertexShader = DBG_NEW QEShader();
	hr = vertexShader->compileFromFile(QuoteEngine::SHADER_TYPE::VERTEX_SHADER, L"Vertex.hlsl");
	std::unordered_map<std::string, QEConstantBuffer*> VSBuffers;
	VSBuffers.insert({ "WVP", VSTest });
	vertexShader->addConstantBuffers(VSBuffers);

	QEShader* pixelShader = new QEShader();
	hr = pixelShader->compileFromFile(QuoteEngine::SHADER_TYPE::PIXEL_SHADER, L"Fragment.hlsl");

	QEShaderProgram* basicProgram = new QEShaderProgram();
	hr = basicProgram->initializeShaders({ vertexShader, pixelShader, nullptr, nullptr });

	
	//Create layout
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{
			"POSITION",		// "semantic" name in shader
			0,				// "semantic" index (not used)
			DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
			0,							 // input slot
			0,							 // offset of first element
			D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
			0							 // used for INSTANCING (ignore)
		},
		{
			"COLOR",
			0,				// same slot as previous (same vertexBuffer)
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			12,							// offset of FIRST element (after POSITION)
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
	};
	hr = basicProgram->initializeInputLayout(inputDesc, 2);

	m_ShaderPrograms.push_back(basicProgram);

	return hr;
}

void QuoteEngine::QERenderingModule::createModels()
{
	QEModel* triangle = new QEModel();
	m_Models.push_back(triangle);

	QEModel* secondTriangle = new QEModel();
	secondTriangle->setWorldMatrix(DirectX::XMMatrixTranslation(0.5, 0.0, 0.5));
	m_Models.push_back(secondTriangle);
}

QuoteEngine::QERenderingModule::~QERenderingModule()
{
	/*
	*Delete new'd heap assets
	*/

	for (auto model : m_Models)
		delete model;

	for (auto shader : m_Shaders)
		delete shader;

	for (auto shaderProgram : m_ShaderPrograms)
		delete shaderProgram;

}

HRESULT QuoteEngine::QERenderingModule::createDirect3DContext(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = wndHandle;                           // the window to be used
	scd.SampleDesc.Count = 1;                               // how many multi samples
	scd.Windowed = TRUE;                                    // windowed/full-screen mode

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		gSwapChain.ReleaseAndGetAddressOf(),
		gDevice.ReleaseAndGetAddressOf(),
		NULL,
		gDeviceContext.ReleaseAndGetAddressOf());

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		hr = gDevice->CreateRenderTargetView(pBackBuffer, NULL, gBackbufferRTV.ReleaseAndGetAddressOf());
		pBackBuffer->Release();
	}
	return hr;
}

HRESULT QuoteEngine::QERenderingModule::createDepthStencilView()
{
	//Describe our Depth/Stencil Buffer

	ID3D11Texture2D* pDepthStencil = NULL;

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.Width = 640;
	depthStencilDesc.Height = 480;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));

	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;


	HRESULT dHR = gDevice->CreateTexture2D(&depthStencilDesc, NULL, &pDepthStencil);

	HRESULT dHR2 = gDevice->CreateDepthStencilView(pDepthStencil, &descDSV, gDepthStencilView.ReleaseAndGetAddressOf());
	//gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, depthStencilView);

	D3D11_DEPTH_STENCIL_DESC dsDesc;
	ZeroMemory(&dsDesc, sizeof(dsDesc));

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	ID3D11DepthStencilState*   depthStencilState = nullptr;
	dHR = gDevice->CreateDepthStencilState(&dsDesc, &depthStencilState);
	gDeviceContext->OMSetDepthStencilState(depthStencilState, 1);


	// Bind the depth stencil view
	gDeviceContext->OMSetRenderTargets(1,          // One rendertarget view
		gBackbufferRTV.GetAddressOf(),  // Render target view, created earlier
		gDepthStencilView.Get());     // Depth stencil view for the render target

	return dHR;
	/*
	assert(gDevice != nullptr);

	//Hardcoding resolution
	ID3D11Texture2D* pDepthStencil = NULL;
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = 640;
	descDepth.Height = 480;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	HRESULT hr = gDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil);

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	ID3D11DepthStencilState * pDSState;
	hr = gDevice->CreateDepthStencilState(&dsDesc, &pDSState);
	
	// Bind depth stencil state
	gDeviceContext->OMSetDepthStencilState(pDSState, 1);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	hr = gDevice->CreateDepthStencilView(pDepthStencil, // Depth stencil texture
		&descDSV, // Depth stencil desc
		gDepthStencilView.ReleaseAndGetAddressOf());  // [out] Depth stencil view

					  // Bind the depth stencil view
	gDeviceContext->OMSetRenderTargets(1,          // One rendertarget view
		gBackbufferRTV.GetAddressOf(),      // Render target view, created earlier
		gDepthStencilView.Get());     // Depth stencil view for the render target

	return hr;*/
}

void QuoteEngine::QERenderingModule::createViewport()
{
	/*
	*Hardcoded resolution
	*/

	D3D11_VIEWPORT vp;
	vp.Width = (float)640;
	vp.Height = (float)480;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}

QuoteEngine::QEGUI::QEGUI()
{
	init();
}

QuoteEngine::QEGUI::~QEGUI()
{
}

void QuoteEngine::QEGUI::updateAndDraw()
{
	// Start the ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show a simple window.
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
	{
		static float f = 0.0f;
		static int counter = 0;
		ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

bool QuoteEngine::QEGUI::init()
{
	ImGui::StyleColorsDark();

	return false;
}

QuoteEngine::Camera::Camera()
{
	update({ 0.0, 0.0f, -2.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f });
}

QuoteEngine::Camera::Camera(DirectX::XMVECTOR EyePosition = { 0.0f, 0.0f, -1.0f, 1.0f }, DirectX::XMVECTOR Focus = { 0.0f, 0.0f, 1.0f, 1.0f }, DirectX::XMVECTOR UpVector = {0.0f, 1.0f, 0.0f, 0.0f})
{
	update(EyePosition, Focus, UpVector);
}

QuoteEngine::Camera::~Camera()
{
}

DirectX::XMMATRIX QuoteEngine::Camera::getViewMatrix()
{
	return m_ViewMatrix;
}

void QuoteEngine::Camera::update(DirectX::XMVECTOR EyePosition, DirectX::XMVECTOR Focus, DirectX::XMVECTOR UpVector = { 0, 1, 0, 0 })
{
	m_ViewMatrix = DirectX::XMMatrixLookAtLH(EyePosition, Focus, UpVector);
}
