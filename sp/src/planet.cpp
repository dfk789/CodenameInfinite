#include "planet.h"

#include <tengine/renderer/renderer.h>

REGISTER_ENTITY(CPlanet);

NETVAR_TABLE_BEGIN(CPlanet);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CPlanet);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flRadius);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CPlanet);
INPUTS_TABLE_END();

void CPlanet::Precache()
{
	PrecacheModel("models/planet.obj", true);
}

void CPlanet::Spawn()
{
	SetModel("models/planet.obj");
}

void CPlanet::Think()
{
	BaseClass::Think();

	// Revolve once every hour
	SetAngles(GetAngles() + EAngle(0, 360, 0)*GameServer()->GetFrameTime()/60/60);
}

void CPlanet::ModifyContext(CRenderingContext* pContext, bool bTransparent) const
{
	pContext->Scale(m_flRadius, m_flRadius, m_flRadius);
}

void CPlanet::PostRender(bool bTransparent) const
{
	BaseClass::PostRender(bTransparent);
}
