#include "DebugDrawManager.h"


DebugDrawManager& DebugDrawManager::Get()
{
	static DebugDrawManager instance;
	return instance;
}

void DebugDrawManager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, int npipelinestate)
{
	m_pDebugboxShader->Render(pd3dCommandList, pCamera, npipelinestate);
	count = 0;

}

void DebugDrawManager::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pDebugboxShader = new CDebugboxShader();

	m_pDebugboxShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pDebugboxShader->BuildObjects(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
}


void DebugDrawManager::AddDebugCube(XMFLOAT3& position, XMFLOAT3& rotation, float color)
{
	if (m_pDebugboxShader == NULL) return;
	CGameObject* debugTarget = m_pDebugboxShader->m_ppObjects[count++];
	debugTarget->SetActive(true);
	debugTarget->RotatePYR(rotation);
	debugTarget->SetPosition(position);
}
