#pragma once
#include <d3d11.h>
#include <wrl.h>

class QERenderingModule
{
public:
	static const Microsoft::WRL::ComPtr<ID3D11Device> gDevice;
	static const Microsoft::WRL::ComPtr<ID3D11DeviceContext> gDeviceContext;

	QERenderingModule(HWND WindowHandle);
	~QERenderingModule();

private:

};

