#pragma once
#include <d3d11.h>
#include <wrl.h>

class QERenderingModule
{
public:
	static Microsoft::WRL::ComPtr<ID3D11Device> gDevice;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext> gDeviceContext;
	static Microsoft::WRL::ComPtr<IDXGISwapChain> gSwapChain;
	static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> gBackbufferRTV;
	
	QERenderingModule(HWND WindowHandle);
	~QERenderingModule();

private:
	HRESULT createDirect3DContext(HWND wndHandle);
};

