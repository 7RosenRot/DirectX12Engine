#include <Include/Graphics/InterfaceDirectX12.hpp>

D3D12Engine::InterfaceDirectX12::InterfaceDirectX12(UINT WindowHeight, UINT WindowWidht, std::wstring WindowName) :
  m_WindowHeight(WindowHeight), 
  m_WindowWidth(WindowWidht),
  m_WindowName(WindowName)
{}

// Входной параметр: ptr_Factory1 - список доступных графических адаптеров
// Выходной параметр: ptr_Adapter1 - указатель на выбранный адаптер
// Передаваемый параметр: requestHighPerfomanceAdpter - запрашивать высокопроизводительный (дискретный) адаптер?
void D3D12Engine::InterfaceDirectX12::getHardwareAdapter(
  _In_ IDXGIFactory1* ptr_Factory1,
  _Outptr_opt_result_maybenull_ IDXGIAdapter1** ptr_Adapter1,
  bool requestHighPerfomanceAdpter)
{
  *ptr_Adapter1 = nullptr;

  Microsoft::WRL::ComPtr<IDXGIAdapter1> Adapter1;
  Microsoft::WRL::ComPtr<IDXGIFactory6> Factory6;

  // Если система поддреживает DXGI 1.6 - используем новый способ перечисления адаптеров
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

  // Если DXGI 1.6 недоступен - используем старый способ перечисленния адаптеров
  for (UINT AdapterIndex = 0;
    SUCCEEDED(ptr_Factory1->EnumAdapters1(AdapterIndex, &Adapter1));
    AdapterIndex += 1) 
  {
    // Инициализируем и получаем дескриптор адаптера, который сожержит: имя GPU, объем VRAM, флаг (Software/Hardware)
    DXGI_ADAPTER_DESC1 AdapterDescriptor;
    Adapter1->GetDesc1(&AdapterDescriptor);

    // Если флаг адаптера - Software (Графика обрабатывается на CPU), ищем другой адаптер
    if (AdapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }

    // Если GPU поддреживает DirectX 12 (Минимум - DirectX 11) - 
    if (SUCCEEDED(D3D12CreateDevice(Adapter1.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) {
      *ptr_Adapter1 = Adapter1.Detach();
      return;
    }
  }
}

D3D12Engine::InterfaceDirectX12::~InterfaceDirectX12() {}