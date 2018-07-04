#include "QEConstantBuffer.h"
#include "QERenderingModule.h"

QuoteEngine::QEConstantBuffer::QEConstantBuffer(size_t bufferSize, void * initialData, UINT shaderRegister, QuoteEngine::SHADER_TYPE shaderType)
	: m_BufferSize(bufferSize), m_ShaderRegister(shaderRegister), m_ShaderType(shaderType)
{
	//Reset the ComPtr
	m_Buffer.Reset();

	m_Data = new BYTE[bufferSize];

	if (initialData)
		memcpy(m_Data, initialData, m_BufferSize);


	//Create the buffer.
	HRESULT hr = create();
}

QuoteEngine::QEConstantBuffer::~QEConstantBuffer()
{
	delete[] m_Data;
}

ID3D11Buffer * QuoteEngine::QEConstantBuffer::getBuffer()
{
	return m_Buffer.Get();
}

UINT QuoteEngine::QEConstantBuffer::getRegister()
{
	return m_ShaderRegister;
}

QuoteEngine::SHADER_TYPE QuoteEngine::QEConstantBuffer::getShaderType()
{
	return m_ShaderType;
}

HRESULT QuoteEngine::QEConstantBuffer::create()
{
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = m_BufferSize + 16 - (m_BufferSize % 16); //this is incorrect but shouldnt matter..
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Data;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	return QERenderingModule::gDevice->CreateBuffer(&cbDesc, &InitData, m_Buffer.ReleaseAndGetAddressOf());
}
