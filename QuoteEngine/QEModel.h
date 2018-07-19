#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h> //ComPtr
#include <string>

namespace QuoteEngine
{
	struct Vertex_pos3
	{
		float x;
		float y;
		float z;
	};

	struct Vertex_uv2
	{
		float u;
		float v;
	};

	struct Vertex_pos3nor3uv2
	{
		float posX;
		float posY;
		float posZ;
		float norX;
		float norY;
		float norZ;
		float u;
		float v;
	};
}

class QEModel
{
public:
	QEModel(); //Default ctor for now, everything will be hard coded for the time being
	QEModel(std::string file) noexcept;
	~QEModel();
	ID3D11Buffer* getVertexBuffer();
	UINT getSizeInBytes();
	UINT getStrideInBytes();
	UINT getVertexCount();
	std::string getAssociatedShaderProgram();

	DirectX::XMMATRIX getWorldMatrix();
	void setWorldMatrix(DirectX::XMMATRIX);
	void setAssociatedShaderProgram(std::string program);
	bool hasAssociatedShaderProgram();
private:

	std::string m_AssociatedShaderProgramName = "none";

	UINT m_SizeInBytes = 0;
	UINT m_VertexCount = 0;
	UINT m_StrideInBytes = 0;
	DirectX::XMMATRIX m_WorldMatrix;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;

	HRESULT create();
};

inline UINT QEModel::getStrideInBytes()
{
	return m_StrideInBytes;
}

inline ID3D11Buffer * QEModel::getVertexBuffer()
{
	return m_VertexBuffer.Get();
}

inline UINT QEModel::getSizeInBytes()
{
	return m_SizeInBytes;
}

inline UINT QEModel::getVertexCount()
{
	return m_VertexCount;
}

inline DirectX::XMMATRIX QEModel::getWorldMatrix()
{
	return m_WorldMatrix;
}

inline void QEModel::setWorldMatrix(DirectX::XMMATRIX matrix)
{
	m_WorldMatrix = matrix;
}
