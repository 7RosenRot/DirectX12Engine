#pragma once
#include <wtypes.h>
#include <string>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <Include/Graphics/d3dx12.hpp>

namespace D3D12Engine {
  class InterfaceDirectX12 {
  public:
    InterfaceDirectX12(UINT WindowHeight, UINT WindowWidth, std::wstring WindowName);
    virtual ~InterfaceDirectX12();

    virtual void OnInitialize() = 0;
    virtual void OnRender() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnDestroy() = 0;

    UINT getWindowHeight() { return m_WindowHeight; }
    UINT getWindowWidth() { return m_WindowWidth; }
    const WCHAR* getWindowName() { return m_WindowName.c_str(); }
    std::wstring getAssetPath(LPCWSTR assetName) { return m_assetPath + L"\\DX3D12Engine\\Shaders\\" + assetName;}

  protected:
    void getHardwareAdapter(_In_ IDXGIFactory1* ptr_Factory1, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ptr_Adapter1, bool requestHighPerfomanceAdapter);
    
    std::wstring m_WindowName{0};
    UINT m_WindowHeight{0};
    UINT m_WindowWidth{0};

    float m_Coefficient{0.F};

    std::wstring m_assetPath{0};

    bool m_useWarpAdapter{false};
  };
}