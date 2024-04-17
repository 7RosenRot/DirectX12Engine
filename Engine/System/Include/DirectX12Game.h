#pragma once
#include <InterfaceDirectX.h>
#include <Window.h>

class DirectX12Game : public InterfaceDirectX {
public:
	DirectX12Game(UINT WindowWidth, UINT WindowHeight, wstring WindowName);

	virtual void OnInitialize();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();
private:
	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	static const UINT FrameCount = 2;
	UINT my_FrameCount;
	UINT my_rtvDescriptorSize;

	CD3DX12_VIEWPORT my_viewPort;
	CD3DX12_RECT my_scissorRect;
	
	Microsoft::WRL::ComPtr<ID3D12Device> my_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> my_CommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> my_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> my_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> my_RenderTargets[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> my_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> my_CommandList;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> my_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> my_pipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> my_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW my_vertexBufferView;

	UINT my_frameIndex;
	UINT64 my_fenceValue;
	HANDLE my_fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> my_fence;

	void LoadPipeline();
	void LoadAssets();
	void FillComandList();
	void WaitForPreviousFrame();
};