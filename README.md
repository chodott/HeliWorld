# Intro
원래 싱글플레이어 게임을 목적으로 만든 게임에 소켓 프로그래밍을 붙이는 프로젝트
프로젝트 기간 동안 클라이언트가 지속해서 업데이트 되고 프로젝트 기간도 짧은 상황이었기 때문에, 기존 설계를 뜯어 고치는 것보다는 네트워크 관련 정보를 담고 있는 클래스를 만들어 기존의 플레이어에게 포함 시키는 것이 더 낫다고 판단
- 인원: 3인
- 기간: 2022.11.11 - 2022.12.13 (약 1개월)
- 사용한 툴 or 라이브러리: Visual Studio, WSA(Windows Sockets API)

# Table of Contents
[1. Pipeline](#pipeline) <br />
[2. Classes](#classes) <br />
[3. Server Functions](#server-side-functions) <br />
[4. Client Functions](#client-side-functions) <br />

# Pipeline


# Classes
### 서버
- Singleton 패턴
- 게임 오브젝트 관리
- 다음 업데이트 함수 호출 시 제거해야 할 오브젝트들의 배열

### 클라이언트
- 서버와 데이터 통신을 위한 클래스
- 게임 내의 모든 오브젝트들의 정보를 담을 배열



<details><summary>패킷 설계</summary>
	
```C++
#pragma pack(1)
struct PlayerInfoPacket
{
	char packetType;
	int playerNumber = -1;
	int playerHP;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	bool playerActive;
};

struct PlayerKeyPacket
{
	unsigned char playerKeyInput;
	FPoint deltaMouse;
};

...

#pragma pack()
```
- pragma pack매크로를 사용하여 패킷 구조체의 정보들 사이에 패딩이 생기지 않도록 하였음
- 플레이어 키와 마우스의 움직임을 담은 패킷을 교환하여 각 플레이어의 움직임과 회전이 되도록 시도하였음
> **이슈 - 지속적인 오차의 발생**
마우스의 움직임과 키 입력만 가지고 각 플레이어들의 위치와 회전값을 계속해서 갱신할 경우 오차가 쌓여 1분도 안되는 시간에 실제와 반대 방향을 보는 것처럼 되는 현상이 발생했음

</details>

# Server Side Functions

<details><summary>각 클라이언트로 데이터 전송</summary>

```C++
void Server::SendAllClient()
{
	for (int i = 0; i < MAX_CLIENT_NUM; ++i)		// client number
	{
		if (!clients[i]->IsConnected())
		{
			continue;
		}

		PlayerInfoPacket scInfo;

		Client* client = clients[i];
		CPlayer* player = client->m_player;
		int playerNumber = client->GetPlayerNumber();

		XMFLOAT3 position{ player->m_fxPos, player->m_fyPos, player->m_fzPos };

		// PlayerInfo packet
		scInfo.packetType = SC_PlayerInfo;
		scInfo.playerNumber = playerNumber;
		scInfo.playerHP = player->m_nHp;
		scInfo.position = position;
		scInfo.rotation = XMFLOAT3(player->m_fPitch, player->m_fYaw, player->m_fRoll);

		if ((scInfo.playerActive = !client->ShouldDisconnected()) == false)
		{
			client->Disconnect();			// disconnect
		}

		// let each client know the other clients information
		for (const auto& client : clients)
		{
			if (!client->IsConnected())
			{
				continue;
			}
			send(client->sock, (char*)&scInfo, sizeof(PlayerInfoPacket), 0);

			// sending MissileInfo packet
			for (int i = 0; i < player->maxMissileNum; ++i)
			{
				CMissileObject* missile = player->m_pMissiles[i];
				if (!missile->IsActive())
				{
					continue;
				}

				MissileInfoPacket scMissile;
				scMissile.packetType = SC_MissileInfo;
				scMissile.playerNumber = playerNumber;
				scMissile.missileNumber = i;
				if (missile->shouldDeactivated)
				{
					trashCan.push(missile);
					scMissile.active = false;
				}
				else
				{
					scMissile.active = true;
				}

				scMissile.position = XMFLOAT3(missile->m_fxPos, missile->m_fyPos, missile->m_fzPos);
				scMissile.rotation = XMFLOAT3(missile->m_fPitch, missile->m_fYaw, missile->m_fRoll);
				send(client->sock, (char*)&scMissile, sizeof(MissileInfoPacket), 0);
			}

			// sending ItemInfo packet
			for (int i = 0; i < MAX_ITEM_NUM; ++i)
			{
				CItemObject* item = m_ItemObject[i];
				if (!item->IsActive())
				{
					continue;
				}

				ItemInfoPacket scItem;
				scItem.packetType = SC_ItemInfo;
				if (item->shouldDeactivated)
				{
					trashCan.push(item);
					scItem.active = false;
				}
				else
				{
					scItem.active = true;
				}
				scItem.itemNum = i;
				scItem.position = item->GetCurPos();
				send(client->sock, (char*)&scItem, sizeof(ItemInfoPacket), 0);
			}
		}
	}
}

```
> 모든 클라이언트들에게 각 플레이어를 포함한 미사일, 아이템들의 정보를 패킷화하여 보내는 함수
</details>


<details><summary>클라이언트의 정보 수신</summary>

```C++
DWORD WINAPI ReceiveFromClient(LPVOID arg)
{
	Client* client = (Client*)arg;

	int playerNumber = client->GetPlayerNumber();
	send(client->sock, (char*)&playerNumber, sizeof(int), 0);

	CPlayer* p = client->m_player;
	p->SetActive(true);
	p->SetPosition(p->initialPos[playerNumber].x, p->initialPos[playerNumber].y, p->initialPos[playerNumber].z);
	p->m_fPitch = p->initialRot[playerNumber].x; p->m_fYaw = p->initialRot[playerNumber].y; p->m_fRoll = p->initialRot[playerNumber].z;

	client->Connected();
	++g_server->connectedClients;

	PlayerKeyPacket keyPacket;
	while (true)
	{
		WaitForSingleObject(g_server->updateDone, INFINITE);
		// check the client is still connected
		if (recv(client->sock, (char*)&keyPacket, sizeof(PlayerKeyPacket), 0) == SOCKET_ERROR)
		{
			// cut the connection
			client->Reset();
			--g_server->connectedClients;
			break;
		}
		CPlayer* player = client->m_player;
		player->playerKey = keyPacket.playerKeyInput;

		player->m_deltaX = keyPacket.deltaMouse.x;
		player->m_deltaY = keyPacket.deltaMouse.y;
	}

	return 0;
}
```
> 스레드 함수로서 동작하며, 플레이어 연결 시 위치 등의 정보를 초기화 하고 매 업데이트 루프마다 할당된 클라이언트의 입력 정보를 받아옴
</details>

<details><summary>동기화</summary>

```C++
void Server::Update()
{
	ResetEvent(updateDone);

	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		// is client dead?
		if (clients[i]->IsConnected() && !clients[i]->m_player->IsActive())
		{
			clients[i]->deadTime += elapsedTime;
			// respawn logic
			if (clients[i]->deadTime > RESPAWN_TIME)
			{
				clients[i]->m_player->SetActive(true);
				clients[i]->deadTime = 0.f;
			}
		}
		else
		{
			// if not dead, call update
			clients[i]->m_player->Update(elapsedTime, g_server->connectedClients);
		}
	}

	SetEvent(updateDone);

	CheckCollision();
	SendAllClient();
	
	...
}
```
> 플레이어들의 정보가 업데이트 되는 동안에 각 클라이언트로 정보가 송신되면 서로 엇갈린 정보를 받을 수도 있다고 판단하여 업데이트 되는 동안에는 updateDone 이벤트를 통해 다른 함수들과 동기화 시킴
</details>


# Client Side Functions
<details><summary>서버로부터 데이터 수신</summary>

```C++
DWORD WINAPI ReceiveFromServer(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();

	const int bufSize = 512;
	char buf[bufSize]{};
	while (true)
	{
		const int frameTime = 17;		// 1000ms / 60frame	+ 1
		WaitForSingleObject(client->FrameAdvanced, (DWORD)frameTime);

		if (recv(*sock, (char*)&buf, bufSize, MSG_WAITALL) == SOCKET_ERROR)		err_quit("recv()");

		int restBufSize = bufSize;
		int bufOffset = 0;

		// process remain packet
		if (client->remainSize > 0)
		{
			memcpy(&client->remain[client->remainOffset], buf, client->remainSize);
			restBufSize -= client->remainSize;
			PacketProcessHelper(client->remain[0], client->remain, client);
			bufOffset += client->remainSize;

			// reset remain
			memset(client->remain, 0, 512);
			client->remainOffset = 0;
			client->remainSize = 0;
		}

		while (restBufSize > 0)
		{
			char packetType = buf[bufOffset];
			int packetSize = PacketSizeHelper(packetType);

			// save remain packet
			if (restBufSize < packetSize)
			{
				client->remainOffset = restBufSize;
				client->remainSize = packetSize - client->remainOffset;
				memcpy(&client->remain, buf + bufOffset, restBufSize);
				restBufSize -= client->remainSize;
				break;
			}

			// Packet process
			PacketProcessHelper(packetType, buf + bufOffset, client);
			restBufSize -= packetSize;
			bufOffset += packetSize;
		}
	}
}
```
> 서버로부터 다른 개체들의 정보를 수신함
매 프레임 업데이트를 위해 FrameAdvanced 이벤트가 설정될 때까지 기다림
또한, 패킷이 분할 되어 넘치거나 부족할 경우  재조립 하는 작업이 포함되어 있음
</details>

<details><summary>서버로부터 데이터 수신</summary>

```C++
void CGameFramework::FrameAdvance()
{
	ResetEvent(client->FrameAdvanced);
	m_GameTimer.Tick(0.0f);

	ProcessInput();

	AnimateObjects();
	client->SendtoServer();

	SetEvent(client->FrameAdvanced);

	...
}
```
> Event(SetEvent / ResetEvent)를 사용해서 동기화
매 프레임 시에 호출되는 함수에서 서버에서 받은 정보를 통해 AnimateObjects, 서버로 보낼 정보를 SendtoServer 함수로 보냄
</details>


