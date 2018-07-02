#include "QEShader.h"
#include "QERenderingModule.h"
#include <d3dcompiler.h>
using Microsoft::WRL::ComPtr;

QuoteEngine::QEShader::QEShader()
{
	m_VertexShader.Reset();
	m_PixelShader.Reset();
	m_GeometryShader.Reset();
	m_ComputeShader.Reset();
}


QuoteEngine::QEShader::~QEShader()
{
}

HRESULT QuoteEngine::QEShader::compileFromFile(QuoteEngine::SHADER_TYPE type, LPCWSTR file)
{
	m_Type = type;

	std::string entry = "";
	std::string shaderModel = "";

	switch (m_Type)
	{
	case QuoteEngine::SHADER_TYPE::VERTEX_SHADER:
		entry = "VS_main";
		shaderModel = "vs_5_0";
		break;
	case QuoteEngine::SHADER_TYPE::PIXEL_SHADER:
		entry = "PS_main";
		shaderModel = "ps_5_0";
		break;
	case QuoteEngine::SHADER_TYPE::GEOMETRY_SHADER:
		entry = "GS_main";
		shaderModel = "gs_5_0";
		break;
	case QuoteEngine::SHADER_TYPE::COMPUTE_SHADER:
		entry = "CS_main";
		shaderModel = "cs_5_0";
		break;
	default:
		break;
	}
	
	// Binary Large OBject (BLOB), for compiled shader, and errors.
	ComPtr<ID3DBlob> pVS;
	ComPtr<ID3DBlob> errorBlob;

	// https://msdn.microsoft.com/en-us/library/windows/desktop/hh968107(v=vs.85).aspx
	HRESULT result = D3DCompileFromFile(
		file, // filename
		nullptr,		// optional macros
		nullptr,		// optional include files
		entry.c_str(),		// entry point
		shaderModel.c_str(),		// shader model (target)
		D3DCOMPILE_DEBUG,	// shader compile options (DEBUGGING)
		0,				// IGNORE...DEPRECATED.
		pVS.ReleaseAndGetAddressOf(),			// double pointer to ID3DBlob		
		errorBlob.ReleaseAndGetAddressOf()		// pointer for Error Blob messages.
	);

	// compilation failed?
	if (FAILED(result))
	{
		if (errorBlob.Get())
		{
			OutputDebugStringA((char*)errorBlob.Get()->GetBufferPointer());
		}
		return result;
	}


	switch (m_Type)
	{
	case QuoteEngine::SHADER_TYPE::VERTEX_SHADER:
		result = QERenderingModule::gDevice->CreateVertexShader(
			pVS->GetBufferPointer(),
			pVS->GetBufferSize(),
			nullptr,
			m_VertexShader.ReleaseAndGetAddressOf()
		);
		pVS.Swap(m_VSBlob);
		break;
	case QuoteEngine::SHADER_TYPE::PIXEL_SHADER:
		result = QERenderingModule::gDevice->CreatePixelShader(
			pVS->GetBufferPointer(),
			pVS->GetBufferSize(),
			nullptr,
			m_PixelShader.ReleaseAndGetAddressOf()
		);
		break;
	case QuoteEngine::SHADER_TYPE::GEOMETRY_SHADER:
		result = QERenderingModule::gDevice->CreateGeometryShader(
			pVS->GetBufferPointer(),
			pVS->GetBufferSize(),
			nullptr,
			m_GeometryShader.ReleaseAndGetAddressOf()
		);
		break;
	case QuoteEngine::SHADER_TYPE::COMPUTE_SHADER:
		result = QERenderingModule::gDevice->CreateComputeShader(
			pVS->GetBufferPointer(),
			pVS->GetBufferSize(),
			nullptr,
			m_ComputeShader.ReleaseAndGetAddressOf()
		);
		break;
	default:
		result = E_FAIL;
		break;
	}

	return result;
}

HRESULT QuoteEngine::QEShader::bindShaderAndResources()
{
	switch (m_Type)
	{
	case QuoteEngine::SHADER_TYPE::VERTEX_SHADER:
		QERenderingModule::gDeviceContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		break;
	case QuoteEngine::SHADER_TYPE::PIXEL_SHADER:
		QERenderingModule::gDeviceContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
		break;
	case QuoteEngine::SHADER_TYPE::GEOMETRY_SHADER:
		QERenderingModule::gDeviceContext->GSSetShader(m_GeometryShader.Get(), nullptr, 0);
		break;
	case QuoteEngine::SHADER_TYPE::COMPUTE_SHADER:
		QERenderingModule::gDeviceContext->CSSetShader(m_ComputeShader.Get(), nullptr, 0);
		break;
	default:
		break;
	}

	bindResources();

	return E_NOTIMPL;
}

ID3DBlob * QuoteEngine::QEShader::getVSBlob()
{
	return m_VSBlob.Get();
}

void QuoteEngine::QEShader::addConstantBuffers(std::vector<std::pair<unsigned int, QEConstantBuffer*>> buffers)
{
	//whenever we add buffers, we add the ID3D11Buffer pointers to a vector for easier binding. They are ordered 
	//by their slot in the shader. Gaps in this vector is not allowed, thus the shader should not have gaps in 
	//their occupied cbuffer slots.
	m_ConstantBuffers.clear();
	m_ConstantBuffers.resize(buffers.size());

	for (auto pair : buffers)
	{
		m_ConstantBuffers.push_back(pair);

		m_RawBuffers[pair.first] = pair.second->get();
	}

	//Check integrity of m_RawBuffers
	//TODO
}

void QuoteEngine::QEShader::addTextures(std::vector<std::pair<unsigned int, QETexture*>> textures)
{
	for (auto pair : textures)
		m_Textures.push_back(pair);
}

HRESULT QuoteEngine::QEShader::bindResources()
{
	if (!m_RawBuffers.size())
		return E_NOTIMPL;
	//Bind constant buffers
	switch (m_Type)
	{
	case QuoteEngine::SHADER_TYPE::VERTEX_SHADER:
		QERenderingModule::gDeviceContext->VSSetConstantBuffers(0, m_RawBuffers.size(), &m_RawBuffers[0]);
		break;
	case QuoteEngine::SHADER_TYPE::PIXEL_SHADER:
		QERenderingModule::gDeviceContext->PSSetConstantBuffers(0, m_RawBuffers.size(), &m_RawBuffers[0]);
		break;
	case QuoteEngine::SHADER_TYPE::GEOMETRY_SHADER:
		QERenderingModule::gDeviceContext->GSSetConstantBuffers(0, m_RawBuffers.size(), &m_RawBuffers[0]);
		break;
	case QuoteEngine::SHADER_TYPE::COMPUTE_SHADER:
		QERenderingModule::gDeviceContext->CSSetConstantBuffers(0, m_RawBuffers.size(), &m_RawBuffers[0]);
		break;
	default:
		break;
	}
	return E_NOTIMPL;
}


QuoteEngine::QEShaderProgram::QEShaderProgram()
{
}

QuoteEngine::QEShaderProgram::~QEShaderProgram()
{
}

HRESULT QuoteEngine::QEShaderProgram::initializeShaders(const std::vector<QEShader*>& shaders)
{
	if (shaders.size() != 4)
		return E_INVALIDARG;

	m_Shaders.resize(4);
	
	for (int i = 0; i < 4; i++)
		m_Shaders[i] = shaders[i];

	return S_OK;
}

HRESULT QuoteEngine::QEShaderProgram::initializeInputLayout(const D3D11_INPUT_ELEMENT_DESC * inputDesc, const UINT numElements)
{
	return QERenderingModule::gDevice->CreateInputLayout(inputDesc, numElements, m_Shaders[0]->getVSBlob()->GetBufferPointer(), m_Shaders[0]->getVSBlob()->GetBufferSize(), m_InputLayout.ReleaseAndGetAddressOf());
}

void QuoteEngine::QEShaderProgram::bind()
{
	for (auto shader : m_Shaders)
	{
		if (shader)
			shader->bindShaderAndResources();
	}

	QERenderingModule::gDeviceContext->IASetInputLayout(m_InputLayout.Get());
}
