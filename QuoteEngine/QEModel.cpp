#include "QEModel.h"
#include "QERenderingModule.h"


QEModel::QEModel()
{
	m_VertexBuffer.Reset(); //clean ComPtr

	create();
}


QEModel::~QEModel()
{
}

HRESULT QEModel::create()
{
	struct TriangleVertex
	{
		float x, y, z;
		float r, g, b;
	};

	// Array of Structs (AoS)
	TriangleVertex triangleVertices[3] =
	{
		0.0f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		0.5f, -0.5f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		-0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f	//v2 color
	};

	// Describe the Vertex Buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	// what type of buffer will this be?
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// what type of usage (press F1, read the docs)
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	bufferDesc.ByteWidth = sizeof(triangleVertices);

	// this struct is created just to set a pointer to the
	// data containing the vertices.
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = triangleVertices;

	// create a Vertex Buffer
	QERenderingModule::gDevice->CreateBuffer(&bufferDesc, &data, m_VertexBuffer.ReleaseAndGetAddressOf());
	return E_NOTIMPL;
}
