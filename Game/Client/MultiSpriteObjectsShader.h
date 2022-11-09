#pragma once
#include"stdafx.h"
#include"Shader.h"
#include"Player.h"
#include"CMultiSpriteObject.h"


class CMultiSpriteObjectsShader : public CObjectsShader
{
public:
	CMultiSpriteObjectsShader();
	virtual ~CMultiSpriteObjectsShader();

	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual void ReleaseObjects();
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int pipelinestate);

	virtual void ReleaseUploadBuffers();
	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();


};