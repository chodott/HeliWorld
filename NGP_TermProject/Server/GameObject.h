#pragma once

#include "stdafx.h"
class GameObject {
public:

	XMFLOAT4X4 m_xmf4x4World;
	BoundingOrientedBox m_xmOOBB;
	XMFLOAT3 m_xmf3MovingDirection;
	XMFLOAT3 m_xmf3RotationAxis;

	bool m_bActive;//active
	float m_fSpeed;//speed
	int m_nObjects{};


	void Move();
	void Rotate(float Pitch, float Yaw, float Roll);
	void SetPosition(float x, float y, float z);
	void SetActive(bool active);
	bool GetActive();
	void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation)
	{
		m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation);
	}
	BoundingOrientedBox GetBoundingBox() { return m_xmOOBB; }

};

class CMissileObject :public GameObject
{
	char m_cPlayerNumber;
};
class CPlayer : public GameObject
{
public:
	float m_fFriction;
	int m_nHp;
	CMissileObject m_pMissiles[8];

	void Move(DWORD Direction, float Distance, bool updateVelocity);
};

class Item : public GameObject
{

};