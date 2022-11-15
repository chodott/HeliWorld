#include"stdafx.h"
#include "CMultiSpriteObject.h"

CMultiSpriteObjects::CMultiSpriteObjects() :CGameObject(1, 1)
{
}

CMultiSpriteObjects::~CMultiSpriteObjects()
{
}

void CMultiSpriteObjects::Animate(float fTimeElapsed)
{
	if (m_ppMaterials && m_ppMaterials[0]->m_pTexture)
	{
		m_fTime += fTimeElapsed * 0.5f;
		if (m_fTime >= m_fSpeed) m_fTime = 0.0f;
		m_ppMaterials[0]->m_pTexture->AnimateRowColumn(m_fTime);
	}
}

void CMultiSpriteObjects::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_ppMaterials)
	{
		if (m_ppMaterials[0]->m_pShader)//->½¦ÀÌ´õ 
		{
			m_ppMaterials[0]->m_pShader->Render(pd3dCommandList, pCamera);
			m_ppMaterials[0]->m_pShader->UpdateShaderVariables(pd3dCommandList);

			UpdateShaderVariables(pd3dCommandList);
		}
		if (m_ppMaterials[0]->m_pTexture)
		{
			m_ppMaterials[0]->m_pTexture->UpdateShaderVariables(pd3dCommandList);
			if (m_pcbMappedGameObject) XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4Texture, XMMatrixTranspose(XMLoadFloat4x4(&m_ppMaterials[0]->m_pTexture->m_xmf4x4Texture)));
		}
	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(13, m_d3dCbvGPUDescriptorHandle);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Render(pd3dCommandList);
		}
	}
}
