#include"stdafx.h"
#include"HealObjectShader.h"



CHealObjectsShader::CHealObjectsShader()
{
}

CHealObjectsShader::~CHealObjectsShader()
{
}

D3D12_RASTERIZER_DESC CHealObjectsShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	//d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
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

D3D12_BLEND_DESC CHealObjectsShader::CreateBlendState()
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
	//
	//	D3D12_BLEND_DESC d3dBlendDesc;
	//::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	//d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	//d3dBlendDesc.IndependentBlendEnable = FALSE;
	//d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	//d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	//d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	//d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	//d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	//d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	//d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	//d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	//d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	//d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	//
	//return(d3dBlendDesc);
}

void CHealObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{

	m_pHealTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pHealTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/HealItem.dds", RESOURCE_TEXTURE2D, 0);

	//->메테리얼 생성 텍스쳐와 쉐이더를 넣어야되는데 쉐이더이므로 안 넣어도 됨
	m_pHealMaterial = new CMaterial();
	m_pHealMaterial->SetTexture(m_pHealTexture);

	m_pHealTexturedMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 10.f, 10.5f, 5.f);

	m_nObjects = 10;
	m_ppObjects = new CGameObject * [m_nObjects];
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	//	CreateShader(pd3dDevice,  pd3dCommandList,pd3dGraphicsRootSignature);
	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 10);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, m_pHealTexture, 0, 11);

	int nObjects = 0;
	for (int i = 0; i < m_nObjects; i++) {
		pHealObject = new CItemObject();
		pHealObject->SetMesh(0, m_pHealTexturedMesh);
		pHealObject->SetMaterial(0, m_pHealMaterial);
		pHealObject->SetActive(false);
		pHealObject->SetPosition(100.f, 100.f, 10*i);
		pHealObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nObjects));
		m_ppObjects[nObjects++] = pHealObject;
	}
	//m_BulletList.push_back(pBullet);
}

void CHealObjectsShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}
void CHealObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];//파이프라인을 배열로 여기서 만든다.

	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}
D3D12_INPUT_LAYOUT_DESC CHealObjectsShader::CreateInputLayout()
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
D3D12_SHADER_BYTECODE CHealObjectsShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}
D3D12_SHADER_BYTECODE CHealObjectsShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}
void CHealObjectsShader::AnimateObjects(float fTimeElapsed)
{
	
}


bool CHealObjectsShader::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam, float* fTimeelapsed)
{
	return 0;
}
void CHealObjectsShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CHealObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int npipelinestate)
{
	CShader::Render(pd3dCommandList, pCamera, npipelinestate);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j] && m_ppObjects[j]->GetActive())
		{
			m_ppObjects[j]->Animate(0.16f); //->헬기 
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}


