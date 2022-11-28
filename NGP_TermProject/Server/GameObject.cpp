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

void GameObject::SetActive(bool active)
{
	m_bActive = active;
}

bool GameObject::GetActive()
{
	return m_bActive;
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	/*if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}*/
	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
	//	m_pCamera->Move(xmf3Shift); ->
}

void CPlayer::Move(DWORD Direction, float Distance, bool updateVelocity)
{
	if (Direction)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (Direction & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, Distance);
		if (Direction & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -Distance);
		if (Direction & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, Distance);
		if (Direction & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -Distance);
		if (Direction & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, Distance);
		if (Direction & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -Distance);
		Move(xmf3Shift, updateVelocity);
	}
}

void CPlayer::Move(char key, float Distance, bool updateVelocity)
{
	RecalculateLook();
	RecalculateRight();
	if (key)
	{
		int n = 0;
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if ((key >> n++) & option0) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, Distance);
		if ((key >> n++) & option1) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -Distance);
		if ((key >> n++) & option2) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -Distance);
		if ((key >> n++) & option3) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, Distance);
		Move(xmf3Shift, updateVelocity);
	}
}

