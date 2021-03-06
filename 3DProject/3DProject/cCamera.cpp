#include "StdAfx.h"
#include "cCamera.h"


cCamera::cCamera(void)
	: m_vEye(0, 0, -m_fDist )
	, m_vOldEye(0, 0, -m_fDist )
	, m_vUp(0, 1, 0)
	, m_vLookAt(0, 0, 0)
	, m_isLButtonDown(false)
	, m_fRotX(0.0f)
	, m_fRotY(0.0f)
	, m_fDist(100)
	, m_followTarget( nullptr )
	, m_cameraHeight( 20.f )
{
	this->SetupProjection( D3DX_PI/4.0f, 1.f, 3000.f );
}

cCamera::~cCamera(void)
{
}

void cCamera::SetupProjection( 
	float fovy,
	float nearZ,
	float farZ )
{
	assert( g_hWnd );

	RECT rc;
	GetClientRect( g_hWnd, &rc );

	D3DXMatrixIdentity( &m_matProjection );
	D3DXMatrixPerspectiveFovLH( &m_matProjection, fovy, rc.right/( float )rc.bottom, nearZ, farZ );
	g_pD3DDevice->SetTransform( D3DTS_PROJECTION, &m_matProjection );

	m_matViewProjection = m_matView;
	m_matViewProjection *= m_matProjection;
}

void cCamera::SetupView( 
	const D3DXVECTOR3& eye, 
	const D3DXVECTOR3& lookAt )
{
	D3DXMatrixIdentity( &m_matView );
	D3DXMatrixLookAtLH( &m_matView, &eye, &lookAt, &m_vUp );
	g_pD3DDevice->SetTransform( D3DTS_VIEW, &m_matView );

	m_matViewProjection = m_matView;
	m_matViewProjection *= m_matProjection;
}

void cCamera::Update( )
{
	m_vOldEye = D3DXVECTOR3(0, 0, -m_fDist);
	D3DXMATRIXA16 matRotX, matRotY;
	D3DXMatrixRotationX(&matRotX, m_fRotX);
	D3DXMatrixRotationY(&matRotY, m_fRotY);
	D3DXMATRIXA16 matRot = matRotX * matRotY;
	D3DXVec3TransformCoord(&m_vOldEye, &m_vOldEye, &matRot);
	
	if ( m_followTarget )
	{
		m_vOldEye = m_vOldEye + m_followTarget->GetPosition( );
		m_vLookAt = m_followTarget->GetPosition( );
	}

	D3DXVec3Lerp( &m_vEye, &m_vEye, &m_vOldEye, g_pTimeManager->Get( )->GetDeltaTime( )*2 );

	this->SetupView( m_vEye, m_vLookAt );
}

void cCamera::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_RBUTTONDOWN:
		SetCapture( hWnd );
		m_isLButtonDown = true;
		m_ptPrevMouse.x = LOWORD(lParam);
		m_ptPrevMouse.y = HIWORD(lParam);
		break;
	case WM_RBUTTONUP:
		ReleaseCapture( );
		m_isLButtonDown = false;
		break;
	case WM_MOUSEMOVE:
	{
		if (m_isLButtonDown)
		{
			POINT ptCurrMouse;
			ptCurrMouse.x = LOWORD(lParam);
			ptCurrMouse.y = HIWORD(lParam);
			m_fRotX += (ptCurrMouse.y - m_ptPrevMouse.y) / 100.0f;
			m_fRotY += (ptCurrMouse.x - m_ptPrevMouse.x) / 100.0f;
			m_ptPrevMouse = ptCurrMouse;
			if (m_fRotX >= D3DX_PI / 2.0f - EPSILON)
			{
				m_fRotX = D3DX_PI / 2.0f - EPSILON;
			}
			if (m_fRotX <= -D3DX_PI / 2.0f + EPSILON)
			{
				m_fRotX = -D3DX_PI / 2.0f + EPSILON;
			}
		}
	}
	break;
	case WM_MOUSEWHEEL:
		m_fDist -= GET_WHEEL_DELTA_WPARAM(wParam) / 10;
		break;
	}
}