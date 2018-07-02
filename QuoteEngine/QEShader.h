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

		HRESULT compileFromFile();
		HRESULT bindShader();
		HRESULT bindShaderAndResources();

	protected:
		//Adds constant buffers to m_ConstantBuffers 
		void addConstantBuffers(std::vector<std::pair<unsigned int, QEConstantBuffer*>>);

		//Adds textures to m_Textures
		void addTextures(std::vector<std::pair<unsigned int, QETexture*>>);

	private:
		std::vector<std::pair<unsigned int, QEConstantBuffer*>>	m_ConstantBuffers;
		std::vector<std::pair<unsigned int, QETexture*>>	m_Textures;

		std::vector<ID3D11Buffer*> m_RawBuffers;

		//Binds resources to the currently bound shader of the passed in shader type
		HRESULT bindResources(SHADER_TYPE);
	};
}
