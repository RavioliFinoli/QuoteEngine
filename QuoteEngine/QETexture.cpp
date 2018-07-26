#include "QETexture.h"
#include "QERenderingModule.h"
#include "WICTextureLoader/WICTextureLoader.h"

QuoteEngine::QETexture::QETexture(LPCWSTR file)
{
	HRESULT hr = S_OK;
	hr = DirectX::CreateWICTextureFromFile(QERenderingModule::gDevice.Get(), QERenderingModule::gDeviceContext.Get(), file, nullptr, m_texture.GetAddressOf());
}

QuoteEngine::QETexture::QETexture()
{

}

QuoteEngine::QETexture::~QETexture()
{
}

void QuoteEngine::QETexture::bind()
{
	if (m_texture)
		QERenderingModule::gDeviceContext->PSSetShaderResources(0, 1, m_texture.GetAddressOf());
}

HRESULT QuoteEngine::QETexture::load(LPCWSTR file)
{
	return DirectX::CreateWICTextureFromFile(QERenderingModule::gDevice.Get(), file, nullptr, m_texture.ReleaseAndGetAddressOf());
}
