//--------------------------------------------------------------------------------------
// BTH - Stefan Petersson 2014.
//	   - modified by FLL
//--------------------------------------------------------------------------------------

//Mem leak identification
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  

#include <windows.h>

#include <d3d11.h>
#include <d3dcompiler.h>
#include "QERenderingModule.h"
#include "QEInputModule.h"
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_dx11.h"
#include "ImGUI/imgui_impl_win32.h"




using QuoteEngine::QERenderingModule;
using QuoteEngine::QEInputModule;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

HWND InitWindow(HINSTANCE hInstance);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	MSG msg = { 0 };
	HWND wndHandle = InitWindow(hInstance);
	


	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGuiContext* imguiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(imguiContext);
	ImGuiIO& io = ImGui::GetIO();


	if (wndHandle)
	{
		QERenderingModule renderingModule(wndHandle);
		renderingModule.compileShadersAndCreateShaderPrograms();
		renderingModule.createModels();

		QEInputModule inputModule(hInstance, wndHandle);

		//Init imgui
		ImGui_ImplWin32_Init(wndHandle);
		ImGui_ImplDX11_Init(QERenderingModule::gDevice.Get(), QERenderingModule::gDeviceContext.Get());

		ShowWindow(wndHandle, nCmdShow);

		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				inputModule.Update();
				renderingModule.render();
			}
		}
		DestroyWindow(wndHandle);
	}
	ImGui::DestroyContext(imguiContext);

	_CrtDumpMemoryLeaks();
	return (int) msg.wParam;
}

HWND InitWindow(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.hInstance      = hInstance;
	wcex.lpszClassName = L"BTH_D3D_DEMO";
	if (!RegisterClassEx(&wcex))
		return false;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND handle = CreateWindow(
		L"BTH_D3D_DEMO",
		L"BTH Direct3D Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		hInstance,
		nullptr);

	return handle;
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch (message) 
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}