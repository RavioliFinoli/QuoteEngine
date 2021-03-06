#pragma once
#include <d3d11.h>
#include <vector>
#include "QEConstantBuffer.h"
#include "QuoteEngineCommon.h"
#include <unordered_map>
#include <string>
class QETexture;
using Microsoft::WRL::ComPtr;

namespace QuoteEngine
{
	class QEShaderPass
	{
	public:
		QEShaderPass();
		~QEShaderPass();

	private:
		std::vector<ComPtr<ID3D11RenderTargetView>> m_RenderTargets;
		std::vector<ComPtr<ID3D11ShaderResourceView>> m_ShaderResources;

		std::vector<std::shared_ptr<QEConstantBuffer>> m_ConstantBuffers;
		std::vector<ID3D11Buffer*> m_RawBuffers;

	};

	class QEShader
	{
	public:
		QEShader();
		~QEShader();

		HRESULT compileFromFile(QuoteEngine::SHADER_TYPE type, LPCWSTR file);
		HRESULT bindShaderAndResources();
		ID3DBlob* getVSBlob();
		std::unordered_map<std::string, std::shared_ptr<QEConstantBuffer>>* getBuffers();

		//Adds constant buffers to m_ConstantBuffers 
		void addConstantBuffers(std::unordered_map<std::string , std::shared_ptr<QEConstantBuffer>>& buffers);
		void updateBuffer(std::string key, PVOID value);

	private:
		SHADER_TYPE m_Type = SHADER_TYPE::VERTEX_SHADER;

		ComPtr<ID3D11VertexShader> m_VertexShader;
		ComPtr<ID3D11PixelShader> m_PixelShader;
		ComPtr<ID3D11GeometryShader> m_GeometryShader;
		ComPtr<ID3D11ComputeShader> m_ComputeShader;
		ComPtr<ID3DBlob> m_VSBlob;

		std::unordered_map < std::string, std::shared_ptr<QEConstantBuffer>> m_ConstantBuffers;
		std::vector<ID3D11Buffer*> m_RawBuffers;

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

		HRESULT initializeShaders(const std::vector<std::shared_ptr<QEShader>>&);
		HRESULT initializeInputLayout(const D3D11_INPUT_ELEMENT_DESC*, const UINT numElements);
		HRESULT updateBuffer(std::string key, PVOID data);
		void bind();
		void unbind();
		void addRenderTargets(std::vector<ComPtr<ID3D11RenderTargetView>> renderTargets);
		void addShaderResources(std::vector<std::pair<ComPtr<ID3D11ShaderResourceView>, SHADER_TYPE>> shaderResources);
		void clearRenderTargets(bool value = true);
	private:
		std::vector<std::shared_ptr<QEShader>> m_Shaders;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
		std::unordered_map<std::string, std::shared_ptr<QEConstantBuffer>> m_ConstantBuffers;

		//----
		std::vector<ComPtr<ID3D11RenderTargetView>> m_RenderTargets;
		std::vector<std::pair<ComPtr<ID3D11ShaderResourceView>, SHADER_TYPE>> m_ShaderResources;
		bool m_ClearRenderTargets = true;

		//Binds resources to the currently bound shader of the passed in shader type
		HRESULT bindShaderResourceViews();
		HRESULT bindRenderTargetViews();
	};


}
