#include "QEShader.h"
#include "QERenderingModule.h"
#include <d3dcompiler.h>

using Microsoft::WRL::ComPtr;
using QuoteEngine::QERenderingModule;

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
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,	// shader compile options (DEBUGGING)
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

inline ID3DBlob * QuoteEngine::QEShader::getVSBlob()
{
	return m_VSBlob.Get();
}

std::unordered_map<std::string, std::shared_ptr<QuoteEngine::QEConstantBuffer>>* QuoteEngine::QEShader::getBuffers()
{
	return &m_ConstantBuffers;
}

void QuoteEngine::QEShader::addConstantBuffers(std::unordered_map<std::string , std::shared_ptr<QEConstantBuffer>>& buffers)
{
	//whenever we add buffers, we add the ID3D11Buffer pointers to a vector for easier binding. They are ordered 
	//by their slot in the shader. Gaps in this vector is not allowed, thus the shader should not have gaps in 
	//their occupied cbuffer slots.
	m_ConstantBuffers.clear();

	for (auto buffer : buffers)
	{
		m_ConstantBuffers.insert(buffer);

		m_RawBuffers.push_back(buffer.second->getBuffer());
	}

	//Check integrity of m_RawBuffers
	//TODO
}

void QuoteEngine::QEShader::updateBuffer(std::string key, PVOID value)
{
	m_ConstantBuffers.at(key)->update(value);
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

HRESULT QuoteEngine::QEShaderProgram::bindShaderResourceViews()
{
	for (int i = 0; i < m_ShaderResources.size(); ++i)
	{
		switch (m_ShaderResources[i].second /*Shader type*/)
		{
		case SHADER_TYPE::PIXEL_SHADER:
		{
			QERenderingModule::gDeviceContext->PSSetShaderResources(i, 1, m_ShaderResources[i].first.GetAddressOf());
			break;
		}
		case SHADER_TYPE::VERTEX_SHADER:
		{
			QERenderingModule::gDeviceContext->VSSetShaderResources(i, 1, m_ShaderResources[i].first.GetAddressOf());
			break;
		}
		case SHADER_TYPE::GEOMETRY_SHADER:
		{
			QERenderingModule::gDeviceContext->GSSetShaderResources(i, 1, m_ShaderResources[i].first.GetAddressOf());
			break;
		}
		case SHADER_TYPE::COMPUTE_SHADER:
		{
			QERenderingModule::gDeviceContext->CSSetShaderResources(i, 1, m_ShaderResources[i].first.GetAddressOf());
			break;
		}
		default:
			break;
		}
	}
	return S_OK;
}

HRESULT QuoteEngine::QEShaderProgram::bindRenderTargetViews()
{
	float clearColor[4] = { 1.0, 1.0, 1.0, 1.0 };
	std::vector<ID3D11RenderTargetView*> v;
	for (auto& rt : m_RenderTargets)
	{
		QERenderingModule::gDeviceContext->ClearRenderTargetView(rt.Get(), clearColor);
		v.push_back(rt.Get());
	}
	QERenderingModule::gDeviceContext->OMSetRenderTargets(m_RenderTargets.size(), v.data(), QERenderingModule::gDepthStencilView.Get());
	return S_OK;
}

QuoteEngine::QEShaderProgram::QEShaderProgram()
{
}

QuoteEngine::QEShaderProgram::~QEShaderProgram()
{
}

void QuoteEngine::QEShaderProgram::addRenderTargets(std::vector<ComPtr<ID3D11RenderTargetView>> renderTargets)
{
	m_RenderTargets = renderTargets;
}

void QuoteEngine::QEShaderProgram::addShaderResources(std::vector<std::pair<ComPtr<ID3D11ShaderResourceView>, SHADER_TYPE>> shaderResources)
{
	m_ShaderResources = shaderResources;
}

void QuoteEngine::QEShaderProgram::clearRenderTargets(bool value /*= true*/)
{
	m_ClearRenderTargets = value;
}

HRESULT QuoteEngine::QEShaderProgram::initializeShaders(const std::vector<std::shared_ptr<QEShader>>& shaders)
{
	if (shaders.size() != 4)
		return E_INVALIDARG;

	m_Shaders.resize(4);
	
	for (int i = 0; i < 4; i++)
		m_Shaders[i] = shaders[i];

	//Fetch constant buffers from shaders
	for (auto shader : m_Shaders)
	{
		if (shader)
		{
			std::unordered_map<std::string, std::shared_ptr<QEConstantBuffer>>* pBufferMap = shader->getBuffers();
			for (auto buffer : *pBufferMap)
			{
				m_ConstantBuffers.insert({ buffer.first, buffer.second });
			}
		}
	}

	return S_OK;
}

HRESULT QuoteEngine::QEShaderProgram::initializeInputLayout(const D3D11_INPUT_ELEMENT_DESC * inputDesc, const UINT numElements)
{
	if (inputDesc)
		return QERenderingModule::gDevice->CreateInputLayout(inputDesc, numElements, m_Shaders[0]->getVSBlob()->GetBufferPointer(), m_Shaders[0]->getVSBlob()->GetBufferSize(), m_InputLayout.ReleaseAndGetAddressOf());
	else
	{
		ID3D11InputLayout * nullLayout = { nullptr };
		m_InputLayout = nullLayout;
		return E_NOTIMPL;
	}
}

HRESULT QuoteEngine::QEShaderProgram::updateBuffer(std::string key, PVOID data)
{
	std::shared_ptr<QEConstantBuffer> buffer = nullptr;
	try
	{
		buffer = m_ConstantBuffers.at(key);
	}
	catch (std::out_of_range e)
	{
		return E_INVALIDARG;
	}

	if (data)
	{
		m_ConstantBuffers.at(key)->update(data);
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

void QuoteEngine::QEShaderProgram::bind()
{
	if (m_ShaderResources.size())
		bindShaderResourceViews();
	if (m_RenderTargets.size())
		bindRenderTargetViews();

	for (auto shader : m_Shaders)
	{
		if (shader)
			shader->bindShaderAndResources();
	}

	QERenderingModule::gDeviceContext->IASetInputLayout(m_InputLayout.Get());
}

void QuoteEngine::QEShaderProgram::unbind()
{
	ID3D11RenderTargetView* nullTargets[10] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	ID3D11ShaderResourceView* nullResources[10] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	QERenderingModule::gDeviceContext->OMSetRenderTargets(m_RenderTargets.size(), nullTargets, QERenderingModule::gDepthStencilView.Get());
	QERenderingModule::gDeviceContext->PSSetShaderResources(0, m_ShaderResources.size(), nullResources);
}
