#include "stdafx.h"
#include "Missileobject.h"
#include "DebugDrawManager.h"


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
	//	// �Ѿ��� �̵��ӵ��� 0�� �ƴϸ� �̵��� �Ѵ�.
	//	m_fMovingSpeed = 100;
	//	if (m_fMovingSpeed != 0.0f)
	//		Move(m_xmf3MovingDirection, m_fMovingSpeed * fElapsedTime);
	//	// �Ѿ��� �浹�ڽ��� ��� �ִϸ���Ʈ ���־�� �Ѿ��� ��ġ�� ���Կ����� �浹 �ڽ��� ���� ��ġ�� ���Ѵ�.
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
	if (!GetActive()) return;
	if (m_ppMaterials)
	{
		if (m_ppMaterials[0]->m_pShader)//->���̴� 
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


}
void CMissleObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, MissileInfoPacket& prevPacket, MissileInfoPacket& nextPacket, float lerpAlpha)
{
	if (GetLocal() == true)
	{
		if (GetActive() == false)
		{
			return;
		}

		Move(GetMovingDirection(), fTimeElapsed * movingSpeed);
	}

	else
	{
		SetActive(prevPacket.active);
		XMVECTOR prevPosition = XMLoadFloat3(&prevPacket.position);
		XMVECTOR nextPosition = XMLoadFloat3(&nextPacket.position);

		XMVECTOR renderPosition = XMVectorLerp(prevPosition, nextPosition, lerpAlpha);
		XMFLOAT3 resultPosition;

		XMStoreFloat3(&resultPosition, renderPosition);
		SetPosition(resultPosition);
	}

}

void CMissleObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	m_xmf3PredictPosition.x += vDirection.x * fSpeed;
	m_xmf3PredictPosition.z += vDirection.z * fSpeed;
	m_xmf3PredictPosition.y += vDirection.y * fSpeed;

	//SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CMissleObject::ApplyServerResult(bool active)
{
	SetActive(active);
}

void CMissleObject::ApplyVisualSmoothing(float fTimeElapsed)
{
	if (!GetActive()) return;
	XMFLOAT3 renderPosition;
	{
		renderPosition = GetPredictPosition();
	}
	SetPosition(renderPosition);

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
