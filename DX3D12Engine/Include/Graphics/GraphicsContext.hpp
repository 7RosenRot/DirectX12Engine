#pragma once

#include <Include/Graphics/GPUBuffer.hpp>
#include <Include/Graphics/GPUResource.hpp>

#include <d3d12.h>
#include "d3dx12.h"
#include <wrl/client.h>
#include <vector>

namespace D3D12Engine {
  class GraphicsContext {
   public:
    GraphicsContext(ID3D12GraphicsCommandList* cmdList);
    ~GraphicsContext() {}
    
    void TransitionResource(GPUResource& resource, D3D12_RESOURCE_STATES newState);
    
    void FlushResourceBarriers();
    
    void SetPipelineState(ID3D12PipelineState* pso) { m_CommandList->SetPipelineState(pso); }
    void SetGraphicsRootSignature(ID3D12RootSignature* rootSig) { m_CommandList->SetGraphicsRootSignature(rootSig); }
    void SetViewports(UINT NumViewports, const D3D12_VIEWPORT* pViewports) { m_CommandList->RSSetViewports(NumViewports, pViewports); }
    void SetScissorRects(UINT NumRects, const D3D12_RECT* pRects) { m_CommandList->RSSetScissorRects(NumRects, pRects); }
    
    void ClearColor(GPUResource& target, const float* bgColor);
    void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) { m_CommandList->IASetPrimitiveTopology(topology); }
    void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView) { m_CommandList->IASetVertexBuffers(Slot, 1, &VBView); }
    void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView) { m_CommandList->IASetIndexBuffer(&IBView); }
    
    void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS CBV) { m_CommandList->SetGraphicsRootConstantBufferView(RootIndex, CBV); }
    
    void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
      m_CommandList->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
    }
    
    void Close() { m_CommandList->Close(); }
    
    ID3D12GraphicsCommandList* GetCommandList() const { return m_CommandList.Get(); }
   private:
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    std::vector<D3D12_RESOURCE_BARRIER> m_ResourceBarrierBuffer;
  };
}