#include "DirectX12Game.h"

DirectX12Game::DirectX12Game(UINT WindowWidth, UINT WindowHeight, wstring WindowName) :
	InterfaceDirectX(WindowWidth, WindowHeight, WindowName),
	m_frameIndex(0),
	m_viewPort(0.0f, 0.0f, static_cast<float>(WindowWidth), static_cast<float>(WindowHeight)),
	m_scissorRect(0, 0, static_cast<LONG>(WindowWidth), static_cast<LONG>(WindowHeight)),
	m_rtvDescriptorSize(0),
	m_FrameCount(),
	m_fenceEvent(),
	m_fenceValue(),
	m_vertexBufferView()
{}

void DirectX12Game::OnInitialize() {
	LoadPipeline();
	LoadAssets();
}

void DirectX12Game::OnUpdate() {}

void DirectX12Game::OnRender() {
	FillComandList();

	ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	m_SwapChain->Present(1, 0);

	WaitForPreviousFrame();
}

void DirectX12Game::OnDestroy() {
	WaitForPreviousFrame();
	CloseHandle(m_fenceEvent);
}

void DirectX12Game::LoadPipeline() {
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) 
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

	if (useWarpDevice) {
		Microsoft::WRL::ComPtr<IDXGIAdapter> WarpAdapter;
		factory->EnumWarpAdapter(IID_PPV_ARGS(&WarpAdapter));
		D3D12CreateDevice(WarpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	}
	else {
		Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter, false);
		D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	}

	D3D12_COMMAND_QUEUE_DESC queueDescriptor = {};
	queueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDescriptor.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	m_Device->CreateCommandQueue(&queueDescriptor, IID_PPV_ARGS(&m_CommandQueue));

	DXGI_SWAP_CHAIN_DESC1 swapChainDescriptor = {};
	swapChainDescriptor.BufferCount = FrameCount;
	swapChainDescriptor.Width = m_WindowWidth;
	swapChainDescriptor.Height = m_WindowHeight;
	swapChainDescriptor.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDescriptor.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), Window::GetHwnd(), &swapChainDescriptor, nullptr, nullptr, &swapChain);

	factory->MakeWindowAssociation(Window::GetHwnd(), DXGI_MWA_NO_ALT_ENTER);

	swapChain.As(&m_SwapChain);
	m_FrameCount = m_SwapChain->GetCurrentBackBufferIndex();

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescriptor = {};
	rtvHeapDescriptor.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDescriptor.NumDescriptors = FrameCount;
	rtvHeapDescriptor.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDescriptor.NodeMask = 0;
	m_Device->CreateDescriptorHeap(&rtvHeapDescriptor, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT Frame = 0; Frame < FrameCount; Frame += 1) {
		m_SwapChain->GetBuffer(Frame, IID_PPV_ARGS(&m_RenderTargets[Frame]));
		m_Device->CreateRenderTargetView(m_RenderTargets[Frame].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}

	m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
}

void DirectX12Game::LoadAssets() {
	CD3DX12_ROOT_SIGNATURE_DESC rootSiganatureDescriptor{};
	rootSiganatureDescriptor.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	D3D12SerializeRootSignature(&rootSiganatureDescriptor, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFalgs = 0;
#endif

	D3DCompileFromFile(GetAssetsFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
	D3DCompileFromFile(GetAssetsFullPath(L"shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

	
	D3D12_INPUT_ELEMENT_DESC inputElementDescriptors[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescriptor = {};
	psoDescriptor.InputLayout = { inputElementDescriptors, _countof(inputElementDescriptors) };
	psoDescriptor.pRootSignature = m_rootSignature.Get();
	psoDescriptor.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDescriptor.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDescriptor.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDescriptor.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDescriptor.DepthStencilState.DepthEnable = FALSE;
	psoDescriptor.DepthStencilState.StencilEnable = FALSE;
	psoDescriptor.SampleMask = UINT_MAX;
	psoDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDescriptor.NumRenderTargets = 1;
	psoDescriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDescriptor.SampleDesc.Count = 1;

	m_Device->CreateGraphicsPipelineState(&psoDescriptor, IID_PPV_ARGS(&m_pipelineState));

	m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_CommandList));

	m_CommandList->Close();

	Vertex triangleVertices[] = {
		{ { 0.0f, 0.25f * Coefficient, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.25f, -0.25f * Coefficient, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.25f, -0.25f * Coefficient, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const UINT vertexBufferSize = sizeof(triangleVertices);

	CD3DX12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	CD3DX12_RESOURCE_DESC resourceDescriptor = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);

	m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptor, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer));

	UINT8* pVertexDataBegin = {};
	CD3DX12_RANGE readRange(0, 0);
	m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_fenceValue = 1;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr) {
		throw_with_nested(HRESULT_FROM_WIN32(GetLastError()));
	}

	WaitForPreviousFrame();
}

void DirectX12Game::FillComandList() {
	m_CommandAllocator->Reset();
	m_CommandList->Reset(m_CommandAllocator.Get(), m_pipelineState.Get());

	m_CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_CommandList->RSSetViewports(1, &m_viewPort);
	m_CommandList->RSSetScissorRects(1, &m_scissorRect);

	CD3DX12_RESOURCE_BARRIER transitionBarier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	m_CommandList->ResourceBarrier(1, &transitionBarier);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

	m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float bgClearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	m_CommandList->ClearRenderTargetView(rtvHandle, bgClearColor, 0, nullptr);
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_CommandList->DrawInstanced(3, 1, 0, 0);

	transitionBarier = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_CommandList->ResourceBarrier(1, &transitionBarier);
	m_CommandList->Close();
}

void DirectX12Game::WaitForPreviousFrame() {
	const UINT64 fence = m_fenceValue;
	m_CommandQueue->Signal(m_fence.Get(), fence);
	m_fenceValue += 1;

	if (m_fence->GetCompletedValue() < fence) {
		m_fence->SetEventOnCompletion(fence, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_frameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}