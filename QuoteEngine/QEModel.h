#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h> //ComPtr

class QEModel
{
public:
	QEModel(); //Default ctor for now, everything will be hardcoded for the time being
	~QEModel();
	ID3D11Buffer* getVertexBuffer();
	UINT getSizeInBytes();
	UINT getStrideInBytes();
	UINT getVertexCount();

	DirectX::XMMATRIX getWorldMatrix();
	void setWorldMatrix(DirectX::XMMATRIX);
private:

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
