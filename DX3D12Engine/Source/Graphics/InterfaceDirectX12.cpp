#include <Include/Graphics/InterfaceDirectX12.hpp>

D3D12Engine::InterfaceDirectX12::InterfaceDirectX12(UINT WindowHeight, UINT WindowWidht, std::wstring WindowName) :
    m_WindowHeight(WindowHeight), 
    m_WindowWidth(WindowWidht),
    m_WindowName(WindowName)
{}

void D3D12Engine::InterfaceDirectX12::getHardwareAdapter(
    _In_ IDXGIFactory1* ptr_Factory1,
    _Outptr_opt_result_maybenull_ IDXGIAdapter1** ptr_Adapter1,
    bool requestHighPerfomanceAdpter)
{}

D3D12Engine::InterfaceDirectX12::~InterfaceDirectX12() {}