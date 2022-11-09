#include"stdafx.h"
#include"MissileObjectShader.h"



CMissileObjectsShader::CMissileObjectsShader()
{
}

CMissileObjectsShader::~CMissileObjectsShader()
{
}

D3D12_RASTERIZER_DESC CMissileObjectsShader::CreateRasterizerState()
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

D3D12_BLEND_DESC CMissileObjectsShader::CreateBlendState()
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

void CMissileObjectsShader::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	
	m_pMissileTexture = new CTexture(1, RESOURCE_TEXTURE2D, 0, 1);
	m_pMissileTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/stones.dds", RESOURCE_TEXTURE2D, 0);

	//->메테리얼 생성 텍스쳐와 쉐이더를 넣어야되는데 쉐이더이므로 안 넣어도 됨
	m_pMissileMaterial = new CMaterial();
	m_pMissileMaterial->SetTexture(m_pMissileTexture);

	m_pMissileTexturedMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 10.f, 10.5f, 5.f);

	m_nObjects=10;
	m_ppObjects = new CGameObject * [m_nObjects];
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255);

	//	CreateShader(pd3dDevice,  pd3dCommandList,pd3dGraphicsRootSignature);
	CreateCbvSrvDescriptorHeaps(pd3dDevice, m_nObjects, 7);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	CreateConstantBufferViews(pd3dDevice, m_nObjects, m_pd3dcbGameObjects, ncbElementBytes);
	CreateShaderResourceViews(pd3dDevice, m_pMissileTexture, 0, 11);

	int nObjects = 0;
	for (int i = 0; i < m_nObjects; i++) {
		pMissleObject = new CMissleObject();
		pMissleObject->SetMesh(0, m_pMissileTexturedMesh);
		pMissleObject->SetMaterial(0, m_pMissileMaterial);
		pMissleObject->SetPosition(0.f, 0.f, i);
		pMissleObject->SetCbvGPUDescriptorHandlePtr(m_d3dCbvGPUDescriptorStartHandle.ptr + (::gnCbvSrvDescriptorIncrementSize * nObjects));
		m_ppObjects[nObjects++] = pMissleObject;
	}
	//m_BulletList.push_back(pBullet);
}

void CMissileObjectsShader::ReleaseUploadBuffers()
{
	CObjectsShader::ReleaseUploadBuffers();
}
void CMissileObjectsShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];//파이프라인을 배열로 여기서 만든다.

	CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);

	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();

	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}
D3D12_INPUT_LAYOUT_DESC CMissileObjectsShader::CreateInputLayout()
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
D3D12_SHADER_BYTECODE CMissileObjectsShader::CreateVertexShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "VSBillBoardTextured", "vs_5_1", &m_pd3dVertexShaderBlob));
}
D3D12_SHADER_BYTECODE CMissileObjectsShader::CreatePixelShader()
{
	return(CShader::CompileShaderFromFile(L"Shaders.hlsl", "PSBillBoardTextured", "ps_5_1", &m_pd3dPixelShaderBlob));
}
void CMissileObjectsShader::AnimateObjects(float fTimeElapsed)
{
	double distance = 0.f;
	// 플레이어의 총알 리스트를 루프를 통해 순회하면서, 애니메이트 시켜준다.
	for (int i = 0; i<m_nObjects;i++)
	{
		m_ppObjects[i]->AnimateObject(fTimeElapsed);
		//// 총알이 충돌이라면
		//if ((m_ppObjects[i]->GetCollision() == true)
		//{
		//	// 삭제해야하는 Fire파티클 ID를 넘겨준다.
		//	//m_pFireParticleShader->SetDeleteFireParticleID(((CBullet*)(*iter))->GetID());

		//	// 마지막에 터지는 파티클을 총알의 위치에 생성
		////	m_pExplosionParticleShader->Initialize((*iter)->GetPosition());

		//	// 총알 삭제
		//	delete (*iter);
		//	iter = m_BulletList.erase(iter);
		//}
		//// 충돌된 총알이 아니면
		// 플레이어 위치와 총알의 위치 거리를 계산하는 공식이다. 
		distance = sqrt((pow((m_ppObjects[i]->GetPosition().x - m_pPlayer->GetPosition().x), 2.0)
			+ pow((m_ppObjects[i]->GetPosition().y - m_pPlayer->GetPosition().y), 2.0)
			+ pow((m_ppObjects[i]->GetPosition().z - m_pPlayer->GetPosition().z), 2.0)));

			// 플레이어와 총알의 거리가 250m보다 커지면, 총알의 유효사거리를 벗어난거므로
				// 총알을 계속 그리지 않고, 지워주어야 프레임레이트를 올릴 수 있다.
			if (distance >= MaxBulletDistance)
			{
				// 삭제해야하는 Fire파티클 ID를 넘겨준다.
				//m_pFireParticleShader->SetDeleteFireParticleID(((CBullet*)(*iter))->GetID());

				// 마지막에 터지는 파티클을 총알의 위치에 생성
				//m_pExplosionParticleShader->Initialize((*iter)->GetPosition());

				// 총알 삭제
				m_ppObjects[i]->SetActive(false);
			//	cout << "플레이어 총알 거리벗어남 삭제" << endl;
			}
			else
			{
				
			
			}
	}
}


bool CMissileObjectsShader::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam,float *fTimeelapsed)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
			// 총알 발사 키
		case VK_CONTROL:
		{
			 

			if (m_pPlayer&& *fTimeelapsed>3)
			{
				*fTimeelapsed = 0.3;
				if (m_MissileCount == 10) {
					m_MissileCount = 0;
				}
				// ID는 1번부터 시작
				m_ppObjects[m_MissileCount]->SetActive(true);
				
				m_ppObjects[m_MissileCount]->SetLook(m_pPlayer->GetLook());
				// 플레이어의 Up벡터, Right벡터도 똑같이 설정해주어야 플레이어가 회전했을 때,
				// 총알 모양도 회전이 된 모양으로 바뀐다.
				m_ppObjects[m_MissileCount]->SetUp(m_pPlayer->GetUpVector());
				m_ppObjects[m_MissileCount]->SetRight(m_pPlayer->GetRightVector());
				// 총알의 생성위치는 플레이어의 위치로 설정
				m_ppObjects[m_MissileCount]->SetPosition(m_pPlayer->GetPosition());
				m_ppObjects[m_MissileCount]->SetScale(0.2,0.2,1.0);
				// 총알이 나아가는 방향은 총알이 바라보는 방향으로 준다.
				m_ppObjects[m_MissileCount]->SetMovingDirection(m_pPlayer->GetLook());
				//m_BulletList.push_back(pBullet);
				std::cout << "컨트롤 키 눌림 " << std::endl;
				m_MissileCount++;
				//m_pFireParticleShader->Initialize(pBullet, m_BulletCount);
			}
			return true;
		}
		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}
void CMissileObjectsShader::ReleaseObjects()
{
	CObjectsShader::ReleaseObjects();
}

void CMissileObjectsShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int npipelinestate)
{
	CShader::Render(pd3dCommandList, pCamera, npipelinestate);

	for (int j = 0; j < m_nObjects; j++)
	{
		if (m_ppObjects[j]&& m_ppObjects[j]->GetActive())
		{
			m_ppObjects[j]->Animate(0.16f); //->헬기 
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}


