#include "QEShader.h"
#include "QERenderingModule.h"


QuoteEngine::QEShader::QEShader()
{
}


QuoteEngine::QEShader::~QEShader()
{
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

HRESULT QuoteEngine::QEShader::bindResources(SHADER_TYPE type)
{
	//Bind constant buffers
	switch (type)
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
