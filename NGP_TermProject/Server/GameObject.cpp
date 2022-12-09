#include "GameObject.h"

GameObject::GameObject()
{
	m_xmf4x4World = Matrix4x4::Identity();
	SetOOBB(XMFLOAT3(0, 0, 0), XMFLOAT3(10, 10, 10), XMFLOAT4(0, 0, 0, 1));
	m_xmf3Right = XMFLOAT3(0, 0, 0);
	m_xmf3Up = XMFLOAT3(0, 0, 0);
	m_xmf3Look = XMFLOAT3(0, 0, 0);
}

void GameObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
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

	m_fxPos = x;
	m_fyPos = y;
	m_fzPos = z;

	m_xmf3Position = XMFLOAT3{ x,y,z };

	//m_xmOOBB.Center = { x,y,z };
	m_xmOOBB = BoundingOrientedBox{ m_xmf3Position, XMFLOAT3(10,10,10), XMFLOAT4(0,0,0,1) };
}

void GameObject::SetActive(bool active)
{
	m_bActive = active;
}

bool GameObject::GetActive()
{
	return m_bActive;
}



CPlayer::CPlayer()
{
	for (int i = 0; i < 8; ++i)
	{
		m_pMissiles[i] = new CMissileObject();
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	m_fOldxPos = m_xmf3Position.x;
	m_fOldyPos = m_xmf3Position.y;
	m_fOldzPos = m_xmf3Position.z;

	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

	m_fxPos = m_xmf3Position.x;
	m_fyPos = m_xmf3Position.y;
	m_fzPos = m_xmf3Position.z;
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

void CPlayer::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
		if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
	}
	if (y != 0.0f)
	{
		m_fYaw += y;
		if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
		if (m_fYaw < 0.0f) m_fYaw += 360.0f;
	}
	if (z != 0.0f)
	{
		m_fRoll += z;
		if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
		if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
	}

	//float sensitivity = 0.1f;
	//float degreeX = x * sensitivity;
	//float radianX = degreeX * (180.0f / DirectX::XM_PI);
	//float degreeY = y * sensitivity;
	//float radianY = degreeY * (180.0f / DirectX::XM_PI);

	if (y != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
	}
	if (x != 0.0f)
	{
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
	}

	//m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	//m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);


	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
}


void CPlayer::LaunchMissile()
{
	for (int i = 0; i < 8; ++i)
	{
		char temp = activatedMissiles;
		if (!((temp >> i) & 0x01))
		{
			activatedMissiles |= (0x01 << i);
			m_pMissiles[i]->m_bActive = true;
			m_pMissiles[i]->SetPosition(m_fxPos, m_fyPos, m_fzPos);
			m_pMissiles[i]->m_xmf3Look = m_xmf3Look;
			break;
		}
	}
}

void CPlayer::UpdateMissiles()
{
	for (auto& missile : m_pMissiles)
	{
		if (missile->GetActive())
		{
			missile->Move();
		}
	}
}

void CPlayer::Update(float Distance, bool updateVelocity)
{
	RecalculateLook();
	RecalculateRight();
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	Rotate(m_deltaY, m_deltaX, 0.f);
	m_deltaX = 0.f;
	m_deltaY = 0.f;

	if (playerKey)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0.f, 0.f, 0.f);

		if (playerKey & option0)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, Distance);
			playerKey &= ~option0;
		}
		if (playerKey & option1)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -Distance);
			playerKey &= ~option1;
		}
		if (playerKey & option2)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -Distance);
			playerKey &= ~option2;
		}
		if (playerKey & option3)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, Distance);
			playerKey &= ~option3;
		}
		if (playerKey & option4)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -Distance);
			playerKey &= ~option4;
		}
		if (playerKey & option5)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, Distance);
			playerKey &= ~option5;
		}
		Move(xmf3Shift, updateVelocity);

		//m_xmOOBB.Center = GetCurPos();
		m_xmOOBB = BoundingOrientedBox{ m_xmf3Position, XMFLOAT3(10,10,10), XMFLOAT4(0,0,0,1) };

		//Attack
		if (playerKey & option6)
		{
			LaunchMissile();
			playerKey &= ~option6;
		}
		m_xmf4x4World._41 = m_xmf3Position.x;
		m_xmf4x4World._42 = m_xmf3Position.y;
		m_xmf4x4World._43 = m_xmf3Position.z;
	}


	UpdateMissiles();
}


void CMissileObject::Move()
{
	m_fMovingSpeed = 0.1f;
	if (m_fMovingSpeed != 0.0f)
		Move(m_xmf3Look, m_fMovingSpeed);
	SetOOBB(m_xmf3Position, XMFLOAT3(1, 1, 1), XMFLOAT4(0., 0., 0., 1.));
}


void CMissileObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}