#pragma once
#include <d3d11.h>
#include <vector>
#include "QEConstantBuffer.h"

class QETexture;

namespace QuoteEngine
{
	enum class SHADER_TYPE
	{
		VERTEX_SHADER,
		PIXEL_SHADER,
		GEOMETRY_SHADER,
		COMPUTE_SHADER
	};

	class QEShader
	{
	public:
		QEShader();
		~QEShader();

		HRESULT compileFromFile(QuoteEngine::SHADER_TYPE type, LPCWSTR file);
		HRESULT bindShader();
		HRESULT bindShaderAndResources();

	protected:
		//Adds constant buffers to m_ConstantBuffers 
		void addConstantBuffers(std::vector<std::pair<unsigned int, QEConstantBuffer*>>);

		//Adds textures to m_Textures
		void addTextures(std::vector<std::pair<unsigned int, QETexture*>>);

	private:
		SHADER_TYPE m_Type = SHADER_TYPE::VERTEX_SHADER;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_ComputeShader;

		std::vector<std::pair<unsigned int, QEConstantBuffer*>>	m_ConstantBuffers;
		std::vector<std::pair<unsigned int, QETexture*>>	m_Textures;

		std::vector<ID3D11Buffer*> m_RawBuffers;

		//Binds resources to the currently bound shader of the passed in shader type
		HRESULT bindResources();

	};

	class QEShaderProgram
	{
	public:
		QEShaderProgram();
		~QEShaderProgram();

		HRESULT initializeShaders(std::vector<QEShader*>&);
		HRESULT initializeInputLayout(const D3D11_INPUT_ELEMENT_DESC*, const UINT numElements);
		void bind();
	private:
		std::vector<QEShader*> m_Shaders;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	};


}
