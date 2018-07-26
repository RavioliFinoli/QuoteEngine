#include "QERenderingModule.h"

#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"
#include <unordered_map>
#include <d3d11_1.h>
#include <fstream>
#include "debugCommon.h"

using QuoteEngine::QEShader;
using QuoteEngine::QEShaderProgram;
using QuoteEngine::CB_PerModel_WVP_W;
using QuoteEngine::CB_PerModel;

bool QuoteEngine::QERenderingModule::gUseDeferredShader = true;
bool QuoteEngine::QERenderingModule::gBlur = false;
INT QuoteEngine::QERenderingModule::gActiveScene = 0;
INT QuoteEngine::QERenderingModule::gSelectedModel = 0;
INT QuoteEngine::QERenderingModule::m_s_SceneCount = 0;
std::vector<std::unique_ptr<QEScene>> QuoteEngine::QERenderingModule::m_s_Scenes;

Microsoft::WRL::ComPtr<ID3D11Device> QuoteEngine::QERenderingModule::gDevice(nullptr);
Microsoft::WRL::ComPtr<ID3D11DeviceContext> QuoteEngine::QERenderingModule::gDeviceContext(nullptr);
Microsoft::WRL::ComPtr<IDXGISwapChain> QuoteEngine::QERenderingModule::gSwapChain(nullptr);
Microsoft::WRL::ComPtr<ID3D11RenderTargetView> QuoteEngine::QERenderingModule::gBackbufferRTV(nullptr);
Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> QuoteEngine::QERenderingModule::gBackbufferUAV(nullptr);
Microsoft::WRL::ComPtr<ID3D11DepthStencilView> QuoteEngine::QERenderingModule::gDepthStencilView(nullptr);
Microsoft::WRL::ComPtr<ID3D11SamplerState> QuoteEngine::QERenderingModule::gSampleState(nullptr);

QuoteEngine::Camera QuoteEngine::QERenderingModule::gCamera;

void QuoteEngine::QERenderingModule::render()
{
	/*
	*Some of these statements will be moved to QEShaderProgram class, since not
	*all shader programs will use only one render target and might need several 
	*passes etc.
	*/

	// clear the back buffer to a deep blue
	float clearColor[] = { 0.9, 0.9, 0.9, 1 };
	gDeviceContext->ClearRenderTargetView(gBackbufferRTV.Get(), clearColor);
	gDeviceContext->ClearDepthStencilView(gDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0, 0.0);

	// specify the topology to use when drawing
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//drawModels();
	if (gUseDeferredShader) //deferred rendering
	{
		//Geometry pass
		{
			m_ShaderPrograms.at("deferredGeometryPass")->bind();

			//Verify scene
			if (gActiveScene >= m_s_Scenes.size())
				return;

			auto models = m_s_Scenes[gActiveScene]->getModels();
			for (auto& model : models)
			{
				//Update cbuffers and draw
				{
					drawInstancesDeferred(model);
				}
			}
			m_ShaderPrograms.at("deferredGeometryPass")->unbind();
		}
		//Lighting pass
		{
			QuoteEngine::CB_PerFrame_Cam perFrame = {};
			DirectX::XMFLOAT4 camPos;
			DirectX::XMStoreFloat4(&camPos, QERenderingModule::gCamera.getCameraPosition());
			perFrame._CamPosition = { camPos.x, camPos.y, camPos.z };
			m_ShaderPrograms.at("deferredLightingPass")->updateBuffer("CAM", &perFrame);

			m_ShaderPrograms.at("deferredLightingPass")->bind();
			gDeviceContext->OMSetRenderTargets(1, gBackbufferRTV.GetAddressOf(), gDepthStencilView.Get());
			gDeviceContext->Draw(3, 0);
			m_ShaderPrograms.at("deferredLightingPass")->unbind();
			gDeviceContext->OMSetRenderTargets(1, gBackbufferRTV.GetAddressOf(), gDepthStencilView.Get());
		}
	}
	else
	{
		gDeviceContext->OMSetRenderTargets(1, gBackbufferRTV.GetAddressOf(), gDepthStencilView.Get());

		if (gActiveScene >= m_s_Scenes.size())
			return;

		std::vector<std::shared_ptr<QEModel>>& models = m_s_Scenes[gActiveScene]->getModels();
		for (auto& model : models)
		{
			if (model->hasAssociatedShaderProgram())
			{
				//forward shading programs

				std::string program = model->getAssociatedShaderProgram();
				m_ShaderPrograms.at(program)->bind();

				if (program == "diffuseProgram")
				{
					//Update VS cbuffer
					{
						QuoteEngine::CB_PerModel_WVP_W perModel = {};
						auto WVP = model->getWorldMatrix();
						DirectX::XMStoreFloat4x4(&perModel._W, WVP);
						DirectX::XMMATRIX ViewMatrix = QERenderingModule::gCamera.getViewMatrix();
						DirectX::XMMATRIX ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(1.2, (float)m_Width / (float)m_Height, 0.1f, 100.f);

						WVP = DirectX::XMMatrixMultiply(WVP, DirectX::XMMatrixMultiply(ViewMatrix, ProjectionMatrix));
						DirectX::XMStoreFloat4x4(&perModel._WVP, WVP);

						m_ShaderPrograms.at(program)->updateBuffer("WVP_W", &perModel);
					}

					//Update PS cbuffers
					{
						QuoteEngine::CB_PerFrame_Cam perFrame = {};
						DirectX::XMFLOAT4 camPos;
						DirectX::XMStoreFloat4(&camPos, QERenderingModule::gCamera.getCameraPosition());
						perFrame._CamPosition = { camPos.x, camPos.y, camPos.z };
						m_ShaderPrograms.at(program)->updateBuffer("CAM", &perFrame);

						QEModel::QEMaterial material = model->getMaterial();
						m_ShaderPrograms.at(program)->updateBuffer("MATERIAL", &material);
					}
				}

				//get the buffer, stride and offset
				ID3D11Buffer* buffer = model->getVertexBuffer();
				UINT stride = model->getStrideInBytes();
				UINT offset = 0;

				//set the buffer and draw model
				gDeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
				gDeviceContext->PSSetSamplers(0, 1, gSampleState.GetAddressOf());
				model->bindTexture();
				gDeviceContext->Draw(model->getVertexCount(), 0);
			}
		}
		
	}
	m_gui.updateAndDraw();
	if (gBlur)
	{
		//Null views for unbinding
		ID3D11RenderTargetView* nullRTV = { nullptr };
		ID3D11UnorderedAccessView* nullUAV = { nullptr };

		//Bind CS
		m_StandaloneShaders.at("BlurCS")->bindShaderAndResources();

		//Set UAV (backbuffer)
		gDeviceContext->OMSetRenderTargets(1, &nullRTV, gDepthStencilView.Get());
		gDeviceContext->CSSetUnorderedAccessViews(0, 1, gBackbufferUAV.GetAddressOf(), 0);

		//Blur
		gDeviceContext->Dispatch((UINT)m_Width / 32, (UINT)m_Height / 30, 1);

		//Unbind UAV, rebind RTV
		gDeviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, 0);
		gDeviceContext->OMSetRenderTargets(1, gBackbufferRTV.GetAddressOf(), gDepthStencilView.Get());
	}
	//all models drawn; present.
	gSwapChain->Present(0, 0);
}

QuoteEngine::QERenderingModule::QERenderingModule(HWND WindowHandle, LONG width, LONG height) : m_Width(width), m_Height(height)
{
	m_s_SceneCount = 0;
	createDirect3DContextAndBackbuffer(WindowHandle);
	createViewport();
	createDepthStencilView();
}

HRESULT QuoteEngine::QERenderingModule::compileShadersAndCreateShaderPrograms()
{
	/*
	*Currently hard coding shaders and shader programs*
	*/

	//Create shader resource views and render targets for deferred rendering
	ComPtr<ID3D11RenderTargetView> rt_norm(nullptr);
	ComPtr<ID3D11RenderTargetView> rt_diff(nullptr);
	ComPtr<ID3D11RenderTargetView> rt_position(nullptr);
	ComPtr<ID3D11RenderTargetView> rt_spec(nullptr);
	ComPtr<ID3D11ShaderResourceView> sr_norm(nullptr);
	ComPtr<ID3D11ShaderResourceView> sr_diff(nullptr);
	ComPtr<ID3D11ShaderResourceView> sr_position(nullptr);
	ComPtr<ID3D11ShaderResourceView> sr_spec(nullptr);
	HRESULT hr = S_OK;
	{
		ComPtr<ID3D11Texture2D> norm2D(nullptr);
		ComPtr<ID3D11Texture2D> diff2D(nullptr);
		ComPtr<ID3D11Texture2D> spec2D(nullptr);
		ComPtr<ID3D11Texture2D> position2D(nullptr);




		//Tex2d desc
		D3D11_TEXTURE2D_DESC desc2D = {};
		desc2D.ArraySize = 1;
		desc2D.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc2D.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc2D.Width = m_Width;
		desc2D.Height = m_Height;
		desc2D.Usage = D3D11_USAGE_DEFAULT;
		desc2D.MipLevels = 1;
		desc2D.SampleDesc.Count = 1;
		desc2D.SampleDesc.Quality = 0;

		//RTV desc
		D3D11_RENDER_TARGET_VIEW_DESC descRTV = {};
		descRTV.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		descRTV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		//SRV desc
		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		descSRV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		descSRV.Texture2D.MipLevels = 1;
		descSRV.Texture2D.MostDetailedMip = 0;

		hr = gDevice->CreateTexture2D(&desc2D, NULL, norm2D.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateTexture2D(&desc2D, NULL, diff2D.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateTexture2D(&desc2D, NULL, position2D.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateTexture2D(&desc2D, NULL, spec2D.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);

		hr = gDevice->CreateRenderTargetView(position2D.Get(), &descRTV, rt_position.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateRenderTargetView(norm2D.Get(), &descRTV, rt_norm.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateRenderTargetView(spec2D.Get(), &descRTV, rt_spec.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateRenderTargetView(diff2D.Get(), &descRTV, rt_diff.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);

		hr = gDevice->CreateShaderResourceView(norm2D.Get(), &descSRV, sr_norm.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateShaderResourceView(position2D.Get(), &descSRV, sr_position.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateShaderResourceView(spec2D.Get(), &descSRV, sr_spec.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
		hr = gDevice->CreateShaderResourceView(diff2D.Get(), &descSRV, sr_diff.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);
	}


	//Constant buffers structs
	CB_PerModel_WVP_W initPerModel    = {};
	CB_PerModel_Material initMaterial = {};
	CB_PerFrame_Cam initPerFrame      = {};

	//Constant buffers creation
	auto WVP_W    = createConstantBuffer(initPerModel, QuoteEngine::SHADER_TYPE::VERTEX_SHADER, 0);
	auto Cam      = createConstantBuffer(initPerFrame, QuoteEngine::SHADER_TYPE::PIXEL_SHADER, 0);
	auto Material = createConstantBuffer(initMaterial, SHADER_TYPE::PIXEL_SHADER, 1);
	
	//Shader creation
	auto diffuseVertexShader = std::make_shared<QEShader>();
	auto diffusePixelShader  = std::make_shared<QEShader>();
	auto deferredPixelPass1  = std::make_shared<QEShader>();
	auto deferredPixelPass2  = std::make_shared<QEShader>();
	auto deferredVertexPass1 = std::make_shared<QEShader>();
	auto deferredVertexPass2 = std::make_shared<QEShader>();
	auto gaussianBlurCS      = std::make_shared<QEShader>();

	//Compiling shaders
	hr = diffuseVertexShader->compileFromFile(SHADER_TYPE::VERTEX_SHADER, L"DiffuseVertexShader.hlsl");
	ASSERT_SOK(hr);
	hr = diffusePixelShader->compileFromFile(SHADER_TYPE::PIXEL_SHADER, L"DiffusePixelShader.hlsl");
	ASSERT_SOK(hr);

	hr = deferredPixelPass1->compileFromFile(SHADER_TYPE::PIXEL_SHADER, L"PS_DRP1.hlsl");
	ASSERT_SOK(hr);
	hr = deferredPixelPass2->compileFromFile(SHADER_TYPE::PIXEL_SHADER, L"PS_DRP2.hlsl");
	ASSERT_SOK(hr);
	hr = deferredVertexPass1->compileFromFile(SHADER_TYPE::VERTEX_SHADER, L"VS_passthrough.hlsl");
	ASSERT_SOK(hr);
	hr = deferredVertexPass2->compileFromFile(SHADER_TYPE::VERTEX_SHADER, L"VS_quad.hlsl");
	ASSERT_SOK(hr);

	hr = gaussianBlurCS->compileFromFile(SHADER_TYPE::COMPUTE_SHADER, L"GaussianBlurCS.hlsl");
	ASSERT_SOK(hr);

	//Making maps of cbuffers and adding them to shaders
	auto DiffusePerModel = createConstantBufferMap({ {"WVP_W", WVP_W} });
	auto PSPerFrame      = createConstantBufferMap({ {"CAM", Cam}, {"MATERIAL", Material} });
	auto PSMaterial = createConstantBufferMap({ { "MATERIAL", Material } });
	auto PSCamera = createConstantBufferMap({ {"CAM", Cam} });

	diffuseVertexShader->addConstantBuffers(DiffusePerModel);
	diffusePixelShader->addConstantBuffers(PSPerFrame);
	deferredPixelPass1->addConstantBuffers(PSMaterial);
	deferredPixelPass2->addConstantBuffers(PSCamera);

	//Store shaders in this
	m_Shaders.push_back(diffuseVertexShader);
	m_Shaders.push_back(diffusePixelShader);
	m_Shaders.push_back(deferredVertexPass2);
	m_Shaders.push_back(deferredPixelPass1);
	m_Shaders.push_back(deferredPixelPass2);
	m_StandaloneShaders.insert({ "BlurCS", gaussianBlurCS });

	//Create layouts
	D3D11_INPUT_ELEMENT_DESC basicInputDesc[] = {
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
			D3D11_APPEND_ALIGNED_ELEMENT,							// offset of FIRST element (after POSITION)
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
	};
	D3D11_INPUT_ELEMENT_DESC posInputDesc[] = {
		{
			"POSITION",		// "semantic" name in shader
			0,				// "semantic" index (not used)
			DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
			0,							 // input slot
			0,							 // offset of first element
			D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
			0							 // used for INSTANCING (ignore)
		}
	};
	D3D11_INPUT_ELEMENT_DESC diffuseInputDesc[] = {
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
			"NORMAL",
			0,				// same slot as previous (same vertexBuffer)
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,							// offset of FIRST element (after POSITION)
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		{
			"TEXCOORD",
			0,				// same slot as previous (same vertexBuffer)
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,							// offset of FIRST element (after POSITION)
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};

	//Creating shader programs and inserting into map
	m_ShaderPrograms.insert({ "diffuseProgram", createProgram("diffuseProgram", {diffuseVertexShader, diffusePixelShader, nullptr, nullptr}, diffuseInputDesc, 3) });
	m_ShaderPrograms.insert({ "deferredGeometryPass", createProgram("deferredGeometryPass", {diffuseVertexShader, deferredPixelPass1, nullptr, nullptr}, diffuseInputDesc, 3) });
	m_ShaderPrograms.insert({ "deferredLightingPass", createProgram("deferredLightingPass", {deferredVertexPass2, deferredPixelPass2, nullptr, nullptr }, posInputDesc, 1) });

	m_ShaderPrograms.at("deferredGeometryPass")->addRenderTargets({ rt_norm, rt_diff, rt_spec, rt_position });
	m_ShaderPrograms.at("deferredLightingPass")->addShaderResources
	({ {sr_norm, SHADER_TYPE::PIXEL_SHADER}, { sr_diff, SHADER_TYPE::PIXEL_SHADER }, { sr_spec, SHADER_TYPE::PIXEL_SHADER }, { sr_position, SHADER_TYPE::PIXEL_SHADER } });
	return hr;
}

void QuoteEngine::QERenderingModule::createModels()
{
	m_Models.push_back(std::make_unique<QEModel>("sphere2", "diffuseProgram"));
	m_Models.back()->setWorldMatrix(DirectX::XMMatrixScaling(0.2, 0.2, 0.2), 0);
	m_Models.push_back(std::make_unique<QEModel>("cube", "diffuseProgram"));
	m_Models.back()->setWorldMatrix(DirectX::XMMatrixTranslation(-2.0, -1.0, 0.0), 0);
	m_Models.back()->setWorldMatrix(DirectX::XMMatrixTranslation(-3.0, -1.0, 0.0), 1);
	m_Models.back()->setWorldMatrix(DirectX::XMMatrixTranslation(-4.0, -1.0, 0.0), 2);

	m_Models.push_back(std::make_unique<QEModel>("room", "diffuseProgram"));
	m_Models.back()->setWorldMatrix(DirectX::XMMatrixTranslation(0.0, 11.0, 0.0), 0);

	m_Models.push_back(std::make_unique<QEModel>("sphere2", "diffuseProgram"));
	m_Models.back()->setWorldMatrix(DirectX::XMMatrixMultiply(DirectX::XMMatrixScaling(0.1, 0.1, 0.1), DirectX::XMMatrixTranslation(0.0f, 2.0f, 1.5f)), 0);

	loadScenes();
	if (m_s_Scenes.size())
		gActiveScene = 0;
}

INT QuoteEngine::QERenderingModule::getSceneCount()
{
	return m_s_SceneCount;
}

std::vector<std::string> QuoteEngine::QERenderingModule::getModelNamesInScene(INT sceneIndex)
{
	assert(sceneIndex < m_s_SceneCount);

	if (sceneIndex < 0)
		return m_s_Scenes.at(gActiveScene)->getModelNames();
	else
		return m_s_Scenes.at(sceneIndex)->getModelNames();
}

std::shared_ptr<QEModel> QuoteEngine::QERenderingModule::getModelInActiveScene(INT modelIndex)
{
	return m_s_Scenes.at(gActiveScene)->getModels()[modelIndex];
}

QuoteEngine::QERenderingModule::~QERenderingModule()
{
}

std::unique_ptr<QuoteEngine::QEShaderProgram> QuoteEngine::QERenderingModule::createProgram(const std::string name, const std::vector<std::shared_ptr<QEShader>>& shaders, D3D11_INPUT_ELEMENT_DESC* inputElementDescriptions, const size_t numElements)
{
	std::unique_ptr<QEShaderProgram> program = std::make_unique<QEShaderProgram>();
	program->initializeShaders(shaders);
	program->initializeInputLayout(inputElementDescriptions, numElements);

	return program;
}

HRESULT QuoteEngine::QERenderingModule::createDirect3DContextAndBackbuffer(HWND wndHandle)
{
	// create a struct to hold information about the swap chain
	DXGI_SWAP_CHAIN_DESC scd;

	// clear out the struct for use
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;      // how swap chain is to be used
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
		ASSERT_SOK(hr);

		D3D11_UNORDERED_ACCESS_VIEW_DESC backbufferUAVDesc = {};
		backbufferUAVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		backbufferUAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		backbufferUAVDesc.Texture2D.MipSlice = 0;

		hr = gDevice->CreateUnorderedAccessView(pBackBuffer, &backbufferUAVDesc, gBackbufferUAV.ReleaseAndGetAddressOf());
		ASSERT_SOK(hr);

		pBackBuffer->Release();
	}
	return hr;
}

HRESULT QuoteEngine::QERenderingModule::createDepthStencilView()
{
	{
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Create the texture sampler state.
		gDevice->CreateSamplerState(&samplerDesc, gSampleState.ReleaseAndGetAddressOf());
	}
	//Describe our Depth/Stencil Buffer

	ID3D11Texture2D* pDepthStencil = NULL;

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
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
}

void QuoteEngine::QERenderingModule::createViewport()
{
	/*
	*Hardcoded resolution
	*/

	D3D11_VIEWPORT vp;
	vp.Width = (float)m_Width;
	vp.Height = (float)m_Height;
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
		static int counter = 0;                  // Display some text (you can use a format string too)
		ImGui::SliderInt("Scene", &QERenderingModule::gActiveScene, 0, QERenderingModule::getSceneCount() -1);            // Edit 1 float using a slider from 0.0f to 1.0f    

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::BulletText("counter = %d", counter);
		ImGui::Indent(10.0);
		ImGui::BulletText("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::NewLine();
		ImGui::Spacing();
		ImGui::Checkbox("Blur", &QERenderingModule::gBlur);
		std::vector<std::string> names = QERenderingModule::getModelNamesInScene(-1);
		std::vector<const char*> listbox_items;
		for (auto& name : names)
			listbox_items.push_back(name.c_str());

		static int listbox_item_current = 1;

		if (ImGui::ListBox("listbox\n(single select)", &listbox_item_current, listbox_items.data(), listbox_items.size(), 5))
		{
			QERenderingModule::gSelectedModel = listbox_item_current;
		}

		static float nudge_strength = 0.1f;
		ImGui::SliderFloat("", &nudge_strength, 0.01, 1.0);

		//Translation of active mesh (all instances)
		{
			ImGui::BulletText("Translation");
			float x = 0.0;
			float y = 0.0;
			float z = 0.0;
			ImGui::Indent(25.0);
			if (ImGui::Button("+Y", { 25.0, 25.0 }))
			{
				y += nudge_strength;
			}
			ImGui::Unindent(25.0);
			if (ImGui::Button("-X", { 25.0, 25.0 }))
			{
				x -= nudge_strength;
			}
			ImGui::SameLine(); ImGui::Indent(25.0); ImGui::Indent(25.0);
			if (ImGui::Button("+X", { 25.0, 25.0 }))
			{
				x += nudge_strength;
			}
			ImGui::Unindent(25.0);
			if (ImGui::Button("-Y", { 25.0, 25.0 }))
			{
				y -= nudge_strength;
			}

			if (x != 0.0 || y != 0.0)
				QERenderingModule::getModelInActiveScene(QERenderingModule::gSelectedModel)->move(x, y, z);
		}
		ImGui::Separator();
		//Rotation of active mesh (all instances)
		{
			float y = 0;
			ImGui::Unindent(25.0);
			ImGui::BulletText("Rotation");
			if (ImGui::Button("+rY", { 25.0, 25.0 }))
			{
				y += nudge_strength;
			}
			ImGui::SameLine();
			ImGui::Indent(50.0);
			if (ImGui::Button("-rY", { 25.0, 25.0 }))
			{
				y -= nudge_strength;
			}
			QERenderingModule::getModelInActiveScene(QERenderingModule::gSelectedModel)->rotate(0.0, y, 0.0);
		}
		ImGui::Separator();
		////Translation of active mesh (all instances)
		//{
		//	float x = 0.0;
		//	float y = 0.0;
		//	float z = 0.0;
		//	ImGui::Indent(25.0);
		//	if (ImGui::Button("+Y", { 25.0, 25.0 }))
		//	{
		//		y += nudge_strength;
		//	}
		//	ImGui::Unindent(25.0);
		//	if (ImGui::Button("-X", { 25.0, 25.0 }))
		//	{
		//		x -= nudge_strength;
		//	}
		//	ImGui::SameLine(); ImGui::Indent(25.0); ImGui::Indent(25.0);
		//	if (ImGui::Button("+X", { 25.0, 25.0 }))
		//	{
		//		x += nudge_strength;
		//	}
		//	ImGui::Unindent(25.0);
		//	if (ImGui::Button("-Y", { 25.0, 25.0 }))
		//	{
		//		y -= nudge_strength;
		//	}
		//	if (x != 0.0 || y != 0.0)
		//		QERenderingModule::getModelInActiveScene(QERenderingModule::gSelectedModel)->move(x, y, z);
		//}
		QERenderingModule::getModelInActiveScene(QERenderingModule::gSelectedModel)->Update();

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
	update({ 0.0, 0.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 0.0f });
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

DirectX::XMVECTOR QuoteEngine::Camera::getCameraPosition()
{
	return m_CamPosition;
}

DirectX::XMVECTOR QuoteEngine::Camera::getCameraTarget()
{
	return m_CamTarget;
}

void QuoteEngine::Camera::setViewMatrix(DirectX::XMMATRIX matrix)
{
	m_ViewMatrix = matrix;
}

void QuoteEngine::Camera::update(DirectX::XMVECTOR EyePosition, DirectX::XMVECTOR Focus, DirectX::XMVECTOR UpVector)
{
	m_ViewMatrix = DirectX::XMMatrixLookAtLH(EyePosition, Focus, UpVector);
	m_CamPosition = EyePosition;
	m_CamTarget = Focus;
}

void QuoteEngine::Camera::update(CameraUpdateData data)
{
	m_CamPosition = data.camPosition;
	m_CamTarget = data.camTarget;
	m_ViewMatrix = DirectX::XMMatrixLookAtLH(data.camPosition, data.camTarget, data.camUp);
}

QuoteEngine::Camera::CameraData QuoteEngine::Camera::getCameraData()
{
	return {m_CamPitch, m_CamYaw, m_CamPosition};
}

void QuoteEngine::Camera::updateCameraInformation(CameraData info)
{
	m_CamPitch = info.camPitch;
	m_CamYaw = info.camYaw;
	m_CamPosition = info.camPosition;
}

template <typename S>
std::shared_ptr<QuoteEngine::QEConstantBuffer> QuoteEngine::QERenderingModule::createConstantBuffer(S& s, QuoteEngine::SHADER_TYPE shaderType, UINT bufferIndex)
{
	std::shared_ptr<QuoteEngine::QEConstantBuffer> buffer = std::make_shared<QuoteEngine::QEConstantBuffer>
		(sizeof(S), &s, bufferIndex, shaderType);
	return buffer;
}

std::unordered_map<std::string, std::shared_ptr<QuoteEngine::QEConstantBuffer>> QuoteEngine::QERenderingModule::createConstantBufferMap(const std::vector<std::pair<std::string, std::shared_ptr<QuoteEngine::QEConstantBuffer>>>& pairs)
{
	std::unordered_map<std::string, std::shared_ptr<QuoteEngine::QEConstantBuffer>> map;
	for (auto& element : pairs)
		map.insert({ element.first, element.second });
	return map;
}

void QuoteEngine::QERenderingModule::drawInstancesDeferred(std::unique_ptr<QEModel>& model)
{
	//Get vector containing matrices for this model
	auto matrices = model->getWorldMatrices();
	//get the buffer, stride, offset and vertex count
	ID3D11Buffer* buffer = model->getVertexBuffer();
	UINT stride = model->getStrideInBytes();
	UINT offset = 0;
	UINT vertexCount = model->getVertexCount();

	//Get per-frame matrices
	auto ViewMatrix = QERenderingModule::gCamera.getViewMatrix();
	auto ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(1.2, (float)m_Width / (float)m_Height, 0.1f, 100.f);

	//Set vertex buffer and sampler
	gDeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	gDeviceContext->PSSetSamplers(0, 1, gSampleState.GetAddressOf());

	for (int i = 0; i < matrices.size(); i++)
	{
		//Update VS cbuffer
		{
			QuoteEngine::CB_PerModel_WVP_W perModel = {};
			auto WVP = matrices[i];
			DirectX::XMStoreFloat4x4(&perModel._W, WVP);
			WVP = DirectX::XMMatrixMultiply(WVP, DirectX::XMMatrixMultiply(ViewMatrix, ProjectionMatrix));
			DirectX::XMStoreFloat4x4(&perModel._WVP, WVP);

			m_ShaderPrograms.at("deferredGeometryPass")->updateBuffer("WVP_W", &perModel);
		}

		//Update PS cbuffers
		{
			QEModel::QEMaterial material = model->getMaterial();
			m_ShaderPrograms.at("deferredGeometryPass")->updateBuffer("MATERIAL", &material);
		}

		//Bind texture, set vbuffer and draw
		{
			//set the buffer and draw model
			model->bindTexture();
			gDeviceContext->Draw(vertexCount, 0);
		}
	}
}
void QuoteEngine::QERenderingModule::drawInstancesDeferred(std::shared_ptr<QEModel>& model)
{
	//Get vector containing matrices for this model
	auto matrices = model->getWorldMatrices();
	//get the buffer, stride, offset and vertex count
	ID3D11Buffer* buffer = model->getVertexBuffer();
	UINT stride = model->getStrideInBytes();
	UINT offset = 0;
	UINT vertexCount = model->getVertexCount();

	//Get per-frame matrices
	auto ViewMatrix = QERenderingModule::gCamera.getViewMatrix();
	auto ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(1.2, (float)m_Width / (float)m_Height, 0.1f, 100.f);

	//Set vertex buffer and sampler
	gDeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	gDeviceContext->PSSetSamplers(0, 1, gSampleState.GetAddressOf());

	for (int i = 0; i < matrices.size(); i++)
	{
		//Update VS cbuffer
		{
			QuoteEngine::CB_PerModel_WVP_W perModel = {};
			auto WVP = matrices[i];
			DirectX::XMStoreFloat4x4(&perModel._W, WVP);
			WVP = DirectX::XMMatrixMultiply(WVP, DirectX::XMMatrixMultiply(ViewMatrix, ProjectionMatrix));
			DirectX::XMStoreFloat4x4(&perModel._WVP, WVP);

			m_ShaderPrograms.at("deferredGeometryPass")->updateBuffer("WVP_W", &perModel);
		}

		//Update PS cbuffers
		{
			QEModel::QEMaterial material = model->getMaterial();
			m_ShaderPrograms.at("deferredGeometryPass")->updateBuffer("MATERIAL", &material);
		}

		//Bind texture, set vbuffer and draw
		{
			//set the buffer and draw model
			model->bindTexture();
			gDeviceContext->Draw(vertexCount, 0);
		}
	}
}

void QuoteEngine::QERenderingModule::loadScenes()
{
	//TODO Move out of QEScene into QERenderingModule

	std::ifstream SceneFile;
	//Open file
	SceneFile.open("scenes.txt");

	assert(SceneFile.is_open() && "scene file not open");

	{
		std::string input;
		std::string garbage;
		SceneFile >> input;
		while (!SceneFile.eof())
		{
			if (input == "scene")
			{
				SceneFile >> input;
				m_s_Scenes.push_back(std::make_unique<QEScene>(std::stoi(input)));
				m_s_SceneCount++;
				while (input != "scene")
				{
					if (SceneFile.eof())
						return;
					SceneFile >> input;
					if (input == "model")
					{
						std::string modelName = "";
						std::string shaderProgram = "";
						UINT numInstances;
						SceneFile >> modelName >> shaderProgram >> garbage >> numInstances;

						std::vector<DirectX::XMMATRIX> matrices;
						float t[3] = {};
						float r[3] = {};
						float s[3] = {};
						for (UINT i = 0; i < numInstances; i++)
						{
							SceneFile >> garbage >> t[0] >> t[1] >> t[2];
							SceneFile >> garbage >> r[0] >> r[1] >> r[2];
							SceneFile >> garbage >> s[0] >> s[1] >> s[2];

							//construct matrices
							auto matrixT = DirectX::XMMatrixTranslation(t[0], t[1], t[2]);
							auto matrixR = DirectX::XMMatrixRotationRollPitchYaw(r[0], r[1], r[2]);
							auto matrixS = DirectX::XMMatrixScaling(s[0], s[1], s[2]);
							auto finalMatrix = DirectX::XMMatrixMultiply(matrixT, DirectX::XMMatrixMultiply(matrixR, matrixS));
							matrices.push_back(finalMatrix);

						}
						m_s_Scenes.back()->addModel(modelName, shaderProgram, matrices);
						m_s_Scenes.back()->getModels().back()->setRawTransformations(t, r, s);
					}
				}
			}
		}
	}

	return;
}
