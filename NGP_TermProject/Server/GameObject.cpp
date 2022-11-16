#include "GameObject.h"

void GameObject::Move()
{
}

void GameObject::Rotate(float Pitch, float Yaw, float Roll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(Pitch), XMConvertToRadians(Yaw), XMConvertToRadians(Roll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

	//UpdateTransform(NULL);

}

void GameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void Player::Move(DWORD Direction, float Distance, bool updateVelocity)
{
	//if ((dwDirection != 0) || (cxDelta != 0.0f) || (cyDelta != 0.0f))
	//{
	//	if (cxDelta || cyDelta)
	//	{
	//		if (pKeysBuffer[VK_RBUTTON] & 0xF0)
	//			m_pPlayer->Rotate(cyDelta, 0.0f, -cxDelta);
	//		else
	//			m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
	//	}
	//	if (dwDirection) m_pPlayer->Move(dwDirection, 1.21f, true);
	//}
	//m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
}

void Player::Rotate(float Pitch, float Yaw, float Roll)
{
	//if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	//{
	//	if (Pitch != 0.0f)
	//	{
	//		m_fPitch += x;
	//		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
	//		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	//	}
	//	if (y != 0.0f)
	//	{
	//		m_fYaw += y;
	//		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
	//		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	//	}
	//	if (z != 0.0f)
	//	{
	//		m_fRoll += z;
	//		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
	//		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	//	}
	//	m_pCamera->Rotate(x, y, z);
	//	if (y != 0.0f)
	//	{
	//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
	//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
	//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	//	}
	//}

	//else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	//{
	//	m_pCamera->Rotate(x, y, z);
	//	if (x != 0.0f)
	//	{
	//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
	//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
	//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	//	}
	//	if (y != 0.0f)
	//	{
	//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
	//		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
	//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	//	}
	//	if (z != 0.0f)
	//	{
	//		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
	//		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	//		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	//	}

	}

//	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
//	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
//	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
//}

