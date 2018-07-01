#include "QEShader.h"



QuoteEngine::QEShader::QEShader()
{
}


QuoteEngine::QEShader::~QEShader()
{
}

void QuoteEngine::QEShader::addConstantBuffers(std::vector<std::pair<unsigned int, QEConstantBuffer*>> buffers)
{
	for (auto pair : buffers)
		m_ConstantBuffers.push_back(pair);
}
