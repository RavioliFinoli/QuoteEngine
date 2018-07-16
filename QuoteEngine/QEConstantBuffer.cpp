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

void QuoteEngine::QEConstantBuffer::update(PVOID data)
{
	//HRESULT hr = S_OK;
	//D3D11_MAPPED_SUBRESOURCE subresource = {};
	//subresource.pData = data;
	//hr = QuoteEngine::QERenderingModule::gDeviceContext->Map(
	//	m_Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource
	//);

	//QuoteEngine::QERenderingModule::gDeviceContext->Unmap(m_Buffer.Get(), 0);

	QuoteEngine::QERenderingModule::gDeviceContext->UpdateSubresource(m_Buffer.Get(), 0, NULL, data, 0, 0);
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
	cbDesc.ByteWidth = (m_BufferSize + 16 - 1) & -16;
	cbDesc.Usage = D3D11_USAGE_DEFAULT;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = 0;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = m_Data;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	return QERenderingModule::gDevice->CreateBuffer(&cbDesc, &InitData, m_Buffer.ReleaseAndGetAddressOf());
}
