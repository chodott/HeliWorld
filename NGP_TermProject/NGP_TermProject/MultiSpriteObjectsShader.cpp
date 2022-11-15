#include"stdafx.h"
#include"MultiSpriteObjectsShader.h"
CMultiSpriteObjectsShader::CMultiSpriteObjectsShader()
{
}

CMultiSpriteObjectsShader::~CMultiSpriteObjectsShader()
{
}

D3D12_RASTERIZER_DESC CMultiSpriteObjectsShader::CreateRasterizerState()
{
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	//	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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

D3D12_BLEND_DESC CMultiSpriteObjectsShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = TRUE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
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

void CMultiSpriteObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	/*CTexture* ppSpriteTextures[2];
	ppSpriteTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 8, 8);
	ppSpriteTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Explode_8x8.dds", RESOURCE_TEXTURE2D, 0);
	ppSpriteTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 6, 6);
	ppSpriteTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Explosion_6x6.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* ppSpriteMaterials[2];
	ppSpriteMaterials[0] = new CMaterial();
	ppSpriteMaterials[0]->SetTexture(ppSpriteTextures[0]);
	ppSpriteMaterials[1] = new CMaterial();
	ppSpriteMaterials[1]->SetTexture(ppSpriteTextures[1]);

	CTexturedRectMesh* pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 20;

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[0], 0, 11);
	CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[1], 0,11);

	m_ppObjects = new CGameObject * [m_nObjects];

	XMFLOAT3 xmf3Position = XMFLOAT3(1030.0f, 180.0f, 1410.0f);
	CMultiSpriteObjects* pSpriteObject = NULL;
	for (int j = 0; j < m_nObjects; j++)
	{
		pSpriteObject = new CMultiSpriteObjects();
		pSpriteObject->SetMesh(0, pSpriteMesh);
		pSpriteObject->SetMaterial(0, ppSpriteMaterials[j%2]);
		pSpriteObject->SetPosition(XMFLOAT3(xmf3Position.x, xmf3Position.y, xmf3Position.z));
		pSpriteObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j));
		pSpriteObject->m_fSpeed = 3.0f / (ppSpriteTextures[j % 2]->m_nRows * ppSpriteTextures[j % 2]->m_nCols);
		m_ppObjects[j] = pSpriteObject;
	}*/

	CTexture* ppSpriteTextures[2];
	ppSpriteTextures[1] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 8, 8);
	ppSpriteTextures[1]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Explode_8x8.dds", RESOURCE_TEXTURE2D, 0);
	ppSpriteTextures[0] = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1, 6, 6);
	ppSpriteTextures[0]->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Explosion_6x6.dds", RESOURCE_TEXTURE2D, 0);

	CMaterial* ppSpriteMaterials[2];
	ppSpriteMaterials[1] = new CMaterial();
	ppSpriteMaterials[1]->SetTexture(ppSpriteTextures[0]);
	ppSpriteMaterials[0] = new CMaterial();
	ppSpriteMaterials[0]->SetTexture(ppSpriteTextures[1]);

	CTexturedRectMesh* pSpriteMesh = new CTexturedRectMesh(pd3dDevice, pd3dCommandList, 50.0f, 50.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	m_nObjects = 1;

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 2);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[0], 0, 11);
	CreateShaderResourceViews(pd3dDevice, ppSpriteTextures[1], 0,11);

	m_ppObjects = new CGameObject * [m_nObjects];

	XMFLOAT3 xmf3Position = XMFLOAT3(1030.0f, 180.0f, 1410.0f);
	CMultiSpriteObjects* pSpriteObject = NULL;
	for (int j = 0; j < m_nObjects; j++)
	{
		pSpriteObject = new CMultiSpriteObjects();
		pSpriteObject->SetMesh(0, pSpriteMesh);
		pSpriteObject->SetMaterial(0,ppSpriteMaterials[j]);
		pSpriteObject->SetPosition(XMFLOAT3(xmf3Position.x, xmf3Position.y, xmf3Position.z));
		pSpriteObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * j));
		pSpriteObject->m_fSpeed = 3.0f / (ppSpriteTextures[j]->m_nRows * ppSpriteTextures[j]->m_nCols);
		m_ppObjects[j] = pSpriteObject;
	}
}

void CMultiSpriteObjectsShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CMultiSpriteObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int pipelinestate)
{

	if (m_bActive)
	{
		XMFLOAT3 xmf3CameraPosition = pCamera->GetPosition();
		CPlayer* pPlayer = pCamera->GetPlayer();
		XMFLOAT3 xmf3PlayerPosition = pPlayer->GetPosition();
		XMFLOAT3 xmf3PlayerLook = pPlayer->GetLookVector();
		xmf3PlayerPosition.y += 5.0f;
		XMFLOAT3 xmf3Position = Vector3::Add(xmf3PlayerPosition, Vector3::ScalarProduct(xmf3PlayerLook, 50.0f, false));
		for (int j = 0; j < m_nObjects; j++)
		{
			if (m_ppObjects[j])
			{
				m_ppObjects[j]->SetPosition(xmf3Position);
				m_ppObjects[j]->SetLookAt(xmf3CameraPosition, XMFLOAT3(0.0f, 1.0f, 0.0f));
			}
		}

		CObjectsShader::Render(pd3dCommandList, pCamera);
	}
}

void CMultiSpriteObjectsShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}

void CMultiSpriteObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];//파이프라인을 배열로 여기서 만든다.

	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

D3D12_INPUT_LAYOUT_DESC CMultiSpriteObjectsShader::CreateInputLayout()
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

D3D12_SHADER_BYTECODE CMultiSpriteObjectsShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSSpriteAnimation", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CMultiSpriteObjectsShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSSpriteAnimation", "ps_5_1", &m_pd3dPixelShaderBlob));
}

