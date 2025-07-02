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
	m_pMissileTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Missile2.dds", RESOURCE_TEXTURE2D, 0);

	//->���׸��� ���� �ؽ��Ŀ� ���̴��� �־�ߵǴµ� ���̴��̹Ƿ� �� �־ ��
	m_pMissileMaterial = new CMaterial();
	m_pMissileMaterial->SetTexture(m_pMissileTexture);

	m_pMissileTexturedMesh = new CCubeMeshTextured(pd3dDevice, pd3dCommandList, 2.f, 2.f, 10.f);
	CMesh* pMeshIlluminated = new CSphereMeshIlluminated(pd3dDevice, pd3dCommandList, 3.0f, 20, 20);
	m_nObjects = 32;//0~32 
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

		pMissleObject->SetMesh(0, pMeshIlluminated);
		pMissleObject->SetMaterial(0, m_pMissileMaterial);
		pMissleObject->m_xmf4x4World = Matrix4x4::Identity();
		pMissleObject->SetPosition(0.f, 0.f, i);
		//pMissleObject->Rotate(0, 270, 0);
		//pMissleObject->SetRotation();
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
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];//������������ �迭�� ���⼭ �����.

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
	// �÷��̾��� �Ѿ� ����Ʈ�� ������ ���� ��ȸ�ϸ鼭, �ִϸ���Ʈ �����ش�.
	for (int i = 0; i < m_nObjects; i++)
	{
		m_ppObjects[i]->AnimateObject(fTimeElapsed);
		//// �Ѿ��� �浹�̶��
		//if ((m_ppObjects[i]->GetCollision() == true)
		//{
		//	// �����ؾ��ϴ� Fire��ƼŬ ID�� �Ѱ��ش�.
		//	//m_pFireParticleShader->SetDeleteFireParticleID(((CBullet*)(*iter))->GetID());

		//	// �������� ������ ��ƼŬ�� �Ѿ��� ��ġ�� ����
		////	m_pExplosionParticleShader->Initialize((*iter)->GetPosition());

		//	// �Ѿ� ����
		//	delete (*iter);
		//	iter = m_BulletList.erase(iter);
		//}
		//// �浹�� �Ѿ��� �ƴϸ�
		// �÷��̾� ��ġ�� �Ѿ��� ��ġ �Ÿ��� ����ϴ� �����̴�. 
		distance = sqrt((pow((m_ppObjects[i]->GetPosition().x - m_pPlayer->GetPosition().x), 2.0)
			+ pow((m_ppObjects[i]->GetPosition().y - m_pPlayer->GetPosition().y), 2.0)
			+ pow((m_ppObjects[i]->GetPosition().z - m_pPlayer->GetPosition().z), 2.0)));

		// �÷��̾�� �Ѿ��� �Ÿ��� 250m���� Ŀ����, �Ѿ��� ��ȿ��Ÿ��� ����ŹǷ�
			// �Ѿ��� ��� �׸��� �ʰ�, �����־�� �����ӷ���Ʈ�� �ø� �� �ִ�.
		if (distance >= MaxBulletDistance)
		{
			// �����ؾ��ϴ� Fire��ƼŬ ID�� �Ѱ��ش�.
			//m_pFireParticleShader->SetDeleteFireParticleID(((CBullet*)(*iter))->GetID());

			// �������� ������ ��ƼŬ�� �Ѿ��� ��ġ�� ����
			//m_pExplosionParticleShader->Initialize((*iter)->GetPosition());

			// �Ѿ� ����
			//m_ppObjects[i]->SetActive(false);
			//	cout << "�÷��̾� �Ѿ� �Ÿ���� ����" << endl;
		}
		else
		{


		}
	}
}


bool CMissileObjectsShader::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam, float* fTimeelapsed)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
			// �Ѿ� �߻� Ű
		case VK_CONTROL:
		{


			if (m_pPlayer && *fTimeelapsed > 3)
			{
				*fTimeelapsed = 0.3;
				if (m_MissileCount == 10) {
					m_MissileCount = 0;
				}
				// ID�� 1������ ����
				m_ppObjects[m_MissileCount]->SetActive(true);

				m_ppObjects[m_MissileCount]->SetLook(m_pPlayer->GetLook());
				// �÷��̾��� Up����, Right���͵� �Ȱ��� �������־�� �÷��̾ ȸ������ ��,
				// �Ѿ� ��絵 ȸ���� �� ������� �ٲ��.
				m_ppObjects[m_MissileCount]->SetUp(m_pPlayer->GetUpVector());
				m_ppObjects[m_MissileCount]->SetRight(m_pPlayer->GetRightVector());
				// �Ѿ��� ������ġ�� �÷��̾��� ��ġ�� ����
				m_ppObjects[m_MissileCount]->SetPosition(m_pPlayer->GetPosition());
				m_ppObjects[m_MissileCount]->SetScale(0.2, 0.2, 1.0);
				// �Ѿ��� ���ư��� ������ �Ѿ��� �ٶ󺸴� �������� �ش�.
				m_ppObjects[m_MissileCount]->SetMovingDirection(m_pPlayer->GetLook());
				//m_BulletList.push_back(pBullet);
				std::cout << "��Ʈ�� Ű ���� " << std::endl;
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
		if (m_ppObjects[j] && m_ppObjects[j]->GetActive())
		{
			m_ppObjects[j]->Animate(0.16f); //->��� 
			m_ppObjects[j]->Render(pd3dCommandList, pCamera);
		}
	}
}


