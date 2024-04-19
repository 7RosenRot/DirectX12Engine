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
	UINT m_FrameCount;
	UINT m_rtvDescriptorSize;

	CD3DX12_VIEWPORT m_viewPort;
	CD3DX12_RECT m_scissorRect;
	
	Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_SwapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_RenderTargets[FrameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	UINT m_frameIndex;
	UINT64 m_fenceValue;
	HANDLE m_fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;

	void LoadPipeline();
	void LoadAssets();
	void FillComandList();
	void WaitForPreviousFrame();
};