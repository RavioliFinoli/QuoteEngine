#pragma once
#include <d3d11.h>
#include <wrl.h>

namespace QuoteEngine
{
	class QEConstantBuffer
	{
	public:
		QEConstantBuffer(size_t bufferSize, void* initialData);
		~QEConstantBuffer();
	
		ID3D11Buffer* get();
	private:
		//The ID3D11Buffer pointer for this constant buffer
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_Buffer;
		
		//The size of the buffer.
		const size_t m_BufferSize;

		//The data of the buffer. This data will be updated by clients and when
		//a client is about to bind the buffer, the actual buffer will be updated 
		//with this data.
		void* m_Data = nullptr;

		//Creates the buffer
		HRESULT create();
	};
}
