#include "QEConstantBuffer.h"
#include "QERenderingModule.h"

QuoteEngine::QEConstantBuffer::QEConstantBuffer(size_t bufferSize, void * initialData) 
	: m_BufferSize(bufferSize)
{
	//Reset the ComPtr
	m_Buffer.Reset();

	m_Data = new BYTE[bufferSize];

	//Create the buffer.
	create();
}

QuoteEngine::QEConstantBuffer::~QEConstantBuffer()
{
	delete[] m_Data;
}

ID3D11Buffer * QuoteEngine::QEConstantBuffer::get()
{
	return m_Buffer.Get();
}

HRESULT QuoteEngine::QEConstantBuffer::create()
{
	D3D11_BUFFER_DESC cbDesc = {};
	cbDesc.ByteWidth = m_BufferSize;
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
