#include "stdafx.h"
#include "Missileobject.h"


CMissleObject::CMissleObject() :CGameObject(1, 1)
{
	SetOOBB(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.25f, 1.25f, 2.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	SetActive(false);
}

CMissleObject::~CMissleObject()
{
}

void CMissleObject::AnimateObject(float fElapsedTime)
{
	//if (GetActive())
	//{
	//	if (m_fRotationSpeed != 0.0f)
	//		Rotate(m_xmf3RotationAxis, m_fRotationSpeed * fElapsedTime);
	//	// 총알의 이동속도가 0이 아니면 이동을 한다.
	//	m_fMovingSpeed = 100;
	//	if (m_fMovingSpeed != 0.0f)
	//		Move(m_xmf3MovingDirection, m_fMovingSpeed * fElapsedTime);
	//	// 총알의 충돌박스를 계속 애니메이트 해주어야 총알의 위치가 변함에따라 충돌 박스도 같이 위치가 변한다.
	//	SetOOBB(GetPosition(), XMFLOAT3(5, 5, 5), XMFLOAT4(0., 0., 0., 1.));
	//	//XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
	//}
}

void CMissleObject::OnPrepareRender()
{
}

void CMissleObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_ppMaterials)
	{
		if (m_ppMaterials[0]->m_pShader)//->쉐이더 
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
void CMissleObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, MissileInfoPacket* MissilePacket)
{
	bool bServerActive = MissilePacket->active;
	if (bServerActive)
	{
		bServerLife = true;
		SetActive(true);
	}
	if (bServerLife == true && bServerActive == false)
	{
		bServerLife = false;
		SetActive(false);
	}
	if (!GetActive()) return;
	
	XMFLOAT3 newPosition = ConvertInt16tofloat3(MissilePacket->positionX, MissilePacket->positionY, MissilePacket->positionZ, MAP_SCALE);
	if (bLocalMissile)
	{
		Move(GetMovingDirection(), movingSpeed * fTimeElapsed);
		if (bServerActive)
		{
			XMFLOAT3 resultPosition;
			XMVECTOR prevPosition = XMLoadFloat3(&GetPosition());
			XMVECTOR nextPosition = XMLoadFloat3(&newPosition);
			XMVECTOR curPosition = XMVectorLerp(prevPosition, nextPosition, 0.1f);

			XMStoreFloat3(&resultPosition, curPosition);
			SetPosition(resultPosition);
		}
	}
	else SetPosition(newPosition);

}
void CMissleObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerInfoPacket* MissilePacket)
{
	//SetActive(MissilePacket->active);

	//SetPosition(MissilePacket->position);
	//rotate


	//if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	//if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);
}
void CMissleObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CMissleObject::Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationAxis(xmf3RotationAxis, fAngle);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
void CMissleObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}


void CMissleObject::RotatePYR(XMFLOAT3& xmf3RotationAxis)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(xmf3RotationAxis.x), XMConvertToRadians(xmf3RotationAxis.y), XMConvertToRadians(xmf3RotationAxis.z));
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
