#pragma once
#include <d3d11.h>

class QERenderingModule
{
public:
	QERenderingModule();
	~QERenderingModule();

private:
	static const ID3D11Device* gDevice;
	static const ID3D11DeviceContext* gDeviceContext;
};

