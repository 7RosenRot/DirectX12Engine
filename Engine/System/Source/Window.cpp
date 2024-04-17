#include <Window.h>

HWND Window::m_hwnd = nullptr;

LRESULT CALLBACK Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	DirectX12Game* Geometry = reinterpret_cast<DirectX12Game*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (msg) {
	case WM_CREATE: {
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	return 0;
	case WM_PAINT:
		if (Geometry) {
			Geometry->OnUpdate();
			Geometry->OnRender();
		}
		return 0;
	case WM_DESTROY: {
		Window* DestroyWindow = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		DestroyWindow->OnDestroy();
		::PostQuitMessage(0);
		break;
	}
	case WM_SIZE: {
		int Width = LOWORD(lParam);
		int Height = HIWORD(lParam);
		break;
	}
	default:
		return ::DefWindowProcW(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

Window::Window() {}

int Window::Run(InterfaceDirectX* InterfaceDirectXGame, HINSTANCE hInstance, int nCmdShow) {
	WNDCLASSEX wc = { 0 };

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadIcon(NULL, IDC_ARROW);
	wc.hInstance = NULL;
	wc.lpszClassName = L"WindowClass";
	wc.lpszMenuName = L"";

	wc.lpfnWndProc = WndProc;

	RegisterClassEx(&wc);

	RECT WindowRect = { 0, 0, static_cast<LONG>(InterfaceDirectXGame->GetWindowWidth()), static_cast<LONG>(InterfaceDirectXGame->GetWindowHeight()) };

	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);

	m_hwnd = CreateWindow(wc.lpszClassName, InterfaceDirectXGame->GetWindowName(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, nullptr, nullptr, hInstance, InterfaceDirectXGame);

	InterfaceDirectXGame->OnInitialize();
	ShowWindow(m_hwnd, nCmdShow);

	MSG msg = {};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	InterfaceDirectXGame->OnDestroy();

	return static_cast<char>(msg.wParam);
}

void Window::OnCreate() {}

void Window::OnUpdate() {}

void Window::OnDestroy() {}

Window::~Window() {}