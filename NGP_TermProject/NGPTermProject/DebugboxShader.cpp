#include "DebugboxShader.h"

CDebugboxShader::CDebugboxShader()
{
}

CDebugboxShader::~CDebugboxShader()
{
}

D3D12_RASTERIZER_DESC CDebugboxShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_WIREFRAME;
	//	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	return(d3dRasterizerDesc);
}

D3D12_BLEND_DESC CDebugboxShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	return(d3dBlendDesc);
}

void CDebugboxShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}

void CDebugboxShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CDebugboxShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int npipelinestate)
{
	CShader::Render(pd3dCommandList, pCamera, npipelinestate);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j] && m_ppObjects[j]->GetActive())
		{
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
			//m_ppObjects[j]->SetActive(false);
		}
	}
}


void CDebugboxShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pDebugboxTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pDebugboxTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/HealItem.dds", RESOURCE_TEXTURE2D, 0);

	m_pMaterial = new CMaterial();
	m_pMaterial->SetTexture(m_pDebugboxTexture);

	m_pBoxMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 20.f, 20.f, 20.f);

	m_nObjects = 10;
	m_ppObjects = new CGameObject * [m_nObjects];
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	//	CreateShader(pd3dDevice,  pd3dCommandList,pd3dGraphicsRootSignature);
	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 10);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, m_pDebugboxTexture, 0, 11);

	for (int i = 0; i < m_nObjects; i++) {
		m_ppObjects[i] = new CDebugboxObject();
		m_ppObjects[i]->SetMesh(0, m_pBoxMesh);
		m_ppObjects[i]->SetMaterial(0, m_pMaterial);
		m_ppObjects[i]->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * i));
		m_ppObjects[i]->SetActive(false);
	}
}

void CDebugboxShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];//파이프라인을 배열로 여기서 만든다.

	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

D3D12_INPUT_LAYOUT_DESC CDebugboxShader::CreateInputLayout()
{
	UINT nInputElementDescs = 2;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}
D3D12_SHADER_BYTECODE CDebugboxShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}
D3D12_SHADER_BYTECODE CDebugboxShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}