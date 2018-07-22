#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace QuoteEngine
{
	class QETexture
	{
	public:
		QETexture(LPCWSTR file);
		QETexture();
		~QETexture();

		void bind();
		HRESULT load(LPCWSTR file);
	private:
		ComPtr<ID3D11ShaderResourceView> m_texture;

	};
}


