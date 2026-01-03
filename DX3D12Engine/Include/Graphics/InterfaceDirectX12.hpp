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

    UINT GetWindowHeight() { return m_WindowHeight; }
    UINT GetWindowWidth() { return m_WindowWidth; }
    const WCHAR* GetWindowName() { return m_WindowName.c_str(); }
    std::wstring GetAssetPath(LPCWSTR assetName) { return m_assetPath + L"\\DX3D12Engine\\Shaders\\" + assetName;}

  protected:
    void GetHardwareAdapter(_In_ IDXGIFactory1* ptr_Factory1, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ptr_Adapter1, bool requestHighPerfomanceAdapter);
    
    float GetElapsedSeconds();

    std::wstring m_WindowName{0};
    UINT m_WindowHeight{0};
    UINT m_WindowWidth{0};

    float m_Coefficient{0.F};
    float m_timeRatio{0.F};

    std::wstring m_assetPath{0};

    bool m_useWarpAdapter{false};
  };
}