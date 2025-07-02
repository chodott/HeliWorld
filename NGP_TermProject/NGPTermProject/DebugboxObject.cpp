#include "DebugboxObject.h"

CDebugboxObject::CDebugboxObject() :CGameObject(1, 1)
{
}

CDebugboxObject::~CDebugboxObject()
{
}

void CDebugboxObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_ppMaterials)
	{
		m_ppMaterials[0]->m_xmf4AmbientColor = debugColor;
		m_ppMaterials[0]->UpdateShaderVariables(pd3dCommandList);
		if (m_ppMaterials[0]->m_pTexture)
		{
			m_ppMaterials[0]->m_pTexture->UpdateShaderVariables(pd3dCommandList);
			if (m_pcbMappedGameObject)
			{
				XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4Texture, XMMatrixTranspose(XMLoadFloat4x4(&m_ppMaterials[0]->m_pTexture->m_xmf4x4Texture)));
			}
		}
	}

	pd3dCommandList->SetGraphicsRootDescriptorTable(13, m_d3dCbvGPUDescriptorHandle);


	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i])
			{
				m_ppMeshes[i]->Render(pd3dCommandList);
			}
		}
	}

}
