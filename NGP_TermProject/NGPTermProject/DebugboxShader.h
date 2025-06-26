#pragma once
#pragma once

#pragma once
#include"stdafx.h"
#include"Shader.h"


class CDebugboxShader : public CObjectsShader
{
public:
	CDebugboxShader();
	virtual ~CDebugboxShader();

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


private:
	CTexture* m_pDebugboxTexture{ NULL };
	CCubeMeshTextured* m_pBoxMesh{ NULL };
};
