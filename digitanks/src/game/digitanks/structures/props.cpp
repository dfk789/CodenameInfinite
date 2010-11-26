#include "props.h"

#include <renderer/renderer.h>

#include "digitanksgame.h"

NETVAR_TABLE_BEGIN(CStaticProp);
	NETVAR_DEFINE(bool, m_bAdditive);
	NETVAR_DEFINE(bool, m_bDepthMask);
	NETVAR_DEFINE(bool, m_bBackCulling);
	NETVAR_DEFINE(bool, m_bSwap);
	NETVAR_DEFINE(Color, m_clrSwap);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CStaticProp);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bAdditive);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bDepthMask);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bBackCulling);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bSwap);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Color, m_clrSwap);
SAVEDATA_TABLE_END();

void CStaticProp::Precache()
{
	PrecacheModel(L"models/props/prop01.obj", true);
	PrecacheModel(L"models/props/prop02.obj", true);
	PrecacheModel(L"models/props/prop03.obj", true);
	PrecacheModel(L"models/props/prop04.obj", true);
}

void CStaticProp::Spawn()
{
	SetCollisionGroup(CG_PROP);

	m_bAdditive = false;
	m_bDepthMask = true;
	m_bBackCulling = true;

	m_bSwap = false;
}

void CStaticProp::ModifyContext(CRenderingContext* pContext)
{
//	if (m_bAdditive)
	//	pContext->SetBlend(BLEND_ADDITIVE);

	//pContext->SetDepthMask(m_bDepthMask);
	pContext->SetBackCulling(m_bBackCulling);

	if (m_bSwap)
		pContext->SetColorSwap(m_clrSwap);

	BaseClass::ModifyContext(pContext);
}