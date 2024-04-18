#include "InterfaceDirectX.h"

InterfaceDirectX::InterfaceDirectX(UINT WindowWidth, UINT WindowHeight, wstring WindowName):
	new_WindowWidth(WindowWidth),
	new_WindowHeight(WindowHeight),
	new_WindowName(WindowName),
	useWarpDevice(false) 
{
	WCHAR assetsPath[512]{ L"D:\\Microsoft Visual Studio\\Visual Studio 2022\\DirectX12Engine\\Engine\\System\\Shaders\\" };
	my_assetsPath = assetsPath;
	Coefficient = static_cast<float>(WindowWidth) / static_cast<float>(WindowHeight);
}

InterfaceDirectX::~InterfaceDirectX() {}

_Use_decl_annotations_
wstring InterfaceDirectX::GetAssetsFullPath(LPCWSTR assetName) {
	return my_assetsPath + assetName;
}

void InterfaceDirectX::GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHightPerfomanceAdapter) {
	*ppAdapter = nullptr;
	requestHightPerfomanceAdapter = false;

	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;

	if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
		for (UINT AdapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(AdapterIndex, requestHightPerfomanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter))); AdapterIndex += 1) {
			DXGI_ADAPTER_DESC1 descriptor;
			adapter->GetDesc1(&descriptor);

			if (descriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) break;
		}
	}

	if (adapter.Get() == nullptr) {
		for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); adapterIndex += 1) {
			DXGI_ADAPTER_DESC1 descriptor;
			adapter->GetDesc1(&descriptor);

			if (descriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
			if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) break;
		}
	}
	*ppAdapter = adapter.Detach();
}