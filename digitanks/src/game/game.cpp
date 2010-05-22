#include "game.h"

#include <ui/digitankswindow.h>
#include <renderer/renderer.h>

#include "camera.h"

CGame* CGame::s_pGame = NULL;

CGame::CGame()
{
	assert(!s_pGame);
	s_pGame = this;

	m_flGameTime = 0;
	m_flFrameTime = 0;

	for (size_t i = 0; i < CBaseEntity::s_apfnEntityRegisterCallbacks.size(); i++)
		CBaseEntity::s_apfnEntityRegisterCallbacks[i]();

	m_pCamera = new CCamera();
	m_pCamera->SnapDistance(120);
	m_pRenderer = new CRenderer(CDigitanksWindow::Get()->GetWindowWidth(), CDigitanksWindow::Get()->GetWindowHeight());
}

CGame::~CGame()
{
	delete m_pRenderer;
	delete m_pCamera;
	assert(s_pGame == this);
	s_pGame = NULL;
}

void CGame::Think(float flGameTime)
{
	m_flFrameTime = flGameTime - m_flGameTime;
	m_flGameTime = flGameTime;

	// Erase anything deleted last frame.
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		delete m_ahDeletedEntities[i];

	m_ahDeletedEntities.clear();

	Simulate();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		pEntity->Think();
	}

	Think();
}

void CGame::Simulate()
{
	// Move all entities
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i));

		pEntity->SetLastOrigin(pEntity->GetOrigin());
		pEntity->SetOrigin(pEntity->GetOrigin() + pEntity->GetVelocity() * m_flFrameTime);
		pEntity->SetVelocity(pEntity->GetVelocity() + pEntity->GetGravity() * m_flFrameTime);
	}

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);

		if (pEntity->IsDeleted())
			continue;

		for (size_t j = 0; j < CBaseEntity::GetNumEntities(); j++)
		{
			CBaseEntity* pEntity2 = CBaseEntity::GetEntityNumber(j);

			if (pEntity2->IsDeleted())
				continue;

			if (!pEntity->ShouldTouch(pEntity2))
				continue;

			Vector vecPoint;
			if (pEntity->IsTouching(pEntity2, vecPoint))
			{
				pEntity->SetOrigin(vecPoint);
				pEntity->Touching(pEntity2);
			}
		}
	}
}

void CGame::Render()
{
	m_pCamera->Think();

	m_pRenderer->SetCameraPosition(m_pCamera->GetCameraPosition());
	m_pRenderer->SetCameraTarget(m_pCamera->GetCameraTarget());

	m_pRenderer->SetupFrame();
	m_pRenderer->DrawBackground();
	m_pRenderer->StartRendering();

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
		CBaseEntity::GetEntity(CBaseEntity::GetEntityHandle(i))->Render();

	m_pRenderer->FinishRendering();
}

void CGame::Delete(class CBaseEntity* pEntity)
{
	for (size_t i = 0; i < m_ahDeletedEntities.size(); i++)
		if (m_ahDeletedEntities[i] == pEntity)
			return;

	OnDeleted(pEntity);
	pEntity->SetDeleted();
	m_ahDeletedEntities.push_back(pEntity);
}
