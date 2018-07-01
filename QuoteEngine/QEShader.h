#pragma once
#include <d3d11.h>
#include <vector>

class QEConstantBuffer;
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
		virtual ~QEShader();

		virtual HRESULT compileFromFile() = 0;
		virtual HRESULT bindShader() = 0;
		virtual HRESULT bindShaderAndResources() = 0;

	protected:
		//Adds constant buffers to m_ConstantBuffers 
		void addConstantBuffers(std::vector<std::pair<unsigned int, QEConstantBuffer*>>);

		//Adds textures to m_Textures
		void addTextures(std::vector<std::pair<unsigned int, QETexture*>>);

	private:
		std::vector<std::pair<unsigned int, QEConstantBuffer*>>	m_ConstantBuffers;
		std::vector<std::pair<unsigned int, QETexture*>>	m_Textures;

		//Binds resources to the currently bound shader of the passed in shader type
		HRESULT bindResources(SHADER_TYPE);
	};

	class QEVertexShader : QEShader
	{
	public:
		QEVertexShader();
		~QEVertexShader();
	private:
	};

	class QEPixelShader : QEShader
	{
	public:
		QEPixelShader();
		~QEPixelShader();
	private:

	};

	class QEGeometryShader : QEShader
	{
	public:
		QEGeometryShader();
		~QEGeometryShader();
	private:

	};

	class QEComputeShader : QEShader
	{
	public:
		QEComputeShader();
		~QEComputeShader();
	private:

	};
}
