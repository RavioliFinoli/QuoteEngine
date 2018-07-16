#pragma once
#include <d3d11.h>
#include <vector>
#include "QEConstantBuffer.h"
#include "QuoteEngineCommon.h"
#include <unordered_map>
#include <string>
class QETexture;


namespace QuoteEngine
{
	class QEShader
	{
	public:
		QEShader();
		~QEShader();

		HRESULT compileFromFile(QuoteEngine::SHADER_TYPE type, LPCWSTR file);
		HRESULT bindShader();
		HRESULT bindShaderAndResources();
		ID3DBlob* getVSBlob();
		std::unordered_map<std::string, QEConstantBuffer*>* getBuffers();

		//Adds constant buffers to m_ConstantBuffers 
		void addConstantBuffers(std::unordered_map<std::string ,QEConstantBuffer*>&);

		//Adds textures to m_Textures
		void addTextures(std::vector<QETexture*>);

		void updateBuffer(std::string key, PVOID value);
	private:
		SHADER_TYPE m_Type = SHADER_TYPE::VERTEX_SHADER;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_ComputeShader;
		Microsoft::WRL::ComPtr<ID3DBlob> m_VSBlob;

		std::unordered_map < std::string, QEConstantBuffer* > m_ConstantBuffers;
		std::vector<QETexture*>	m_Textures;

		std::vector<ID3D11Buffer*> m_RawBuffers;

		//Binds resources to the currently bound shader of the passed in shader type
		HRESULT bindResources();

	};

	class QEShaderProgram
	{
	public:

		struct OffsetStride
		{
			UINT offset;
			UINT stride;
		};

		QEShaderProgram();
		~QEShaderProgram();

		HRESULT initializeShaders(const std::vector<QEShader*>&);
		HRESULT initializeInputLayout(const D3D11_INPUT_ELEMENT_DESC*, const UINT numElements);
		HRESULT updateBuffer(std::string key, PVOID data);
		void bind();

	private:
		std::vector<QEShader*> m_Shaders;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
		std::unordered_map<std::string, QEConstantBuffer*> m_ConstantBuffers;
	};


}
