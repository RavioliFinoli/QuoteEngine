#include "QEInputModule.h"
#include "QuoteEngineCommon.h"
#include "QERenderingModule.h"

using QuoteEngine::QERenderingModule;
using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

#define RETURN_IF_FAIL(x) if(x != S_OK)return
#define HALF_PI 1.57

void QuoteEngine::QEInputModule::Update()
{
	//Get delta time and reset timer
	double delta = GetTime();
	ResetTimer();

	//Initialize mouse and keyboard containers
	DIMOUSESTATE mouseCurrState;
	ZeroMemory(&mouseCurrState, sizeof(mouseCurrState));
	BYTE keyboardState[256];
	ZeroMemory(keyboardState, 256);

	//Acquire keyboard
	m_DIKeyboard->Acquire();
	RETURN_IF_FAIL(m_DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState));

	//If mouse is active, acquire mouse
	if (mouseActive)
	{
		(m_DIMouse->Acquire());
		RETURN_IF_FAIL(m_DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState));
	}

	//Camera and movement data
	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	auto cameraCurrent = QERenderingModule::gCamera.getCameraData();
	Camera::CameraUpdateData data;
	data.camPosition = QERenderingModule::gCamera.getCameraPosition();
	data.camTarget = QERenderingModule::gCamera.getCameraTarget();

	//Process

	//Scenes
	{
		if (keyboardState[DIK_0] & 0x80 && (!(m_lastKeyboardState[DIK_0] & 0x80)))
		{
			QERenderingModule::gActiveScene = 0;
		}
		if (keyboardState[DIK_1] & 0x80 && (!(m_lastKeyboardState[DIK_1] & 0x80)))
		{
			QERenderingModule::gActiveScene = 1;
		}
		if (keyboardState[DIK_2] & 0x80 && (!(m_lastKeyboardState[DIK_2] & 0x80)))
		{
			QERenderingModule::gActiveScene = 2;
		}
		if (keyboardState[DIK_3] & 0x80 && (!(m_lastKeyboardState[DIK_3] & 0x80)))
		{
			QERenderingModule::gActiveScene = 3;
		}
		if (keyboardState[DIK_4] & 0x80 && (!(m_lastKeyboardState[DIK_4] & 0x80)))
		{
			QERenderingModule::gActiveScene = 4;
		}
		if (keyboardState[DIK_5] & 0x80 && (!(m_lastKeyboardState[DIK_5] & 0x80)))
		{
			QERenderingModule::gActiveScene = 5;
		}
		if (keyboardState[DIK_6] & 0x80 && (!(m_lastKeyboardState[DIK_6] & 0x80)))
		{
			QERenderingModule::gActiveScene = 6;
		}
		if (keyboardState[DIK_7] & 0x80 && (!(m_lastKeyboardState[DIK_7] & 0x80)))
		{
			QERenderingModule::gActiveScene = 7;
		}
		if (keyboardState[DIK_8] & 0x80 && (!(m_lastKeyboardState[DIK_8] & 0x80)))
		{
			QERenderingModule::gActiveScene = 8;
		}
		if (keyboardState[DIK_9] & 0x80 && (!(m_lastKeyboardState[DIK_9] & 0x80)))
		{
			QERenderingModule::gActiveScene = 9;
		}
	}
	if (keyboardState[DIK_ESCAPE] & 0x80)
	{
		PostMessage(m_hwnd, WM_DESTROY, 0, 0);
	}
	if (keyboardState[DIK_Q] & 0x80 && (!(m_lastKeyboardState[DIK_Q] & 0x80)))
	{
		//Set mouse active/inactive 
		if (mouseActive)
		{
			mouseActive = false;
			m_DIMouse->Unacquire();
		}
		else
		{
			mouseActive = true;
		}
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward = 1.0f;
	}
	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward = -1.0f;
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight = 1.0f;
	}
	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight = -1.0f;
	}
	if (keyboardState[DIK_P] & 0x80 && (!(m_lastKeyboardState[DIK_P] & 0x80)))
	{
		QERenderingModule::gUseDeferredShader = !QERenderingModule::gUseDeferredShader;
	}
	if (keyboardState[DIK_G] & 0x80 && (!(m_lastKeyboardState[DIK_G] & 0x80)))
	{
		QERenderingModule::gBlur = !QERenderingModule::gBlur;
	}
	if (mouseCurrState.lX != m_mouseLastState.lX || mouseCurrState.lY != m_mouseLastState.lY)
	{
		//Increment camera pitch and yaw 
		cameraCurrent.camPitch += mouseCurrState.lY * 0.001f;
		cameraCurrent.camYaw += mouseCurrState.lX * 0.001f;

		//Constrain cam pitch and yaw, to avoid flipping the camera when looking straight up or down
		cameraCurrent.camPitch = cameraCurrent.camPitch < -HALF_PI ? -HALF_PI : cameraCurrent.camPitch;
		cameraCurrent.camPitch = cameraCurrent.camPitch >  HALF_PI ?  HALF_PI : cameraCurrent.camPitch;

	}
	if (mouseCurrState.lZ != m_mouseLastState.lZ)
	{
		m_MovementSpeed += (mouseCurrState.lZ / 24000.0);
		m_MovementSpeed = m_MovementSpeed < 0.0 ? 0.0001 : m_MovementSpeed;
	}
	//Copy current keyboard state to last keyboard state
	memcpy(m_lastKeyboardState, keyboardState, 256);

	//Update camera based on new data
	{
		XMMATRIX camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(cameraCurrent.camPitch, cameraCurrent.camYaw, 0);
		XMVECTOR camTarget = DirectX::XMVector3TransformCoord(DEFAULT_FORWARD, camRotationMatrix);
		camTarget = DirectX::XMVector3Normalize(camTarget);

		XMMATRIX RotateYTempMatrix;
		RotateYTempMatrix = DirectX::XMMatrixRotationY(cameraCurrent.camYaw);

		XMVECTOR camRight = DirectX::XMVector4Transform(DEFAULT_RIGHT, RotateYTempMatrix);
		XMVECTOR camForward = XMVector4Transform(DEFAULT_FORWARD, RotateYTempMatrix);
		cameraCurrent.camPosition = DirectX::XMVectorAdd(DirectX::XMVectorScale(camRight, moveLeftRight * (delta + m_MovementSpeed)), cameraCurrent.camPosition);
		cameraCurrent.camPosition = DirectX::XMVectorAdd(DirectX::XMVectorScale(camForward, moveBackForward * (delta + m_MovementSpeed)), cameraCurrent.camPosition);



		camTarget = DirectX::XMVectorAdd(cameraCurrent.camPosition, camTarget);
		data.camPosition = cameraCurrent.camPosition;
		data.camTarget = camTarget;

		camTarget.m128_f32[3] = 0.0f; //zero w
		m_mouseLastState = mouseCurrState;

		QERenderingModule::gCamera.update(data);
		QERenderingModule::gCamera.updateCameraInformation(cameraCurrent);
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
	ResetTimer();


}

void QuoteEngine::QEInputModule::ResetTimer()
{
	LARGE_INTEGER frequencyCount;
	QueryPerformanceFrequency(&frequencyCount);

	countsPerSecond = double(frequencyCount.QuadPart);

	QueryPerformanceCounter(&frequencyCount);
	CounterStart = frequencyCount.QuadPart;
}

QuoteEngine::QEInputModule::~QEInputModule()
{
}