#include "stdafx.h"
#include "cGameObject.h"

#include "cObjectManager.h"
#include "ICollider.h"


cGameObject::cGameObject( const std::string& objName ) :
	m_objName( objName ),
	m_isActive( true ),
	m_pos( 0.f, 0.f, 0.f ),
	m_angle( 0.f, 0.f, 0.f ),
	m_scale( 1.f, 1.f, 1.f )
{
	D3DXMatrixIdentity( &m_matWorld );
	cObjectManager::Get()->AddObject( this );
}

cGameObject::~cGameObject( )
{
	cObjectManager::Get()->EraseObject( this->GetName());
}

void cGameObject::Update( )
{
	cFSM::Update( );
}

void cGameObject::Render( )
{
	cFSM::Render( );
}