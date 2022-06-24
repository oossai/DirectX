#pragma once

#include "../Core/Graphics.h"

class Bunny : public Graphics
{
public:
	void OnUpdate(float time) override;
	void OnRender(bool vSync) override;
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_graphicsPipeline;
	Model m_bunny;
	DirectX::XMFLOAT4X4 m_cbMatrix;
	DirectX::XMFLOAT4X4 m_cbMatrix2;
private:
	NO_COPY_OR_MOVE(Bunny);
public:
	Bunny(){};
	~Bunny() { OnDestroy(); }
	void CreateRootSignature();
	void CreatePSO();
	void LoadModel();
};