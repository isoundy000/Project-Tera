#include "StdAfx.h"
#include "cGroup.h"
#include "cMtlTex.h"
#include "cDeviceManager.h"

cGroup::cGroup(void)
	: m_pMtlTex(NULL)
{
}


cGroup::~cGroup(void)
{
}

void cGroup::Render()
{
	if (m_pMtlTex)
	{
		g_pD3DDevice->SetMaterial(&m_pMtlTex->GetMtl());
		g_pD3DDevice->SetTexture(0, m_pMtlTex->GetTexture());
	}
	
	g_pD3DDevice->SetFVF(ST_PNT_VERTEX::FVF);
	g_pD3DDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLELIST,
		m_vecVertex.size() / 3,
		&m_vecVertex[0],
		sizeof(ST_PNT_VERTEX));
}
