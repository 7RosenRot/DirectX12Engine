#define UNICODE
#define _UNICODE

#include <Include/Window/Window.hpp>

LRESULT CALLBACK D3D12Engine::Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        break;
    case WM_GETMINMAXINFO: {
            LPMINMAXINFO setBorders = (LPMINMAXINFO)lParam;

            setBorders->ptMinTrackSize.x = min_WndWidth;
            setBorders->ptMinTrackSize.y = min_WndHeight;
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

D3D12Engine::Window::Window(InterfaceDirectX12* m_InterfaceDirectX12, HINSTANCE hInstance, int m_CmdShow) {
    auto registerWindowClassFunction = []() {
        WNDCLASSEX wndClass{};
        
        wndClass.cbSize = sizeof(WNDCLASSEX);
        wndClass.lpszClassName = L"WindowName";
        wndClass.lpszMenuName = L"";

        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hInstance = NULL;

        wndClass.lpfnWndProc = &WndProc;

        return RegisterClassEx(&wndClass);
    };

    static const auto wndClassID = std::invoke(registerWindowClassFunction);
    if (!wndClassID) throw std::runtime_error("RegisterClassEX Failed!");

    RECT WndRect{0, 0, static_cast<LONG>(m_InterfaceDirectX12->getWindowWidth()), static_cast<LONG>(m_InterfaceDirectX12->getWindowHeight())};
    AdjustWindowRect(&WndRect, WS_OVERLAPPEDWINDOW, false);

    m_handle = CreateWindowEx(
        NULL,
        MAKEINTATOM(wndClassID),
        m_InterfaceDirectX12->getWindowName(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        WndRect.right - WndRect.left,
        WndRect.bottom - WndRect.top,
        NULL,
        NULL,
        hInstance,
        m_InterfaceDirectX12
    );
    if (!m_handle) throw std::runtime_error("m_handle Failed!");

    ShowWindow(static_cast<HWND>(m_handle), m_CmdShow);
}

void D3D12Engine::Window::run_GameLoop() {
    MSG msg{};

    while (m_isRunning) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_isRunning = false;

                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

D3D12Engine::Window::~Window() {
    DestroyWindow(static_cast<HWND>(m_handle));
}