#pragma once
#include "cGameObject.h"

class cLightObject;
class TestScenePlane
{
public:
	explicit TestScenePlane( const char* objName );
	virtual ~TestScenePlane( );

	void Render( );
	void Update( );

private:
	cGameObject* m_owner;
	cLightObject* m_lightObject;
	LPD3DXEFFECT m_normalMappingShader;
	LPDIRECT3DTEXTURE9 m_diffuseMap;
	LPDIRECT3DTEXTURE9 m_specularMap;
	LPDIRECT3DTEXTURE9 m_normalMap;
};
