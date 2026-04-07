#pragma once
#include <Include/Graphics/d3dx12.h>

#include <wtypes.h>
#include <string>
#include <dxgi1_6.h>
#include <d3d12.h>

namespace D3D12Engine {
  class InterfaceDirectX12 {
  public:
    InterfaceDirectX12(UINT WindowHeight, UINT WindowWidth, std::wstring WindowName);
    virtual ~InterfaceDirectX12();

    virtual void OnInitialize() = 0;
    virtual void OnRender() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnDestroy() = 0;

    void GetWindowHeight(UINT newHeight) { m_WindowHeight = newHeight; }

    UINT GetWindowHeight() { return m_WindowHeight; }
    UINT GetWindowWidth() { return m_WindowWidth; }
    const WCHAR* GetWindowName() { return m_WindowName.c_str(); }
    std::wstring GetAssetPath(LPCWSTR assetName) { return m_assetPath + L"\\DX3D12Engine\\Shaders\\" + assetName;}

  protected:
    void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory1, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter1, bool requestHighPerfomanceAdapter);

    std::wstring m_WindowName = L"";
    UINT m_WindowHeight{0};
    UINT m_WindowWidth{0};

    std::wstring m_assetPath = L"";

    bool m_useWarpAdapter{false};
  };
}