#include <Include/Graphics/InterfaceDirectX12.hpp>

#include <chrono>
#include <iostream>
#include <filesystem>

D3D12Engine::InterfaceDirectX12::InterfaceDirectX12(UINT WindowHeight, UINT WindowWidht, std::wstring WindowName) :
  m_WindowHeight(WindowHeight), 
  m_WindowWidth(WindowWidht),
  m_WindowName(WindowName)
{
  m_assetPath = std::filesystem::current_path().wstring();
}

_Use_decl_annotations_
void D3D12Engine::InterfaceDirectX12::GetHardwareAdapter(
  _In_ IDXGIFactory1* pFactory1,
  _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter1,
  bool requestHighPerfomanceAdpter)
{
  *ppAdapter1 = nullptr;

  Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter1;
  Microsoft::WRL::ComPtr<IDXGIFactory6> Factory6;

  if (SUCCEEDED(pFactory1->QueryInterface(IID_PPV_ARGS(&Factory6)))) {
    const auto GpuPreference = requestHighPerfomanceAdpter ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED;
    
    for (UINT AdapterIndex = 0;
      SUCCEEDED(Factory6->EnumAdapterByGpuPreference(AdapterIndex, GpuPreference, IID_PPV_ARGS(&Adapter1)));
      AdapterIndex += 1)
    {
      DXGI_ADAPTER_DESC1 AdapterDescriptor;
      Adapter1->GetDesc1(&AdapterDescriptor);

      if (AdapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        continue;
      }
      if (SUCCEEDED(D3D12CreateDevice(Adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
        *ppAdapter1 = Adapter1.Detach();
        return;
      }
    }
  }

  for (UINT AdapterIndex = 0;
    SUCCEEDED(pFactory1->EnumAdapters1(AdapterIndex, &Adapter1));
    AdapterIndex += 1) 
  {
    DXGI_ADAPTER_DESC1 AdapterDescriptor;
    Adapter1->GetDesc1(&AdapterDescriptor);

    if (AdapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }

    if (SUCCEEDED(D3D12CreateDevice(Adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
      *ppAdapter1 = Adapter1.Detach();
      return;
    }
  }
}

D3D12Engine::InterfaceDirectX12::~InterfaceDirectX12() {}