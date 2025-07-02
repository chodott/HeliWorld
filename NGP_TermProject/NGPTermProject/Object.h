//------------------------------------------------------- ----------------------
// File: Object.h
//-----------------------------------------------------------------------------

#pragma once

#include "Mesh.h"
#include "Camera.h"
#include"CSPacket.h"


#define DIR_FORWARD               0x01
#define DIR_BACKWARD            0x02
#define DIR_LEFT               0x04
#define DIR_RIGHT               0x08
#define DIR_UP                  0x10
#define DIR_DOWN               0x20

#define MAP_SCALE 32.767

class CShader;
class CStandardShader;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define RESOURCE_TEXTURE2D         0x01
#define RESOURCE_TEXTURE2D_ARRAY   0x02   //[]
#define RESOURCE_TEXTURE2DARRAY      0x03
#define RESOURCE_TEXTURE_CUBE      0x04
#define RESOURCE_BUFFER            0x05

class CGameObject;
struct MATERIAL
{
    XMFLOAT4                  m_xmf4Ambient;
    XMFLOAT4                  m_xmf4Diffuse;
    XMFLOAT4                  m_xmf4Specular; //(r,g,b,a=power)
    XMFLOAT4                  m_xmf4Emissive;
};
struct SPRITEANIMATIONMATERIAL
{
    XMFLOAT4                  m_xmf4Ambient;
    XMFLOAT4                  m_xmf4Diffuse;
    XMFLOAT4                  m_xmf4Specular; //(r,g,b,a=power)
    XMFLOAT4                  m_xmf4Emissive;
};
struct CB_GAMEOBJECT_INFO
{
    XMFLOAT4X4                  m_xmf4x4World;
    MATERIAL                  m_material;

    XMFLOAT4X4                  m_xmf4x4Texture;
    XMINT2                     m_xmi2TextureTiling;
    XMFLOAT2                  m_xmf2TextureOffset;
    float                     g_fDeltaTime;
};

class CTexture
{
public:
    CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters);
    CTexture(int nTextureResources, UINT nResourceType, int nSamplers, int nRootParameters, int nRows, int nCols);
    virtual ~CTexture();

private:
    int                        m_nReferences = 0;

    UINT                     m_nTextureType;

    int                        m_nTextures = 0;
    _TCHAR(*m_ppstrTextureNames)[64] = NULL;
    ID3D12Resource** m_ppd3dTextures = NULL;
    ID3D12Resource** m_ppd3dTextureUploadBuffers;

    UINT* m_pnResourceTypes = NULL;

    DXGI_FORMAT* m_pdxgiBufferFormats = NULL;
    int* m_pnBufferElements = NULL;

    int                        m_nRootParameters = 0;
    int* m_pnRootParameterIndices = NULL;
    D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSrvGpuDescriptorHandles = NULL;

    int                        m_nSamplers = 0;
    D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;

    int                      m_nRow = 0;
    int                      m_nCol = 0;
public:
    int                      m_nRows = 1;
    int                      m_nCols = 1;

    XMFLOAT4X4                  m_xmf4x4Texture;

public:
    void AddRef() { m_nReferences++; }
    void Release() { if (--m_nReferences <= 0) delete this; }

    void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

    void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nParameterIndex, int nTextureIndex);
    void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    void ReleaseShaderVariables();

    void LoadTextureFromDDSFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
    void LoadBuffer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nElements, UINT nStride, DXGI_FORMAT ndxgiFormat, UINT nIndex);
    ID3D12Resource* CreateTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, UINT nIndex, UINT nResourceType, UINT nWidth, UINT nHeight, UINT nElements, UINT nMipLevels, DXGI_FORMAT dxgiFormat, D3D12_RESOURCE_FLAGS d3dResourceFlags, D3D12_RESOURCE_STATES d3dResourceStates, D3D12_CLEAR_VALUE* pd3dClearValue);

    int LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader, UINT nIndex);
    void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nResourceType, UINT nIndex);
    void SetRootParameterIndex(int nIndex, UINT nRootParameterIndex);
    void SetGpuDescriptorHandle(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSrvGpuDescriptorHandle);

    int GetRootParameters() { return(m_nRootParameters); }
    int GetTextures() { return(m_nTextures); }
    _TCHAR* GetTextureName(int nIndex) { return(m_ppstrTextureNames[nIndex]); }
    ID3D12Resource* GetResource(int nIndex) { return(m_ppd3dTextures[nIndex]); }
    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(int nIndex) { return(m_pd3dSrvGpuDescriptorHandles[nIndex]); }
    int GetRootParameter(int nIndex) { return(m_pnRootParameterIndices[nIndex]); }

    UINT GetTextureType() { return(m_nTextureType); }
    UINT GetTextureType(int nIndex) { return(m_pnResourceTypes[nIndex]); }
    DXGI_FORMAT GetBufferFormat(int nIndex) { return(m_pdxgiBufferFormats[nIndex]); }
    int GetBufferElements(int nIndex) { return(m_pnBufferElements[nIndex]); }

    D3D12_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(int nIndex);

    void ReleaseUploadBuffers();

    void AnimateRowColumn(float fTime = 0.0f);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define MATERIAL_ALBEDO_MAP         0x01
#define MATERIAL_SPECULAR_MAP      0x02
#define MATERIAL_NORMAL_MAP         0x04
#define MATERIAL_METALLIC_MAP      0x08
#define MATERIAL_EMISSION_MAP      0x10
#define MATERIAL_DETAIL_ALBEDO_MAP   0x20
#define MATERIAL_DETAIL_NORMAL_MAP   0x40

class CGameObject;

class CMaterial
{
public:
    CMaterial();
    virtual ~CMaterial();

private:
    int                        m_nReferences = 0;

public:
    void AddRef() { m_nReferences++; }
    void Release() { if (--m_nReferences <= 0) delete this; }

public:
    CShader* m_pShader = NULL;
    CTexture* m_pTexture = NULL;

    XMFLOAT4                  m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    XMFLOAT4                  m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4                  m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    XMFLOAT4                  m_xmf4AmbientColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

    void SetAlbedo(XMFLOAT4 xmf4Albedo) { m_xmf4AlbedoColor = xmf4Albedo; }
    void SetShader(CShader* pShader);
    void SetMaterialType(UINT nType) { m_nType |= nType; }
    void SetTexture(CTexture* pTexture);

    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();

    virtual void ReleaseUploadBuffers();


public:
    UINT                     m_nType = 0x00;

    float                     m_fGlossiness = 0.0f;//??
    float                     m_fSmoothness = 0.0f;//??
    float                     m_fSpecularHighlight = 0.0f;//??
    float                     m_fMetallic = 0.0f;//??
    float                     m_fGlossyReflection = 0.0f;//??
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGameObject
{
private:
    int                        m_nReferences = 0;
    bool                     b_Active = TRUE;
    float                        m_HP = 100;
public:
    void AddRef();
    void Release();

public:
    CGameObject();
    CGameObject(int nMaterials);
    CGameObject(int nMaterials, int meshes);
    virtual ~CGameObject();

    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, MissileInfoPacket& prevPacket, MissileInfoPacket& nextPacket, float value) {};
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, MissileInfoPacket* PlayerPacket) {};
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, ItemInfoPacket* ItemPacket) {};
    //void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerStatusPacket* PlayerStatus) {};

public:

    XMFLOAT4X4                  m_xmf4x4Transform;//��ȯ ���
    XMFLOAT4X4                  m_xmf4x4World; //���� ���


    char                     m_pstrFrameName[64];

    CMesh* m_pMesh = NULL;
    CMesh** m_ppMeshes;//->�׸���
    int                        m_nMeshes;//->�׸���

    int                        m_nMaterials = 0;
    CMaterial** m_ppMaterials = NULL;
    //CMaterial                  *m_pMaterial = NULL;


    D3D12_GPU_DESCRIPTOR_HANDLE      m_d3dCbvGPUDescriptorHandle;

    BoundingOrientedBox m_xmOOBB;
    BoundingOrientedBox m_xmOOBBTransformed;
protected:
    XMFLOAT3 m_movement = { 0,0,0 };
    XMFLOAT3 m_rotation = { 0,0,0 };

public:
    void SetShifts(const XMFLOAT3& position, const XMFLOAT3& rotation)
    {
        m_movement = position;
        m_rotation = rotation;
    }
    XMFLOAT3 GetMovement() { return m_movement; }
    XMFLOAT3 GetRotation() { return m_rotation; }
    virtual void RotatePYR(XMFLOAT3& xmf3RotationAxis);
    XMFLOAT3 XMVectorAngleLerp(XMFLOAT3& prevRotation, XMFLOAT3& nextRotation, float t);





protected:
    ID3D12Resource* m_pd3dcbGameObject = NULL;
    CB_GAMEOBJECT_INFO* m_pcbMappedGameObject = NULL;

    XMFLOAT3   m_xmf3MovingDirection;
    XMFLOAT3   m_xmf3RotationAxis;

    float            m_fMovingSpeed;
    float            m_fMovingRange;
    float            m_fRotationSpeed;

    XMFLOAT3   m_xmf3Right;
    XMFLOAT3   m_xmf3Up;
    XMFLOAT3   m_xmf3Look;



public:

    CGameObject* m_pParent = NULL;
    CGameObject* m_pChild = NULL;
    CGameObject* m_pSibling = NULL;

    void SetMesh(int nIndex, CMesh* pMesh);
    void SetMesh(CMesh* pMesh);
    void SetShader(CShader* pShader);
    void SetShader(int nMaterial, CShader* pShader);
    void SetMaterial(int nMaterial, CMaterial* pMaterial);


    void SetCbvGPUDescriptorHandle(D3D12_GPU_DESCRIPTOR_HANDLE d3dCbvGPUDescriptorHandle) { m_d3dCbvGPUDescriptorHandle = d3dCbvGPUDescriptorHandle; }
    void SetCbvGPUDescriptorHandlePtr(UINT64 nCbvGPUDescriptorHandlePtr) { m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return(m_d3dCbvGPUDescriptorHandle); }

    void SetChild(CGameObject* pChild);

    virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

    virtual void PrepareAnimate() { }
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerInfoPacket& prevPacket, PlayerInfoPacket& nextPacket, float value);
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerInfoPacket* PlayerPacket);
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
    //   virtual void Animate(float fTimeElapsed);
    virtual void AnimateObject(float fTimeElapsed);

    virtual void OnPrepareRender() { }
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();

    virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
    virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial* pMaterial);

    virtual void ReleaseUploadBuffers();

    XMFLOAT3 GetPosition();
    XMFLOAT3 GetLook();
    XMFLOAT3 GetUp();
    XMFLOAT3 GetRight();

    void SetLook(XMFLOAT3 value);
    void SetUp(XMFLOAT3 value);
    void SetRight(XMFLOAT3 value);
    virtual void SetPosition(float x, float y, float z);
    void SetPosition(XMFLOAT3 xmf3Position);
    void SetRotation(XMFLOAT3X3 xmf3Rotation);
    void SetScale(float x, float y, float z);
    void SetOOBB(XMFLOAT3& xmCenter, XMFLOAT3& xmExtents, XMFLOAT4& xmOrientation)
    {
        m_xmOOBBTransformed = m_xmOOBB = BoundingOrientedBox(xmCenter, xmExtents, xmOrientation);
    }
    void SetLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& xmf3Up);
    XMFLOAT3 GetMovingDirection()               const { return m_xmf3MovingDirection; }
    void SetMovingDirection(XMFLOAT3 value) { m_xmf3MovingDirection = value; }


    void MoveStrafe(float fDistance = 1.0f);
    void MoveUp(float fDistance = 1.0f);
    void MoveForward(float fDistance = 1.0f);

    virtual   void Move(ULONG nDirection, float fDistance, bool bVelocity = false);
    virtual void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
    virtual void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);
    //virtual void Rotate(float x, float y, float z);

    void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
    void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
    void Rotate(XMFLOAT4* pxmf4Quaternion);

    void SetBillboardLookAt(XMFLOAT3& xmf3Target, XMFLOAT3& xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f));

    CGameObject* GetParent() { return(m_pParent); }
    void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
    CGameObject* FindFrame(char* pstrFrameName);

    int FindReplicatedTexture(_TCHAR* pstrTextureName, D3D12_GPU_DESCRIPTOR_HANDLE* pd3dSrvGpuDescriptorHandle);

    UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0x00); }

    BoundingOrientedBox GetBoundingBox() { return m_xmOOBB; }

    bool GetActive() { return b_Active; };
    void SetActive(bool Active) { b_Active = Active; }
    void DeleteMesh();

    void SetHP(float HP) { m_HP = HP; };
    float GetHP() { return m_HP; };

public:
    void LoadMaterialsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameObject* pParent, FILE* pInFile, CShader* pShader);

    static CGameObject* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, CGameObject* pParent, FILE* pInFile, CShader* pShader);
    static CGameObject* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, char* pstrFileName, CShader* pShader);

    static void PrintFrameInfo(CGameObject* pGameObject, CGameObject* pParent);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSuperCobraObject : public CGameObject
{
public:
    CSuperCobraObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    virtual ~CSuperCobraObject();

private:
    CGameObject* m_pMainRotorFrame = NULL;
    CGameObject* m_pTailRotorFrame = NULL;

public:
    virtual void PrepareAnimate();
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CGunshipObject : public CGameObject
{
public:
    CGunshipObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    virtual ~CGunshipObject();

private:
    CGameObject* m_pMainRotorFrame = NULL;
    CGameObject* m_pTailRotorFrame = NULL;

public:
    virtual void PrepareAnimate();
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CMi24Object : public CGameObject
{
public:
    CMi24Object(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    virtual ~CMi24Object();

private:
    CGameObject* m_pMainRotorFrame = NULL;
    CGameObject* m_pTailRotorFrame = NULL;

public:
    //virtual void SetPosition(float x, float y, float z);
    virtual void PrepareAnimate();
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerInfoPacket* packet);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CSkyBox : public CGameObject
{
public:
    CSkyBox(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
    virtual ~CSkyBox();

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CPlayer* cPlayer, CCamera* pCamera);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CHeightMapTerrain : public CGameObject
{
public:
    CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color);
    CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, int num);
    virtual ~CHeightMapTerrain();

private:
    CHeightMapImage* m_pHeightMapImage;

    int                        m_nWidth;
    int                        m_nLength;

    XMFLOAT3                  m_xmf3Scale;

public:
    float GetHeight(float x, float z, bool bReverseQuad = false) { return(m_pHeightMapImage->GetHeight(x, z, bReverseQuad) * m_xmf3Scale.y); } //World
    XMFLOAT3 GetNormal(float x, float z) { return(m_pHeightMapImage->GetHeightMapNormal(int(x / m_xmf3Scale.x), int(z / m_xmf3Scale.z))); }

    int GetHeightMapWidth() { return(m_pHeightMapImage->GetHeightMapWidth()); }
    int GetHeightMapLength() { return(m_pHeightMapImage->GetHeightMapLength()); }

    XMFLOAT3 GetScale() { return(m_xmf3Scale); }
    float GetWidth() { return(m_nWidth * m_xmf3Scale.x); }
    float GetLength() { return(m_nLength * m_xmf3Scale.z); }

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CGrassObject : public CGameObject
{
public:
    CGrassObject();
    virtual ~CGrassObject();

    virtual void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    float m_fRotationAngle = 0.0f;
    float m_fRotationDelta = 1.0f;
};
class CUiObject : public CGameObject
{
public:
    CUiObject();
    virtual ~CUiObject();

    //virtual void Animate(float fTimeElapsed);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    //float m_fRotationAngle = 0.0f;
 //   float m_fRotationDelta = 1.0f;
};
class CHPObject : public CGameObject
{
public:

    //void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerStatusPacket* PlayerStatus);
    virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent, PlayerInfoPacket* PlayerStatus);

    CHPObject();
    virtual ~CHPObject();
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

};

class CBullet : public CGameObject
{
public:
    CBullet();
    virtual ~CBullet();

    virtual void Move(XMFLOAT3& vDirection, float fSpeed);
    //void Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle);

    virtual void Animate(float fElapsedTime);
    virtual void OnPrepareRender();
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

    void SetCollision(bool Collision) { m_Collision = Collision; }
    bool GetCollision()   const { return m_Collision; }

    void SetID(int id) { m_ID = id; }
    int GetID()   const { return m_ID; }


private:
    bool m_Collision = false;
    int m_ID;
};

