#include <Include/Window/Window.hpp>

LRESULT CALLBACK D3D12Engine::Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

D3D12Engine::Window::Window(InterfaceDirectX12* m_InterfaceDirectX12, HINSTANCE hInstance, int nCmdShow) {
    auto registerWindowClassFunction = []() {
        WNDCLASSEX wndClass{};
        
        wndClass.cbSize = sizeof(WNDCLASSEX);
        wndClass.lpszClassName = L"WindowName";
        wndClass.lpszMenuName = L"";

        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
        wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
        wndClass.hCursor = LoadIcon(NULL, IDC_ARROW);
        wndClass.hInstance = NULL;

        wndClass.lpfnWndProc = &WndProc;

        return RegisterClassEx(&wndClass);
    };

    static const auto wndClassID = std::invoke(registerWindowClassFunction);
    if (!wndClassID) throw std::runtime_error("RegisterClassEX Failed!");

    RECT wndRectangle{0, 0, 1280, 720};
    AdjustWindowRect(&wndRectangle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, false);

    m_handle = CreateWindowEx(
        NULL,
        MAKEINTATOM(wndClassID),
        L"WindowName",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        wndRectangle.right - wndRectangle.left,
        wndRectangle.bottom - wndRectangle.top,
        NULL,
        NULL,
        hInstance,
        m_InterfaceDirectX12
    );

    if (!m_handle) throw std::runtime_error("m_handle Failed!");

    ShowWindow(static_cast<HWND>(m_handle), SW_SHOW);
}

D3D12Engine::Window::~Window() {
    DestroyWindow(static_cast<HWND>(m_handle));
}