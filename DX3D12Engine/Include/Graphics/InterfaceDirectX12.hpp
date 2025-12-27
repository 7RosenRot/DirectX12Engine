#pragma once
#include <wtypes.h>
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Include/Graphics/d3dx12.hpp>

namespace D3D12Engine {
  class InterfaceDirectX12 {
  public:
    InterfaceDirectX12(UINT m_WindowHeight, UINT m_WindowWidth, std::wstring m_WindowName);
    virtual ~InterfaceDirectX12();

    virtual void OnInitialize() = 0;
    virtual void OnRender() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnDestroy() = 0;

    UINT getWindowHeight() { return WindowHeight; }
    UINT getWindowWidth() { return WindowWidth; }
    const WCHAR* getWindowName() { return WindowName.c_str(); }

    protected:
    void getHardwareAdapter(_In_ IDXGIFactory1* ptr_Factory1, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ptr_Adapter1, bool requestHighPerfomanceAdapter);
    
    std::wstring WindowName;
    UINT WindowHeight;
    UINT WindowWidth;

    bool useWarpAdapter;
  };
}