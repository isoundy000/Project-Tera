#include "stdafx.h"
#include "AbstractWindow.h"

#include "resource.h"

#include "cCamera.h"
#include "cTextureManager.h"
#include "cDeviceManager.h"


AbstractWindow::AbstractWindow( 
	const wchar_t* wndName ) :
	m_wndName( wndName ),
	m_wndClassName( std::wstring(wndName)+L"Class" )
{
}

AbstractWindow::~AbstractWindow( )
{
}

void AbstractWindow::Setup( )
{
	m_myWndHandle = this->SetupWindowComponents( );
	
	
	// Make managers
	cDeviceManager::Get( );
	cTextureManager::Get( );
	cCamera::Get( );

}

AbstractWindow * AbstractWindow::GetOwner( )
{
	return m_owner;
}

void AbstractWindow::GetSize( int * width, int * height )
{
	RECT rt;
	GetClientRect( m_myWndHandle, &rt );

	*width = rt.right;
	*height = rt.bottom;
}

void AbstractWindow::GetPosition( int * x, int * y )
{
	RECT rt;
	GetWindowRect( m_myWndHandle, &rt );

	*x = rt.left;
	*y = rt.top;
}

std::vector<AbstractWindow*>& AbstractWindow::GetChildRepo( )
{
	return m_childRepo;
}