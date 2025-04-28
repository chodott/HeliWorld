#include "stdafx.h"
#include "ItemObject.h"


CItemObject::CItemObject() :CGameObject(1, 1)
{
	SetOOBB(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.25f, 1.25f, 2.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

CItemObject::~CItemObject()
{
}

void CItemObject::RotatePYR(XMFLOAT3& xmf3RotationAxis)
{
	
		XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(xmf3RotationAxis.x), XMConvertToRadians(xmf3RotationAxis.y), XMConvertToRadians(xmf3RotationAxis.z));
		m_xmf4x4World = Matrix4x4::Identity();
		m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	
}

void CItemObject::AnimateObject(float fElapsedTime)
{
	if (GetActive()) {
		if (m_fRotationSpeed != 0.0f)
			Rotate(m_xmf3RotationAxis, m_fRotationSpeed * fElapsedTime);
	}
}

void CItemObject::OnPrepareRender()
{
}

void CItemObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
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
void CItemObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CItemObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, ItemInfoPacket* ItemPacket)
{

	SetActive(ItemPacket->active);
	SetPosition(ConvertInt16tofloat3(ItemPacket->positionX, ItemPacket->positionY, ItemPacket->positionZ,MAP_SCALE));

	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}

void CItemObject::Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationAxis(xmf3RotationAxis, fAngle);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
