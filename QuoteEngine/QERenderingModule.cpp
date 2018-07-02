#include "QERenderingModule.h"
using QuoteEngine::QEShader;
using QuoteEngine::QEShaderProgram;

Microsoft::WRL::ComPtr<ID3D11Device> QERenderingModule::gDevice(nullptr);
Microsoft::WRL::ComPtr<ID3D11DeviceContext> QERenderingModule::gDeviceContext(nullptr);
Microsoft::WRL::ComPtr<IDXGISwapChain> QERenderingModule::gSwapChain(nullptr);
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> QERenderingModule::gBackbufferRTV(nullptr);
void QERenderingModule::render()
{
}
QERenderingModule::QERenderingModule(HWND WindowHandle)
{
	createDirect3DContext(WindowHandle);
	createViewport();
}

HRESULT QERenderingModule::compileShadersAndCreateShaderPrograms()
{
	HRESULT hr = S_OK;
	QEShader* vertexShader = new QEShader();
	hr = vertexShader->compileFromFile(QuoteEngine::SHADER_TYPE::VERTEX_SHADER, L"Vertex.hlsl");

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
	basicProgram->initializeInputLayout(inputDesc, 2);

	return hr;
}

void QERenderingModule::createModels()
{
	QEModel* triangle = new QEModel();
	m_Models.push_back(triangle);
}


QERenderingModule::~QERenderingModule()
{
}

HRESULT QERenderingModule::createDirect3DContext(HWND wndHandle)
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
	scd.SampleDesc.Count = 4;                               // how many multisamples
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
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(hr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackbufferRTV);
		pBackBuffer->Release();

		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackbufferRTV, NULL);
	}
	return hr;
}

void QERenderingModule::createViewport()
{
	D3D11_VIEWPORT vp;
	vp.Width = (float)640;
	vp.Height = (float)480;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gDeviceContext->RSSetViewports(1, &vp);
}
