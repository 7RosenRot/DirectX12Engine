#pragma once
#include <Headers.h>
using namespace std;

class InterfaceDirectX {
public:
	InterfaceDirectX(UINT WindowWidth, UINT WindowHeight, wstring WindowName);
	virtual ~InterfaceDirectX();

	virtual void OnInitialize() = 0;
	virtual void OnRender() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnDestroy() = 0;

	UINT GetWindowWidth() const { return new_WindowWidth; }
	UINT GetWindowHeight() const { return new_WindowHeight; }
	const WCHAR* GetWindowName() { return new_WindowName.c_str(); }

protected:
	wstring GetAssetsFullPath(LPCWSTR assetName);

	void GetHardwareAdapter(_In_ IDXGIFactory1* pFactory, _Outptr_opt_result_maybenull_ IDXGIAdapter1** ppAdapter, bool requestHightPerfomanceAdapter);
	
	UINT new_WindowWidth;
	UINT new_WindowHeight;
	float Coefficient;

	bool useWarpDevice;

private:
	wstring new_WindowName;
	wstring my_assetsPath;
};