#include <Include/Graphics/InterfaceDirectX12.hpp>
#include <filesystem>
#include <iostream>
#include <chrono>

D3D12Engine::InterfaceDirectX12::InterfaceDirectX12(UINT WindowHeight, UINT WindowWidht, std::wstring WindowName) :
  m_WindowHeight(WindowHeight), 
  m_WindowWidth(WindowWidht),
  m_WindowName(WindowName)
{
  std::wstring assetPath = std::filesystem::current_path().wstring();
  std::wstring buffer(assetPath.begin(), assetPath.end());
  m_assetPath = buffer;

  m_Coefficient = static_cast<float>(WindowWidht) / static_cast<float>(WindowHeight);
}

float D3D12Engine::InterfaceDirectX12::GetElapsedSeconds() {
  static const auto startTime = std::chrono::high_resolution_clock::now();
  auto currentTime = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> elapsed = currentTime - startTime;

  return elapsed.count();
}

_Use_decl_annotations_
void D3D12Engine::InterfaceDirectX12::GetHardwareAdapter(
  _In_ IDXGIFactory1* ptr_Factory1,
  _Outptr_opt_result_maybenull_ IDXGIAdapter1** ptr_Adapter1,
  bool requestHighPerfomanceAdpter)
{
  *ptr_Adapter1 = nullptr;

  Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter1;
  Microsoft::WRL::ComPtr<IDXGIFactory6> Factory6;

  if (SUCCEEDED(ptr_Factory1->QueryInterface(IID_PPV_ARGS(&Factory6)))) {
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
        *ptr_Adapter1 = Adapter1.Detach();
        return;
      }
    }
  }

  for (UINT AdapterIndex = 0;
    SUCCEEDED(ptr_Factory1->EnumAdapters1(AdapterIndex, &Adapter1));
    AdapterIndex += 1) 
  {
    DXGI_ADAPTER_DESC1 AdapterDescriptor;
    Adapter1->GetDesc1(&AdapterDescriptor);

    if (AdapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }

    if (SUCCEEDED(D3D12CreateDevice(Adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
      *ptr_Adapter1 = Adapter1.Detach();
      return;
    }
  }
}

D3D12Engine::InterfaceDirectX12::~InterfaceDirectX12() {}