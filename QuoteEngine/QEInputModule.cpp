#include "QEInputModule.h"
#include "QuoteEngineCommon.h"

void QuoteEngine::QEInputModule::Update()
{
	double delta = GetTime();

	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	m_DIKeyboard->Acquire();
	m_DIMouse->Acquire();

	m_DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	m_DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	//Process
	if (keyboardState[DIK_ESCAPE] & 0x80)
	{
		PostMessage(m_hwnd, WM_DESTROY, 0, 0);
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		
	}
	if (keyboardState[DIK_ESCAPE] & 0x80)
	{
		PostMessage(m_hwnd, WM_DESTROY, 0, 0);
	}
}

double QuoteEngine::QEInputModule::GetTime()
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	return double(currentTime.QuadPart - CounterStart) / countsPerSecond;
}

QuoteEngine::QEInputModule::QEInputModule(HINSTANCE hInstance, HWND hwnd)
	: m_hwnd(hwnd)
{
	HRESULT	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&m_DirectInput,
		NULL);
	ASSERT_SOK(hr);

	hr = m_DirectInput->CreateDevice(GUID_SysKeyboard,
		&m_DIKeyboard,
		NULL);
	ASSERT_SOK(hr);

	hr = m_DirectInput->CreateDevice(GUID_SysMouse,
		&m_DIMouse,
		NULL);
	ASSERT_SOK(hr);

	hr = m_DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	ASSERT_SOK(hr);

	hr = m_DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	ASSERT_SOK(hr);

	hr = m_DIMouse->SetDataFormat(&c_dfDIMouse);
	ASSERT_SOK(hr);

	hr = m_DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
	ASSERT_SOK(hr);

	//Time
	{
		LARGE_INTEGER frequencyCount;
		QueryPerformanceFrequency(&frequencyCount);

		countsPerSecond = double(frequencyCount.QuadPart);

		QueryPerformanceCounter(&frequencyCount);
		CounterStart = frequencyCount.QuadPart;
	}

}

QuoteEngine::QEInputModule::~QEInputModule()
{
}