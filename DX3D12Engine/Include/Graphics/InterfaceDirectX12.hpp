#pragma once
#include <wtypes.h>
#include <string>

namespace D3D12Engine {
    class InterfaceDirectX12 {
    public:
        InterfaceDirectX12(UINT WindowHeight, UINT WindowWidth, std::wstring WindowName);
        ~InterfaceDirectX12();

        UINT getWindowHeight() { return m_WindowHeight; }
        UINT getWindowWidth() { return m_WindowWidth; }
        const WCHAR* getWindowName() { return m_WindowName.c_str(); }

    protected:
        UINT m_WindowHeight;
        UINT m_WindowWidth;
        float coefficient;

    private:
        std::wstring m_WindowName;
    };
}