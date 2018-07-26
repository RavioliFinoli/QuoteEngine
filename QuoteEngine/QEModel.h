#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h> //ComPtr
#include <string>
#include <vector>
#include "QETexture.h"

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
	struct QEMaterial
	{
		float KsR, KsG, KsB, Ns;			//specular color + power
		float KdR, KdG, KdB, UseTexture;	//diffuse color  + useTexture 'boolean'
		float KaR, KaG, KaB, pad2;			//ambient color
	};

	QEModel(); //Default ctor for now, everything will be hard coded for the time being
	QEModel(std::string file) noexcept;
	QEModel(std::string file, std::string shaderProgram) noexcept;
	~QEModel();
	ID3D11Buffer* getVertexBuffer();
	UINT getSizeInBytes();
	UINT getStrideInBytes();
	UINT getVertexCount();
	std::string getAssociatedShaderProgram();
	QEMaterial getMaterial();
	void Update();
	DirectX::XMMATRIX getWorldMatrix();
	std::vector<DirectX::XMMATRIX>& getWorldMatrices();
	void setWorldMatrix(DirectX::XMMATRIX worldMatrix, UINT instance);
	void move(float x, float y, float z);
	void rotate(float x, float y, float z);
	void setAssociatedShaderProgram(std::string program);
	void setRawTransformations(float* t, float* r, float* s);
	void bindTexture();
	bool hasAssociatedShaderProgram();
private:

	std::string m_AssociatedShaderProgramName = "none";

	UINT m_SizeInBytes = 0;
	UINT m_VertexCount = 0;
	UINT m_StrideInBytes = 0;
	std::vector<DirectX::XMMATRIX> m_WorldMatrices;
	QEMaterial m_Material = {};

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	std::unique_ptr<QuoteEngine::QETexture> m_Texture;
	float m_Translation[3];
	float m_Rotation[3];
	float m_Scale[3];


	HRESULT create();
	void loadFromFile(std::string file);
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
	if (m_WorldMatrices.size())
		return m_WorldMatrices[0];
	else
		return DirectX::XMMatrixIdentity();
}
