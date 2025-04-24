#include "GameObject.h"

GameObject::GameObject()
{
	m_xmf4x4World = Matrix4x4::Identity();
	InitOOBB();
	m_xmf3Right = XMFLOAT3(0, 0, 0);
	m_xmf3Up = XMFLOAT3(0, 0, 0);
	m_xmf3Look = XMFLOAT3(0, 0, 0);
}

CPlayer::CPlayer()
{
	// Init Missiles
	for (int i = 0; i < maxMissileNum; ++i)
	{
		m_pMissiles[i] = new CMissileObject();
	}
}

CPlayer::~CPlayer()
{
	// Delete Missiles
	for (int i = 0; i < maxMissileNum; ++i)
	{
		delete m_pMissiles[i];
	}
}

void GameObject::Rotate(float Pitch, float Yaw, float Roll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(Pitch), XMConvertToRadians(Yaw), XMConvertToRadians(Roll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
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

	MoveOOBB(m_xmf3Position);
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift)
{
	m_fOldxPos = m_xmf3Position.x;
	m_fOldyPos = m_xmf3Position.y;
	m_fOldzPos = m_xmf3Position.z;

	m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);

	m_fxPos = m_xmf3Position.x;
	m_fyPos = m_xmf3Position.y;
	m_fzPos = m_xmf3Position.z;
}

void CPlayer::Rotate(float x, float y, float z)
{
	if (x != 0.0f)
	{
		m_fPitch += x;
		if (m_fPitch > +maxPitch) { x -= (m_fPitch - maxPitch); m_fPitch = +maxPitch; }
		if (m_fPitch < -maxPitch) { x -= (m_fPitch + maxPitch); m_fPitch = -maxPitch; }
	}
	if (y != 0.0f)
	{
		m_fYaw += y;
		if (m_fYaw > 360.f) m_fYaw -= 360.f;
		if (m_fYaw < 0.f) m_fYaw += 360.f;
	}
	if (z != 0.0f)
	{
		m_fRoll += z;
		if (m_fRoll > +maxRoll) { z -= (m_fRoll - maxRoll); m_fRoll = +maxRoll; }
		if (m_fRoll < -maxRoll) { z -= (m_fRoll + maxRoll); m_fRoll = -maxRoll; }
	}


	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_fPitch), XMConvertToRadians(m_fYaw), XMConvertToRadians(m_fRoll));
	XMFLOAT4X4 tempMatrix = Matrix4x4::Identity();
	tempMatrix = Matrix4x4::Multiply(mtxRotate, tempMatrix);
	m_xmf3Right.x = tempMatrix._11, m_xmf3Right.y = tempMatrix._12, m_xmf3Right.z = tempMatrix._13;
	m_xmf3Up.x = tempMatrix._21, m_xmf3Up.y = tempMatrix._22, m_xmf3Up.z = tempMatrix._23;
	m_xmf3Look.x = tempMatrix._31, m_xmf3Look.y = tempMatrix._32, m_xmf3Look.z = tempMatrix._33;


	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;

	m_deltaX = 0.f;
	m_deltaY = 0.f;
}

void CPlayer::LaunchMissile()
{
	for (int i = 0; i < maxMissileNum; ++i)
	{
		if(!m_pMissiles[i]->IsActive())
		{
			m_pMissiles[i]->m_bActive = true;
			m_pMissiles[i]->SetPosition(m_fxPos, m_fyPos, m_fzPos);
			m_pMissiles[i]->m_fPitch = m_fPitch;
			m_pMissiles[i]->m_fYaw = m_fYaw;
			m_pMissiles[i]->m_fRoll = m_fRoll;
			m_pMissiles[i]->m_xmf3Look = m_xmf3Look;
			break;
		}
	}
}

void CPlayer::UpdateMissiles(float elapsedTime)
{
	for (auto& missile : m_pMissiles)
	{
		if (missile->IsActive())
		{
			missile->Move(elapsedTime);
			missile->m_fLifeSpan -= elapsedTime;
			//Check LifeSpan and Delete
			if (missile->m_fLifeSpan < 0.f)
			{
				missile->m_bActive = false;
				missile->m_fLifeSpan = missileLifeSpan;
			}
		}
	}
}

void CPlayer::Update(float elapsedTime, int connectedClients)
{
	RecalculateLook();
	RecalculateRight();
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);

	Rotate(m_deltaY, m_deltaX, 0.f);

	if (playerKey)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0.f, 0.f, 0.f);

		float distance = movingSpeed * elapsedTime;

		if (playerKey & option0)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, distance);
			playerKey &= ~option0;
		}
		if (playerKey & option1)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -distance);
			playerKey &= ~option1;
		}
		if (playerKey & option2)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -distance);
			playerKey &= ~option2;
		}
		if (playerKey & option3)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, distance);
			playerKey &= ~option3;
		}
		if (playerKey & option4)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -distance);
			playerKey &= ~option4;
		}
		if (playerKey & option5)
		{
			xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, distance);
			playerKey &= ~option5;
		}
		Move(xmf3Shift);

		MoveOOBB(m_xmf3Position);

		// Attack
		if (playerKey & option6)
		{
			if (connectedClients >= 1) LaunchMissile();			// if not alone in the server
			playerKey &= ~option6;
		}
		m_xmf4x4World._41 = m_xmf3Position.x;
		m_xmf4x4World._42 = m_xmf3Position.y;
		m_xmf4x4World._43 = m_xmf3Position.z;
	}

	UpdateMissiles(elapsedTime);
}

void CPlayer::Reset(int playerNum)
{
	m_xmf3Right = XMFLOAT3(0, 0, 0);
	m_xmf3Up = XMFLOAT3(0, 0, 0);
	m_xmf3Look = XMFLOAT3(0, 0, 0);

	m_fPitch = initialRot[playerNum].x;
	m_fYaw = initialRot[playerNum].y;
	m_fRoll = initialRot[playerNum].z;

	m_fOldxPos = 0.f;
	m_fOldyPos = 0.f;
	m_fOldzPos = 0.f;

	SetPosition(initialPos[playerNum].x, initialPos[playerNum].y, initialPos[playerNum].z);

	m_bActive = false;

	m_nHp = 100;
	m_deltaX = 0.f;
	m_deltaY = 0.f;
	playerKey = 0;

	for (auto& missile : m_pMissiles)
		missile->Reset();
}

void CMissileObject::Move(float elapsedTime)
{
	float distance = movingSpeed * elapsedTime;
	//Move(m_xmf3Look, distance);
	SetPosition(m_xmf4x4World._41 + m_xmf3Look.x * distance,
		m_xmf4x4World._42 + m_xmf3Look.y * distance,
		m_xmf4x4World._43 + m_xmf3Look.z * distance);
	MoveOOBB(m_xmf3Position);

}

void CMissileObject::Reset()
{
	m_xmf4x4World = Matrix4x4::Identity();
	m_xmf3MovingDirection = XMFLOAT3(0, 0, 0);
	SetPosition(0, 0, 0);
	m_xmf3Right = XMFLOAT3(0, 0, 0);
	m_xmf3Up = XMFLOAT3(0, 0, 0);
	m_xmf3Look = XMFLOAT3(0, 0, 0);

	m_fPitch = 0.f;
	m_fYaw = 0.f;
	m_fRoll = 0.f;

	m_fOldxPos = 0.f;
	m_fOldyPos = 0.f;
	m_fOldzPos = 0.f;

	m_bActive = false;
	shouldDeactivated = false;
	m_playerNumber = -1;

}