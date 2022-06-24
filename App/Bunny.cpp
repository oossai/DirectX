#include "Bunny.h"

#define TINYOBJLOADER_IMPLEMENTATION

using Microsoft::WRL::ComPtr;
using namespace DirectX;

void Bunny::CreateRootSignature()
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData{};
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(
		D3D12_FEATURE_ROOT_SIGNATURE, 
		&featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	// CD3DX12_DESCRIPTOR_RANGE1 cbvRange;
	// cbvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_ROOT_PARAMETER1 rootParams[2];
	rootParams[0].InitAsConstants(sizeof(XMFLOAT4X4) / 4, 0);
	rootParams[1].InitAsConstants(sizeof(XMFLOAT4X4) / 4, 1);
	// rootParams[0].InitAsDescriptorTable(1, &cbvRange, D3D12_SHADER_VISIBILITY_VERTEX);

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
	D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
	D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
	D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
	D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
	D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc{};
		rootSignatureDesc.Init_1_1(_countof(rootParams), rootParams, 
		0, nullptr, rootSignatureFlags);

	ComPtr<ID3DBlob> blobWithserializedRootSignature;
	ComPtr<ID3DBlob> errorBlob;

	D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, 
		featureData.HighestVersion,&blobWithserializedRootSignature, 
		&errorBlob);

	if (errorBlob)
	{
		throw std::runtime_error((char*)errorBlob->GetBufferPointer());
	}

	THROW_IF_FAILED(m_device->CreateRootSignature(0, 
		blobWithserializedRootSignature->GetBufferPointer(), 
		blobWithserializedRootSignature->GetBufferSize(), 
		IID_PPV_ARGS(m_rootSignature.ReleaseAndGetAddressOf())));
}

void Bunny::CreatePSO()
{
	D3D12_INPUT_ELEMENT_DESC inputLayout[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 
			D3D12_APPEND_ALIGNED_ELEMENT, 
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 
			D3D12_APPEND_ALIGNED_ELEMENT, 
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	ComPtr<ID3DBlob> vShaderBlob;
	D3DReadFileToBlob(L"..\\..\\Shaders\\VertexShader.cso", &vShaderBlob);

	ComPtr<ID3DBlob> pShaderBlob;
	D3DReadFileToBlob(L"..\\..\\Shaders\\PixelShader.cso", &pShaderBlob);

	auto pipelineDesc = GRAPHICS_PIPELINE::DefaultPipelineDesc();
	pipelineDesc.pRootSignature = m_rootSignature.Get();
	pipelineDesc.VS = CD3DX12_SHADER_BYTECODE(vShaderBlob.Get());
	pipelineDesc.PS = CD3DX12_SHADER_BYTECODE(pShaderBlob.Get());
	pipelineDesc.InputLayout.pInputElementDescs = inputLayout;
	pipelineDesc.InputLayout.NumElements = _countof(inputLayout);

	THROW_IF_FAILED(m_device->CreateGraphicsPipelineState(&pipelineDesc, 
		IID_PPV_ARGS(m_graphicsPipeline.ReleaseAndGetAddressOf())));
}

void Bunny::LoadModel()
{
	m_bunny.LoadModel(m_device.Get(), &m_commandQueue, "..\\..\\Models\\Bunny.obj");
}

void Bunny::OnRender(bool vSync)
{
	auto commandList = m_commandQueue.GetCommandList();

	auto currentBackBuffer = m_swapChain.GetCurrentBackBuffer();

	m_currentBackBufferIndex = m_swapChain.GetCurrentBackBufferIndex();

	auto currentRTV = m_swapChain.CurrentRenderTargetView();

	auto currentDSV = m_swapChain.CurrentDepthStencilView();

	auto viewPort = m_swapChain.GetViewPort();

	auto scissorRect = m_swapChain.GetScissorRect();

	auto vertexBufferView = m_bunny.GetVertexBufferView();

	auto indexBufferView = m_bunny.GetIndexBufferView();

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, 
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	
	commandList->ResourceBarrier(1, &barrier);

	float clearColour[] {0.0f, 0.0f, 0.0f, 1.0f};
	commandList->ClearRenderTargetView(currentRTV, clearColour, 0, nullptr);

	commandList->ClearDepthStencilView(currentDSV, D3D12_CLEAR_FLAG_DEPTH | 
		D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->SetPipelineState(m_graphicsPipeline.Get());

	commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

	commandList->IASetIndexBuffer(&indexBufferView);
	
	commandList->RSSetViewports(1, &viewPort);

	commandList->RSSetScissorRects(1, &scissorRect);

	commandList->OMSetRenderTargets(1, &currentRTV, false, &currentDSV);

	commandList->SetGraphicsRoot32BitConstants(0, sizeof(m_cbMatrix) / 4, &m_cbMatrix, 0);

	commandList->SetGraphicsRoot32BitConstants(1, sizeof(m_cbMatrix2) / 4, &m_cbMatrix2, 0);

	m_bunny.Draw(commandList.Get());

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer, 
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	commandList->ResourceBarrier(1, &barrier);

	m_fenceValuesPerFrame[m_currentBackBufferIndex] = 
		m_commandQueue.ExecuteCommandList(commandList.Get());
	
	m_currentBackBufferIndex = m_swapChain.Present(vSync);

	m_commandQueue.MoveToNextFrame(m_fenceValuesPerFrame[m_currentBackBufferIndex]);
}

void Bunny::OnUpdate(float time)
{
	// auto theta = XMConvertToRadians(time * 90.0f);
	// auto phi = XMConvertToRadians(70);
	// auto radius = 4.0f;

	// auto epsilon = std::numeric_limits<float>::epsilon();
	// phi = std::clamp(phi, epsilon, XM_PI - epsilon);

	// auto x = radius * DirectX::XMScalarSin(phi) * 
	// 	DirectX::XMScalarCos(theta);
	
	// auto z = radius * DirectX::XMScalarSin(phi) * 
	// 	DirectX::XMScalarSin(theta);
	
	// auto y = radius * DirectX::XMScalarCos(phi);

	auto model = DirectX::XMMatrixRotationY(XMConvertToRadians(90.0f * time));
	model *= DirectX::XMMatrixTranslation(0.0f, -1.0f, 0.0f);

	auto eyePosition = DirectX::XMVectorSet(0.0f, 0.0f, -2.5f, 1.0f);	
	
	auto focusPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	
	auto upDirection = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	
	auto view = DirectX::XMMatrixLookAtLH(eyePosition, focusPosition, 
		upDirection);

	auto proj = DirectX::XMMatrixPerspectiveFovLH(XM_PIDIV2, 
			m_swapChain.GetAspectRatio(), 0.1f, 100.0f);

	auto modelViewProj = model * view * proj;

	DirectX::XMStoreFloat4x4(&m_cbMatrix, modelViewProj);
	DirectX::XMStoreFloat4x4(&m_cbMatrix2, model);
}