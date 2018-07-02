#pragma once
#include <d3d11.h>
#include <wrl.h> //ComPtr

class QEModel
{
public:
	QEModel(); //Default ctor for now, everything will be hardcoded for the time being
	~QEModel();
	ID3D11Buffer* getVertexBuffer();
	UINT getSizeInBytes();
	UINT getVertexCount();
private:

	UINT m_SizeInBytes = 0;
	UINT m_VertexCount = 0;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;

	HRESULT create();
};

