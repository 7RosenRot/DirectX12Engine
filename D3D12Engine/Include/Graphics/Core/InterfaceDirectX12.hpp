#pragma once

#include <d3d12.h>
#include <Include\Libraries\d3dx12.h>
#include <dxgi1_6.h>
#include <wtypes.h>
#include <string>

namespace D3D12Engine {
  class InterfaceDirectX12 {
  public:
    InterfaceDirectX12(
      UINT WindowWidth, UINT WindowHeight, UINT AspectWidth, UINT AspectHeight, std::wstring WindowName
    );
    virtual ~InterfaceDirectX12();

    virtual void OnInitialize() = 0;
    virtual void OnRender() = 0;
    virtual void OnResize(UINT WindowWidth, UINT WindowHeight) = 0;
    virtual void OnUpdate() = 0;
    virtual void OnDestroy() = 0;

    UINT GetWindowWidth() { return m_WindowWidth; }
    UINT GetWindowHeight() { return m_WindowHeight; }
    
    UINT GetAspectWidth() { return m_AspectWidth; }
    UINT GetAspectHeight() { return m_AspectHeight; }

    float GetAspectRatio() { return m_AspectRatio; }
    
    const WCHAR* GetWindowName() { return m_WindowName.c_str(); }
    
    std::wstring GetAssetPath(LPCWSTR assetName) { return m_assetPath + L"/D3D12Engine/Shaders/" + assetName;}

  protected:
    void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory1, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter1, bool requestHighPerfomanceAdapter);
    
    UINT m_WindowHeight{0};
    UINT m_WindowWidth{0};

    UINT m_AspectHeight{0};
    UINT m_AspectWidth{0};
    float m_AspectRatio{0};

    std::wstring m_WindowName = L"";

    std::wstring m_assetPath = L"";

    bool m_useWarpAdapter{false};
  };
}