#pragma once
#include <d3d11.h>
class QEBuffer
{
public:
	QEBuffer();
	~QEBuffer();

	virtual HRESULT Create() = 0;
	virtual HRESULT Bind() = 0;
};

