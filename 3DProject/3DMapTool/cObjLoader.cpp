#include "StdAfx.h"
#include "cObjLoader.h"
#include "cMtlTex.h"
#include "cGroup.h"

#include "cDeviceManager.h"
#include "cTextureManager.h"


#pragma warning( disable: 4996 )

cObjLoader::cObjLoader(void)
{
}


cObjLoader::~cObjLoader(void)
{
}

void cObjLoader::Load( 
	const char* szFullPath, 
	std::vector<cGroup*>& vecGroup, 
	D3DXMATRIXA16* mat /*= NULL*/ )
{
	for ( auto& iter : m_mapMtlTex )
	{
		SAFE_RELEASE(iter.second);
	}
	m_mapMtlTex.clear();

	std::vector<D3DXVECTOR3> vecV;
	std::vector<D3DXVECTOR2> vecVT;
	std::vector<D3DXVECTOR3> vecVN;
	std::vector<ST_PNT_VERTEX> vecVertex;
	std::string sMtlName;

	FILE* fp = NULL;

	fopen_s(&fp, szFullPath, "r");

	while(true)
	{
		if (feof(fp))
			break;

		char szBuf[1024] = {'\0', };
		fgets(szBuf, 1024, fp);
		
		if (strlen(szBuf) == 0)
			continue;

		//OutputDebugString(szBuf);
		if (szBuf[0] == '#')
		{
			continue;
		}
		else if (szBuf[0] == 'm')
		{
			char szMtlPath[1024];
			sscanf(szBuf, "%*s %s", szMtlPath);
			LoadMtlLib(szMtlPath);
		}
		else if (szBuf[0] == 'g')
		{
			if (vecVertex.empty() == false)
			{
				if(mat)
				{
					for (size_t i = 0; i < vecVertex.size(); ++i)
					{
						D3DXVec3TransformCoord(&vecVertex[i].p, &vecVertex[i].p, mat);
						D3DXVec3TransformNormal(&vecVertex[i].n, &vecVertex[i].n, mat);
					}
				}

				cGroup* pGroup = new cGroup;
				pGroup->SetMtlTex(m_mapMtlTex[sMtlName]);
				pGroup->SetVertex(vecVertex);
				vecGroup.push_back(pGroup);

				vecVertex.clear();
			}
		}
		else if (szBuf[0] == 'v')
		{
			if (szBuf[1] == 't')
			{
				float u, v;
				sscanf(szBuf, "%*s %f %f %*f", &u, &v);
				vecVT.push_back(D3DXVECTOR2(u, v));
			}
			else if (szBuf[1] == 'n')
			{
				float x, y, z;
				sscanf(szBuf, "%*s %f %f %f", &x, &y, &z);
				vecVN.push_back(D3DXVECTOR3(x, y, z));
			}
			else
			{
				float x, y, z;
				sscanf(szBuf, "%*s %f %f %f", &x, &y, &z);
				vecV.push_back(D3DXVECTOR3(x, y, z));
			}
		}
		else if (szBuf[0] == 'u')
		{
			char szMtlName[1024];
			sscanf(szBuf, "%*s %s", szMtlName);
			sMtlName = std::string(szMtlName);
		}
		else if (szBuf[0] == 'f')
		{
			int aIndex[3][3];
			sscanf(szBuf, "%*s %d/%d/%d %d/%d/%d %d/%d/%d",
				&aIndex[0][0], &aIndex[0][1], &aIndex[0][2], 
				&aIndex[1][0], &aIndex[1][1], &aIndex[1][2], 
				&aIndex[2][0], &aIndex[2][1], &aIndex[2][2]);
			for (int i = 0; i < 3; ++i)
			{
				ST_PNT_VERTEX v;
				v.p = vecV[aIndex[i][0] - 1];
				v.t = vecVT[aIndex[i][1] - 1];
				v.n = vecVN[aIndex[i][2] - 1];
				vecVertex.push_back(v);
			}
		}
	}


	for ( auto& iter : m_mapMtlTex )
	{
		SAFE_RELEASE( iter.second );
	}

	m_mapMtlTex.clear();

	fclose(fp);
}

LPD3DXMESH cObjLoader::Load( IN char* szFullPath, 
	OUT std::vector<cMtlTex*>& vecMtlTex,
	IN D3DXMATRIXA16* mat /*= NULL*/ )
{
	for ( auto& iter : m_mapMtlTex )
	{
		SAFE_RELEASE( iter.second );
	}
	
	m_mapMtlTex.clear();

	std::vector<D3DXVECTOR3>	vecV;
	std::vector<D3DXVECTOR2>	vecVT;
	std::vector<D3DXVECTOR3>	vecVN;
	std::vector<ST_PNT_VERTEX>	vecVertex;
	std::vector<DWORD>			vecAttrBuf;

	std::string sMtlName;

	FILE* fp = NULL;

	fopen_s(&fp, szFullPath, "r");

	while(true)
	{
		if (feof(fp))
			break;

		char szBuf[1024] = {'\0', };
		fgets(szBuf, 1024, fp);

		if (strlen(szBuf) == 0)
			continue;

		//OutputDebugString(szBuf);
		if (szBuf[0] == '#')
		{
			continue;
		}
		else if (szBuf[0] == 'm')
		{
			char szMtlPath[1024];
			sscanf(szBuf, "%*s %s", szMtlPath);
			LoadMtlLib(szMtlPath);
		}
		else if (szBuf[0] == 'g')
		{
// 			if (vecVertex.empty() == false)
// 			{
// 				if(mat)
// 				{
// 					for (size_t i = 0; i < vecVertex.size(); ++i)
// 					{
// 						D3DXVec3TransformCoord(&vecVertex[i].p, &vecVertex[i].p, mat);
// 						D3DXVec3TransformNormal(&vecVertex[i].n, &vecVertex[i].n, mat);
// 					}
// 				}
// 
// 				cGroup* pGroup = new cGroup;
// 				pGroup->SetMtlTex(m_mapMtlTex[sMtlName]);
// 				pGroup->SetVertex(vecVertex);
// 				vecGroup.push_back(pGroup);
// 
// 				vecVertex.clear();
// 			}
		}
		else if (szBuf[0] == 'v')
		{
			if (szBuf[1] == 't')
			{
				float u, v;
				sscanf(szBuf, "%*s %f %f %*f", &u, &v);
				vecVT.push_back(D3DXVECTOR2(u, v));
			}
			else if (szBuf[1] == 'n')
			{
				float x, y, z;
				sscanf(szBuf, "%*s %f %f %f", &x, &y, &z);
				vecVN.push_back(D3DXVECTOR3(x, y, z));
			}
			else
			{
				float x, y, z;
				sscanf(szBuf, "%*s %f %f %f", &x, &y, &z);
				vecV.push_back(D3DXVECTOR3(x, y, z));
			}
		}
		else if (szBuf[0] == 'u')
		{
			char szMtlName[1024];
			sscanf(szBuf, "%*s %s", szMtlName);
			sMtlName = std::string(szMtlName);
		}
		else if (szBuf[0] == 'f')
		{
			int aIndex[3][3];
			sscanf(szBuf, "%*s %d/%d/%d %d/%d/%d %d/%d/%d",
				&aIndex[0][0], &aIndex[0][1], &aIndex[0][2], 
				&aIndex[1][0], &aIndex[1][1], &aIndex[1][2], 
				&aIndex[2][0], &aIndex[2][1], &aIndex[2][2]);
			for (int i = 0; i < 3; ++i)
			{
				ST_PNT_VERTEX v;
				v.p = vecV[aIndex[i][0] - 1];
				v.t = vecVT[aIndex[i][1] - 1];
				v.n = vecVN[aIndex[i][2] - 1];
				vecVertex.push_back(v);

				vecAttrBuf.push_back(m_mapMtlTex[sMtlName]->GetAttrID());
			}
		}
	}
	
	vecMtlTex.resize(m_mapMtlTex.size());
	for each(auto& it in m_mapMtlTex)
	{
		vecMtlTex[it.second->GetAttrID()] = it.second;
	}

	LPD3DXMESH pMesh = NULL;
	D3DXCreateMeshFVF(vecAttrBuf.size(),
		vecVertex.size(),
		D3DXMESH_MANAGED,
		ST_PNT_VERTEX::FVF,
		g_pD3DDevice,
		&pMesh);

	ST_PNT_VERTEX* pV = NULL;
	pMesh->LockVertexBuffer(0, (LPVOID*)&pV);
	memcpy(pV, &vecVertex[0], sizeof(ST_PNT_VERTEX) * vecVertex.size());
	pMesh->UnlockVertexBuffer();

	WORD* pI = NULL;
	pMesh->LockIndexBuffer(0, (LPVOID*)&pI);
	for(size_t i = 0; i < vecVertex.size(); ++i)
	{
		pI[i] = i;
	}
	pMesh->UnlockIndexBuffer();

	DWORD* pA = NULL;
	pMesh->LockAttributeBuffer(0, &pA);
	memcpy(pA, &vecAttrBuf[0], sizeof(DWORD) * vecAttrBuf.size());
	pMesh->UnlockAttributeBuffer();

	std::vector<DWORD> vecAdjBuf(pMesh->GetNumFaces() * 3);
	pMesh->GenerateAdjacency(0.0f, &vecAdjBuf[0]);

	pMesh->OptimizeInplace(
		D3DXMESHOPT_ATTRSORT |
		D3DXMESHOPT_COMPACT |
		D3DXMESHOPT_VERTEXCACHE,
		&vecAdjBuf[0],
		0, 0, 0);

	m_mapMtlTex.clear();

// 
// 	for each(auto it in m_mapMtlTex)
// 	{
// 		SAFE_RELEASE(it.second);
// 	}

	//D3DXCreateMeshFVF()
	fclose(fp);

	return pMesh;
}

void cObjLoader::LoadMtlLib( char* szFullPath )
{
	DWORD dwAttrID = 0;

	FILE* fp = NULL;

	fopen_s(&fp, szFullPath, "r");

	std::string sMtlName;

	while(true)
	{
		char szBuf[1024] = {'\0', };
		fgets(szBuf, 1024, fp);

		if (feof(fp))
			break;

		if (strlen(szBuf) == 0)
			continue;

		//OutputDebugString(szBuf);
		if (szBuf[0] == '#')
		{
			continue;
		}
		else if (szBuf[0] == 'n')
		{
			char szMtlName[1024];
			sscanf(szBuf, "%*s %s", szMtlName);
			sMtlName = std::string(szMtlName);
			SAFE_RELEASE(m_mapMtlTex[sMtlName]);
			m_mapMtlTex[sMtlName] = new cMtlTex;
			m_mapMtlTex[sMtlName]->SetAttrID(dwAttrID++);
		}
		else if (szBuf[0] == 'K')
		{
			if (szBuf[1] == 'a')
			{
				float r, g, b;
				sscanf(szBuf, "%*s %f %f %f", &r, &g, &b);

				D3DMATERIAL9& stMtl = m_mapMtlTex[sMtlName]->GetMtl();
				stMtl.Ambient.r = r;
				stMtl.Ambient.g = g;
				stMtl.Ambient.b = b;
				stMtl.Ambient.a = 1.0f;
			}
			else if (szBuf[1] == 'd')
			{
				float r, g, b;
				sscanf(szBuf, "%*s %f %f %f", &r, &g, &b);

				D3DMATERIAL9& stMtl = m_mapMtlTex[sMtlName]->GetMtl();
				stMtl.Diffuse.r = r;
				stMtl.Diffuse.g = g;
				stMtl.Diffuse.b = b;
				stMtl.Diffuse.a = 1.0f;
			}
			else if (szBuf[1] == 's')
			{
				float r, g, b;
				sscanf(szBuf, "%*s %f %f %f", &r, &g, &b);

				D3DMATERIAL9& stMtl = m_mapMtlTex[sMtlName]->GetMtl();
				stMtl.Specular.r = r;
				stMtl.Specular.g = g;
				stMtl.Specular.b = b;
				stMtl.Specular.a = 1.0f;
			}
		}
		else if (szBuf[0] == 'm')
		{
			char szTexPath[1024];
			sscanf(szBuf, "%*s %s", szTexPath);
			
			LPDIRECT3DTEXTURE9 pTexture = NULL;
			pTexture = g_pTextureManager->GetTexture(szTexPath);
			
			m_mapMtlTex[sMtlName]->SetTexture(pTexture);
		}
	}

	fclose(fp);
}

#pragma warning( default: 4996 )