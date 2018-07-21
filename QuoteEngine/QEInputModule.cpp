#include "QEInputModule.h"
#include "QuoteEngineCommon.h"
#include "QERenderingModule.h"

using QuoteEngine::QERenderingModule;
using DirectX::XMMATRIX;
using DirectX::XMVECTOR;

#define PI 3.14

void QuoteEngine::QEInputModule::Update()
{
	double delta = GetTime();
	ResetTimer();
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	m_DIKeyboard->Acquire();
	m_DIMouse->Acquire();

	m_DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	m_DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);
	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	auto cameraCurrent = QERenderingModule::gCamera.getCameraData();
	Camera::CameraUpdateData data;
	data.camPosition = QERenderingModule::gCamera.getCameraPosition();
	data.camTarget = QERenderingModule::gCamera.getCameraTarget();

	//Process
	if (keyboardState[DIK_ESCAPE] & 0x80)
	{
		PostMessage(m_hwnd, WM_DESTROY, 0, 0);
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
	if (mouseCurrState.lX != m_mouseLastState.lX || mouseCurrState.lY != m_mouseLastState.lY)
	{
		cameraCurrent.camPitch += mouseCurrState.lY * 0.001f;
		cameraCurrent.camYaw += mouseCurrState.lX * 0.001f;

		cameraCurrent.camPitch = cameraCurrent.camPitch < -PI ? -PI : cameraCurrent.camPitch;
		cameraCurrent.camPitch = cameraCurrent.camPitch > PI ? PI : cameraCurrent.camPitch;

	}

	XMMATRIX camRotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(cameraCurrent.camPitch, cameraCurrent.camYaw, 0);
	XMVECTOR camTarget = DirectX::XMVector3TransformCoord(DEFAULT_FORWARD, camRotationMatrix);
	camTarget = DirectX::XMVector3Normalize(camTarget);

	XMMATRIX RotateYTempMatrix;
	RotateYTempMatrix = DirectX::XMMatrixRotationY(cameraCurrent.camYaw);

	XMVECTOR camRight = DirectX::XMVector4Transform(DEFAULT_RIGHT, RotateYTempMatrix);
	XMVECTOR camForward = XMVector4Transform(DEFAULT_FORWARD, RotateYTempMatrix);
	cameraCurrent.camPosition = DirectX::XMVectorAdd(DirectX::XMVectorScale(camRight, moveLeftRight * delta), cameraCurrent.camPosition);
	cameraCurrent.camPosition = DirectX::XMVectorAdd(DirectX::XMVectorScale(camForward, moveBackForward * delta), cameraCurrent.camPosition);



	camTarget = DirectX::XMVectorAdd(cameraCurrent.camPosition, camTarget);
	data.camPosition = cameraCurrent.camPosition;
	data.camTarget = camTarget;

	camTarget.m128_f32[3] = 0.0f;
	m_mouseLastState = mouseCurrState;

	QERenderingModule::gCamera.update(data);
	QERenderingModule::gCamera.updateCameraInformation(cameraCurrent);

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