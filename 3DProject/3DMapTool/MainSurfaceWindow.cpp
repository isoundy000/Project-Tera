#include "stdafx.h"
#include "MainSurfaceWindow.h"

#include "TXMLReader.h"
#include "Console.h"
#include "resource.h"
#include "PickingTile.h"
#include "cDeviceManager.h"
#include "cGameObjectManager.h"
#include "ObjObject.h"
#include "LightObject.h"
#include "HierarchyWindow.h"
#include "cCamera.h"
#include "BoundingBox.h"
#include "tinyxml2.h"

#pragma warning( disable: 4996 )

namespace
{


enum ExtensionTable
{
	kX,
	kXML,
	kRaw8,
	kRaw16,
	kObj,
	kUnknown,
};

ExtensionTable AnalyzeExtension( 
	const std::string& filePath )
{
	int i = filePath.size( )-1; 
	while ( i >= 0 &&
		filePath[i] != '.' )
	{
		--i;
	}

	const std::string extension = 
		filePath.substr( i+1, filePath.size( ));

	if ( extension == "x" )
	{
		return ExtensionTable::kX;
	}
	else if ( extension == "obj" )
	{
		return ExtensionTable::kObj;
	}
	else if ( extension == "raw" )
	{
		return ExtensionTable::kObj;
	}
	else if ( extension == "xml" )
	{
		return ExtensionTable::kXML;
	}
	else
	{
		return ExtensionTable::kUnknown;
	}
}

}

MainSurfaceWindow::MainSurfaceWindow( ) :
	AbstractWindow(
		L"3DMapTool",
		WS_EX_ACCEPTFILES,
		WS_OVERLAPPEDWINDOW,
		nullptr,
		this->MakeWindowClass( ),
		GetSystemMetrics( SM_CXSCREEN )/2-(MainWindowWidth/2),
		GetSystemMetrics( SM_CYSCREEN )/2-(MainWindowHeight/2),
		MainWindowWidth,
		MainWindowHeight
	),
	m_dropQueryPath( new char[MAX_PATH] ),
	m_createCount( 0 )
{
}

MainSurfaceWindow::~MainSurfaceWindow( )
{
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

LRESULT MainSurfaceWindow::MessageProc( 
	HWND wndHandle, 
	UINT msg, 
	WPARAM wParam, 
	LPARAM lParam )
{
	switch ( msg )
	{
	case WM_CREATE:
		{
			int x, y;
			this->GetPosition( &x, &y );
			m_prevPos = { x, y };
		}
		break;

	case WM_DROPFILES:
		this->OnDropFile( reinterpret_cast<HDROP>( wParam ));
		break;

	case WM_CLOSE:
		if ( g_wasSomethingChanged )
		{
			if ( MessageBox( this->GetWindowHandle( ),
				L"변경된 내용이 있습니다. 저장하시겠습니까?",
				L"WARNING!",
				MB_YESNO ) == IDYES )
			{
				if ( !this->OnSaveAsClicked( ) )
				{
					return 0;
				}
			}
		}
		break;

	case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// 메뉴 선택을 구문 분석합니다.
			switch (wmId)
			{
			case IDM_ABOUT:
				DialogBox(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDD_ABOUTBOX), wndHandle, About);
				break;

			case ID_MENU_LOAD_LOADSCENE:
				this->OnLoadSceneClicked( );
				break;
				
			case ID_MENU_NEWSCENE:
				this->OnNewSceneClicked( );
				break;

			case ID_MENU_SAVEAS:
				this->OnSaveAsClicked( );
				break;
			case ID_MENU_EXIT:
				DestroyWindow( wndHandle );
				break;
			}
		}
		break;

	case WM_MOVE:
		{
			int x, y;
			this->GetPosition( &x, &y );
			
			POINT currPos{ x, y };
			
			for ( auto& childElem : GetChildRepo( ) )
			{
				childElem->Move(
					( currPos.x-m_prevPos.x ),
					( currPos.y-m_prevPos.y )
				);
			}

			m_prevPos = currPos;
		}
		break;
	}

	return DefWindowProc( wndHandle, msg, wParam, lParam );
}

void MainSurfaceWindow::OnIdle( )
{
	g_pD3DDevice->SetRenderState(
		D3DRENDERSTATETYPE::D3DRS_LIGHTING,
		TRUE
	);

	cGameObjectManager::Get( )->Update( );
	cGameObjectManager::Get( )->Render( );
}

void MainSurfaceWindow::OnDropFile( 
	HDROP dropHandle )
{
	DragQueryFileA( dropHandle, 0,
		&m_dropQueryPath[0], MAX_PATH );

	ExtensionTable extension = AnalyzeExtension(
		m_dropQueryPath.get( ) );
	cGameObject* newObject = nullptr;

	switch ( extension )
	{
	case ExtensionTable::kX:
		break;

	case ExtensionTable::kXML:
		this->OnLoadSceneClicked( m_dropQueryPath.get( ));
		break;

	case ExtensionTable::kObj:
		{
			std::wstring str = L"object_";
			str += std::to_wstring( m_createCount++ );

			newObject = new ObjObject( str.c_str( ),
				m_dropQueryPath.get( ));
			g_hierarchyWnd->AddListItem( str );
		}
		break;

	case ExtensionTable::kUnknown:
		MessageBox( this->GetWindowHandle( ), 
			L"Unknown file extension.", 
			L"WARNING",
			MB_OK | MB_ICONEXCLAMATION 
		);
		break;
	}

	// Set position As mouse
	PickingTile* pickTile = static_cast<PickingTile*>(
		cGameObjectManager::Get( )->FindObject( L"PickingTile" ));
	D3DXVECTOR3 pickPos = CalculatePickPos( pickTile );

	if ( newObject )
	{
		g_wasSomethingChanged = true;
		newObject->SetPosition( pickPos );
		cGameObjectManager::Get( )->AddObject( newObject );
	}
}

bool MainSurfaceWindow::OnSaveAsClicked( )
{
	OPENFILENAMEA ofn{ 0 };
	char openFileName[MAX_PATH]{ 0 };

	ofn.lStructSize = sizeof( OPENFILENAME );
	ofn.hwndOwner = this->GetWindowHandle( );
	ofn.lpstrFilter = "(*.xml)\0*.*\0";
	ofn.lpstrFile = openFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = "xml";

	if ( GetSaveFileNameA( &ofn ) == 0 )
	{
		return false;
	}

	std::wofstream ofs( openFileName );
	ofs << "<?xml version=""\"1.0""\" encoding=""\"UTF-8""\"?>\n";
	
	for ( auto iter = cGameObjectManager::Get( )->begin( );
		iter != cGameObjectManager::Get( )->end( );
		++iter )
	{
		if ( iter->second->GetIdenfier( ) == ObjectIdenfier::kCamera ||
			iter->second->GetIdenfier( ) == ObjectIdenfier::kLight ||
			iter->second->GetIdenfier( ) == ObjectIdenfier::kUnknown ||
			iter->second->GetIdenfier( ) == ObjectIdenfier::kPickTile )
		{
			continue;
		}

		char buf[256];
		std::wcstombs( buf, 
			iter->second->GetName( ).c_str(), 256 );

		// Name
		ofs << "<";
		ofs << buf;
		ofs << ">\n";

		// ObjectName
		ofs << "	<ObjectName>";
		ofs << buf;
		ofs << "</ObjectName>\n";

		// Type
		switch ( iter->second->GetIdenfier( ))
		{
		case ObjectIdenfier::kObj:
			ofs << "	<Type>obj</Type>\n";
			break;

		case ObjectIdenfier::kX:
			ofs << "	<Type>x</Type>\n";
			break;

		case ObjectIdenfier::kRaw:
			ofs << "	<Type>raw</Type>\n";
			break;
		}

		// Model Path
		if ( iter->second->GetIdenfier( ) == ObjectIdenfier::kObj )
		{
			auto* objObject = static_cast<ObjObject*>( iter->second );

			sprintf_s( buf,
				"	<ModelPath>%s</ModelPath>\n",
				objObject->GetModelPath( ).c_str( )
			);
			ofs << buf;
		}

		// Position
		sprintf_s( buf, 
			"	<Position x=""\"%f\" y=""\"%f\" z=""\"%f\"></Position>\n",
			iter->second->GetPosition( ).x,
			iter->second->GetPosition( ).y,
			iter->second->GetPosition( ).z
		);
		ofs << buf;

		// Rotation
		sprintf_s( buf,
			"	<Rotation x=""\"%f\" y=""\"%f\" z=""\"%f\"></Rotation>\n",
			iter->second->GetAngle( ).x,
			iter->second->GetAngle( ).y,
			iter->second->GetAngle( ).z
		);
		ofs << buf;

		// Scale
		sprintf_s( buf,
			"	<Scale x=""\"%f\" y=""\"%f\" z=""\"%f\"></Scale>\n",
			iter->second->GetScale( ).x,
			iter->second->GetScale( ).y,
			iter->second->GetScale( ).z
		);
		ofs << buf;

		// Bounding Box
		if ( iter->second->GetIdenfier( ) == ObjectIdenfier::kObj )
		{
			auto* objObject = static_cast<ObjObject*>( iter->second );
			auto* boundingBox = static_cast<BoundingBox*>( 
				objObject->GetCollider( ));
			
			sprintf_s( buf,
				"	<BoundingBox\n\
		minX=""\"%f\" minY=""\"%f\" minZ=""\"%f\"\n\
		maxX=""\"%f\" maxY=""\"%f\" maxZ=""\"%f\">\n\
	</BoundingBox>\n",
				boundingBox->GetMin( ).x,
				boundingBox->GetMin( ).y,
				boundingBox->GetMin( ).z,
				boundingBox->GetMax( ).x,
				boundingBox->GetMax( ).y,
				boundingBox->GetMax( ).z
			);
			ofs << buf;
		}

		ofs << "	<End></End>\n";

		std::wcstombs( buf,
			iter->second->GetName( ).c_str( ), 256 );
		ofs << "</";
		ofs << buf;
		ofs << ">\n";
	}

	ofs.close( );
	g_wasSomethingChanged = false;

	return true;
}

void MainSurfaceWindow::OnLoadSceneClicked(
	const char* loadPath )
{
	if ( g_wasSomethingChanged )
	{
		if ( MessageBox( this->GetWindowHandle( ),
				L"변경된 내용이 있습니다. 저장하시겠습니까?",
				L"WARNING!",
				MB_YESNO ) == IDYES )
		{
			// If Saving ㄹcanceled, escape this function
			if ( !this->OnSaveAsClicked( ))
			{
				return;
			}
		}
	}

	OPENFILENAMEA ofn{ 0 };
	char openFileName[MAX_PATH] {0};
	if ( !loadPath )
	{
		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = this->GetWindowHandle( );
		ofn.lpstrFilter = "모든 파일(*.*)\0*.*\0";
		ofn.lpstrFile = openFileName;
		ofn.nMaxFile = MAX_PATH;

		if ( GetOpenFileNameA( &ofn ) == 0 )
		{
			return;
		}
	}

	// Delete all of object
	this->OnNewSceneClicked( );

	// Begin reading XML
	std::unique_ptr<tgon::TXMLReader> xmlReader;
	if ( loadPath )
	{
		xmlReader.reset( new tgon::TXMLReader( loadPath ));
	}
	else
	{
		xmlReader.reset( new tgon::TXMLReader( openFileName ));
	}
	
	if ( xmlReader->fail( ))
	{
		MessageBox( GetFocus( ),
			L"XML 로딩에 실패했습니다.",
			L"WARNING!",
			MB_OK | MB_ICONEXCLAMATION
		);
	}
		
	cGameObject* newObject = nullptr;
	std::wstring objName;
	D3DXVECTOR3 pos{0,0,0}, rot{0,0,0}, scale{0,0,0};
	ICollider* collider = nullptr;
	ObjectIdenfier objID = ObjectIdenfier::kUnknown;
	std::string modelPath;

	for ( const auto& xmlNodeElem : *xmlReader )
	{
		if ( !strcmp( "End", xmlNodeElem->Value( ) ) )
		{
			switch ( objID )
			{
			case ObjectIdenfier::kObj:
				++m_createCount;
				newObject = new ObjObject( objName, modelPath, collider );
				break;
			case ObjectIdenfier::kX:
				break;
			}
				
			newObject->SetPosition( pos );
			newObject->SetAngle( rot );
			newObject->SetScale( scale );

			cGameObjectManager::Get( )->AddObject( 
				newObject );
			g_hierarchyWnd->AddListItem( newObject->GetName( ));

			// Reset Items
			newObject = nullptr;
			collider = nullptr;
		}
		else if ( !strcmp( "ObjectName", xmlNodeElem->Value( )))
		{
			const char* objCharName = xmlNodeElem->FirstChild( )->Value( );
			wchar_t objWcharName[256] {0};
			std::mbstowcs( objWcharName, objCharName, 256 );

			objName = objWcharName;
		}
		else if ( !strcmp( "Type", xmlNodeElem->Value( )))
		{
			if ( !strcmp( xmlNodeElem->FirstChild( )->Value(), 
				"obj" ))
			{
				objID = ObjectIdenfier::kObj;
			}
			else if ( !strcmp( xmlNodeElem->FirstChild( )->Value(),
				"x" ))
			{
				objID = ObjectIdenfier::kX;
			}
		}
		else if ( !strcmp( "ModelPath", xmlNodeElem->Value( ) ) )
		{
			modelPath = xmlNodeElem->FirstChild( )
				->Value( );
		}
		else if ( !strcmp( "BoundingBox", xmlNodeElem->Value( )))
		{
			tinyxml2::XMLElement* elem = xmlNodeElem->ToElement( );
			if ( elem )
			{
				const tinyxml2::XMLAttribute* minXAttr =
					elem->FirstAttribute( );
				const tinyxml2::XMLAttribute* minYAttr =
					minXAttr->Next( );
				const tinyxml2::XMLAttribute* minZAttr =
					minYAttr->Next( );

				const tinyxml2::XMLAttribute* maxXAttr =
					minZAttr->Next( );
				const tinyxml2::XMLAttribute* maxYAttr =
					maxXAttr->Next( );
				const tinyxml2::XMLAttribute* maxZAttr =
					maxYAttr->Next( );

				collider = new BoundingBox( 
					{ minXAttr->FloatValue( ), 
					  minYAttr->FloatValue( ), 
					  minZAttr->FloatValue( )}, 
					{ maxXAttr->FloatValue( ), 
					  maxYAttr->FloatValue( ), 
					  maxZAttr->FloatValue( )});
			}
		}
		else if ( !strcmp( "Position", xmlNodeElem->Value( )))
		{
			tinyxml2::XMLElement* elem = xmlNodeElem->ToElement( );
			if ( elem )
			{
				const tinyxml2::XMLAttribute* xAttr = 
					elem->FirstAttribute( );
				const tinyxml2::XMLAttribute* yAttr = 
					xAttr->Next( );
				const tinyxml2::XMLAttribute* zAttr = 
					yAttr->Next( );

				pos = { xAttr->FloatValue( ),
					yAttr->FloatValue( ),
					zAttr->FloatValue( )
				};
			}
			else
			{
				MessageBox( GetFocus( ),
					L"알 수 없는 Element 형식입니다.",
					L"WARNING!",
					MB_OK | MB_ICONEXCLAMATION
				);
			}
		}
		else if ( !strcmp( "Rotation", xmlNodeElem->Value( ) ) )
		{
			tinyxml2::XMLElement* elem = xmlNodeElem->ToElement( );
			if ( elem )
			{
				const tinyxml2::XMLAttribute* xAttr = 
					elem->FirstAttribute( );
				const tinyxml2::XMLAttribute* yAttr = 
					xAttr->Next( );
				const tinyxml2::XMLAttribute* zAttr = 
					yAttr->Next( );

				rot = { xAttr->FloatValue( ),
					yAttr->FloatValue( ),
					zAttr->FloatValue( )
				};
			}
			else
			{
				MessageBox( GetFocus( ),
					L"알 수 없는 Element 형식입니다.",
					L"WARNING!",
					MB_OK | MB_ICONEXCLAMATION
				);
			}
		}
		else if ( !strcmp( "Scale", xmlNodeElem->Value( )))
		{
			tinyxml2::XMLElement* elem = xmlNodeElem->ToElement( );
			if ( elem )
			{
				const tinyxml2::XMLAttribute* xAttr =
					elem->FirstAttribute( );
				const tinyxml2::XMLAttribute* yAttr =
					xAttr->Next( );
				const tinyxml2::XMLAttribute* zAttr =
					yAttr->Next( );

				scale = { xAttr->FloatValue( ),
					yAttr->FloatValue( ),
					zAttr->FloatValue( )
				};
			}
			else
			{
				MessageBox( GetFocus( ),
					L"알 수 없는 Element 형식입니다.",
					L"WARNING!",
					MB_OK | MB_ICONEXCLAMATION
				);
			}
		}

		Log( xmlNodeElem->Value( ), L"\n" );
	}

	g_wasSomethingChanged = false;
}

void MainSurfaceWindow::OnNewSceneClicked( )
{
	if ( g_wasSomethingChanged )
	{
		if ( MessageBox( this->GetWindowHandle( ),
			L"변경된 내용이 있습니다. 저장하시겠습니까?",
			L"WARNING!",
			MB_YESNO ) == IDYES )
		{
			// If Saving canceled, escape this function
			if ( !this->OnSaveAsClicked( ))
			{
				Log( L"Saving canceled, Escape OnNewSceneClicked" );
				return;
			}
		}
	}
	g_wasSomethingChanged = false;

	m_createCount = 0;
	g_hierarchyWnd->ResetListItem( );
	cGameObjectManager::Get( )->ResetAllObject();
}

void MainSurfaceWindow::OnMove( 
	AbstractWindow* sender, 
	int x, 
	int y )
{
	sender->Move( x, y );
}

void MainSurfaceWindow::OnSize( 
	AbstractWindow* sender, 
	int width, 
	int height )
{
}

WNDCLASSEXW MainSurfaceWindow::MakeWindowClass( )
{
	WNDCLASSEXW wcex {0};
	HINSTANCE instanceHandle = GetModuleHandle( nullptr );

	wcex.cbSize = sizeof( WNDCLASSEX );
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = nullptr; // Ignore, using member function
	wcex.hInstance = instanceHandle;
	wcex.hIcon = LoadIcon( instanceHandle, MAKEINTRESOURCE( IDI_MY3DMAPTOOL ));
	wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
	wcex.hbrBackground = (HBRUSH)( COLOR_WINDOW+1 );
	wcex.lpszMenuName = MAKEINTRESOURCEW( IDC_MY3DMAPTOOL );
	wcex.lpszClassName = L"MainSurfaceWindowClass";
	wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_SMALL ));

	return wcex;
}