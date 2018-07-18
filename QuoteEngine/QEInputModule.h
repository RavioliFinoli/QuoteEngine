#pragma once
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")
#include <dinput.h>
#include <wrl.h>


namespace QuoteEngine 
{
	class QEInputModule
	{
	public:
		QEInputModule(HINSTANCE hInstance, HWND hwnd);
		~QEInputModule();
	
		void Update();
		double GetTime();
	private:
		Microsoft::WRL::ComPtr<IDirectInputDevice8> m_DIMouse;
		Microsoft::WRL::ComPtr<IDirectInputDevice8> m_DIKeyboard;
		HWND m_hwnd;
		DIMOUSESTATE m_mouseLastState;
		LPDIRECTINPUT8 m_DirectInput;
	
		//Time
		double countsPerSecond;
		__int64 CounterStart;
		__int64 frameTimeOld = 0;
		double frameTime = 0;
	};
}

