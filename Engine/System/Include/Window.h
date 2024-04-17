#pragma once
#include <InterfaceDirectX.h>
#include <DirectX12Game.h>

class Window {
public:
	Window();
	~Window();

	static int Run(InterfaceDirectX* InterfaceDirectXGame, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; };

	virtual void OnCreate();
	virtual void OnUpdate();
	virtual void OnDestroy();

protected:
	static HWND m_hwnd;
	bool isRunning = false;

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};