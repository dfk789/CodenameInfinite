#include "structure.h"

#include <sstream>

#include <maths.h>
#include <mtrand.h>

#include "dt_renderer.h"
#include "digitanksgame.h"
#include "supplyline.h"

#include <ui/digitankswindow.h>
#include <ui/instructor.h>
#include <shaders/shaders.h>

#include "collector.h"
#include "loader.h"

#include <GL/glew.h>

CStructure::CStructure()
{
	m_bConstructing = false;
	m_iProductionToConstruct = 0;

	m_bInstalling = false;
	m_iProductionToInstall = 0;

	SetCollisionGroup(CG_ENTITY);
}

void CStructure::Spawn()
{
	BaseClass::Spawn();

	m_iFleetSupply = InitialFleetPoints();
	m_iBandwidth = InitialBandwidth();
	m_iPower = InitialPower();
	m_iEnergyBonus = InitialEnergyBonus();
	m_flRechargeBonus = InitialRechargeBonus();
}

void CStructure::StartTurn()
{
	BaseClass::StartTurn();

	FindGround();

	if (IsInstalling())
	{
		if (GetDigitanksTeam()->GetProductionPerLoader() >= GetProductionToInstall())
		{
			std::stringstream s;
			s << "'" << GetUpdateInstalling()->GetName() << "' finished installing on " << GetName();
			DigitanksGame()->AppendTurnInfo(s.str().c_str());

			InstallComplete();
		}
		else
		{
			AddProduction((size_t)GetDigitanksTeam()->GetProductionPerLoader());

			std::stringstream s;
			s << "Installing '" << GetUpdateInstalling()->GetName() << "' on " << GetName() << " (" << GetTurnsToInstall() << " turns left)";
			DigitanksGame()->AppendTurnInfo(s.str().c_str());
		}
	}
}

void CStructure::FindGround()
{
	float flHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x, GetOrigin().z);
	float flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x + GetBoundingRadius(), GetOrigin().z + GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x + GetBoundingRadius(), GetOrigin().z - GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x - GetBoundingRadius(), GetOrigin().z + GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	flCornerHeight = DigitanksGame()->GetTerrain()->GetHeight(GetOrigin().x - GetBoundingRadius(), GetOrigin().z - GetBoundingRadius());
	if (flCornerHeight > flHeight)
		flHeight = flCornerHeight;

	SetOrigin(Vector(GetOrigin().x, flHeight, GetOrigin().z));
}

void CStructure::BeginConstruction()
{
	m_iProductionToConstruct = ConstructionCost();
	m_bConstructing = true;

	FindGround();
}

void CStructure::CompleteConstruction()
{
	m_bConstructing = false;

	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWER);
	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_PSU);
	CDigitanksWindow::Get()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_LOADER);

	size_t iTutorial = CDigitanksWindow::Get()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_POWER)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

	GetDigitanksTeam()->SetCurrentSelection(this);

	if (DigitanksGame()->GetUpdateGrid())
	{
		for (size_t x = 0; x < UPDATE_GRID_SIZE; x++)
		{
			for (size_t y = 0; y < UPDATE_GRID_SIZE; y++)
			{
				if (GetDigitanksTeam()->HasDownloadedUpdate(x, y))
					DownloadComplete(&DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y]);
			}
		}
	}

	if (dynamic_cast<CCollector*>(this) && iTutorial == CInstructor::TUTORIAL_PSU)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();

	if (dynamic_cast<CLoader*>(this) && iTutorial == CInstructor::TUTORIAL_LOADER)
		CDigitanksWindow::Get()->GetInstructor()->NextTutorial();
}

size_t CStructure::GetTurnsToConstruct()
{
	return (size_t)(m_iProductionToConstruct/GetDigitanksTeam()->GetProductionPerLoader())+1;
}

void CStructure::AddProduction(size_t iProduction)
{
	if (IsConstructing())
	{
		if (iProduction > m_iProductionToConstruct)
			m_iProductionToConstruct = 0;
		else
			m_iProductionToConstruct -= iProduction;
	}
	else if (IsInstalling())
	{
		if (iProduction > m_iProductionToInstall)
			m_iProductionToInstall = 0;
		else
			m_iProductionToInstall -= iProduction;
	}
}

void CStructure::InstallUpdate(updatetype_t eUpdate)
{
	m_bInstalling = true;

	int iUninstalled = GetFirstUninstalledUpdate(eUpdate);
	if (iUninstalled < 0)
		return;

	m_eInstallingType = eUpdate;
	m_iInstallingUpdate = iUninstalled;

	m_iProductionToInstall = m_apUpdates[eUpdate][iUninstalled]->m_iProductionToInstall;
}

void CStructure::InstallComplete()
{
	m_bInstalling = false;

	m_aiUpdatesInstalled[m_eInstallingType] = m_iInstallingUpdate+1;

	CUpdateItem* pUpdate = m_apUpdates[m_eInstallingType][m_iInstallingUpdate];

	switch (pUpdate->m_eUpdateType)
	{
	case UPDATETYPE_BANDWIDTH:
		m_iBandwidth += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_PRODUCTION:
		m_iPower += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_FLEETSUPPLY:
		m_iFleetSupply += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_SUPPORTENERGY:
		m_iEnergyBonus += (size_t)pUpdate->m_flValue;
		break;

	case UPDATETYPE_SUPPORTRECHARGE:
		m_flRechargeBonus += (size_t)pUpdate->m_flValue;
		break;
	}

	GetDigitanksTeam()->SetCurrentSelection(this);
}

void CStructure::CancelInstall()
{
	m_bInstalling = false;

	m_iProductionToInstall = 0;
}

size_t CStructure::GetTurnsToInstall()
{
	return (size_t)(m_iProductionToInstall/GetDigitanksTeam()->GetProductionPerLoader())+1;
}

int CStructure::GetFirstUninstalledUpdate(updatetype_t eUpdate)
{
	std::vector<class CUpdateItem*>& aUpdates = m_apUpdates[eUpdate];
	size_t iUpdatesInstalled = m_aiUpdatesInstalled[eUpdate];

	if (iUpdatesInstalled >= aUpdates.size())
		return -1;

	if (aUpdates.size() == 0)
		return -1;

	return (int)iUpdatesInstalled;
}

CUpdateItem* CStructure::GetUpdateInstalling()
{
	if (IsInstalling())
		return m_apUpdates[m_eInstallingType][m_iInstallingUpdate];

	return NULL;
}

void CStructure::DownloadComplete(CUpdateItem* pItem)
{
	if (IsConstructing())
		return;

	if (pItem->m_eStructure != GetUnitType())
		return;

	if (pItem->m_eUpdateClass != UPDATECLASS_STRUCTUREUPDATE)
		return;

	m_apUpdates[pItem->m_eUpdateType].push_back(pItem);
}

void CStructure::SetSupplier(class CSupplier* pSupplier)
{
	m_hSupplier = pSupplier;

	if (m_hSupplyLine == NULL && m_hSupplier != NULL)
		m_hSupplyLine = Game()->Create<CSupplyLine>("CSupplyLine");

	if (m_hSupplyLine != NULL && m_hSupplier == NULL)
		Game()->Delete(m_hSupplyLine);

	if (m_hSupplyLine != NULL && m_hSupplier != NULL)
		m_hSupplyLine->SetEntities(m_hSupplier, this);
}

void CStructure::ModifyContext(class CRenderingContext* pContext)
{
	BaseClass::ModifyContext(pContext);

	if (IsConstructing())
	{
		pContext->SetBlend(BLEND_ALPHA);
		pContext->SetColor(Color(255, 255, 255));
		pContext->SetAlpha(0.3f * GetVisibility());
	}

	if (GetTeam())
		pContext->SetColorSwap(GetTeam()->GetColor());
}

void CStructure::OnDeleted()
{
	BaseClass::OnDeleted();

	SetSupplier(NULL);
}

size_t CSupplier::s_iTendrilBeam = 0;

void CSupplier::Precache()
{
	BaseClass::Precache();

	s_iTendrilBeam = CRenderer::LoadTextureIntoGL(L"textures/beam-pulse.png");
}

void CSupplier::Spawn()
{
	BaseClass::Spawn();

	m_iDataStrength = InitialDataStrength();
	m_flBonusDataFlow = 0;

	m_iTendrilsCallList = 0;
}

float CSupplier::GetDataFlowRate()
{
	return BaseDataFlowPerTurn() + m_flBonusDataFlow;
}

float CSupplier::GetDataFlowRadius() const
{
	// Opposite of formula for area of a circle.
	return sqrt(m_iDataStrength/M_PI) + GetBoundingRadius();
}

float CSupplier::GetDataFlow(Vector vecPoint)
{
	return RemapValClamped((vecPoint - GetOrigin()).Length2D(), GetBoundingRadius(), GetDataFlowRadius()+GetBoundingRadius(), (float)m_iDataStrength, 0);
}

float CSupplier::GetDataFlow(Vector vecPoint, CTeam* pTeam, CSupplier* pIgnore)
{
	float flDataStrength = 0;
	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pTeam)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (pSupplier == pIgnore)
			continue;

		flDataStrength += pSupplier->GetDataFlow(vecPoint);
	}

	return flDataStrength;
}

void CSupplier::CalculateDataFlow()
{
	if (IsConstructing())
		return;

	if (m_hSupplier != NULL)
	{
		// Use the radius of a circle with the area of the given data flow
		// so the flow doesn't get huge when you're close to a source.
		m_flBonusDataFlow = sqrt(m_hSupplier->GetDataFlow(GetOrigin())/M_PI);
	}
	else
		m_flBonusDataFlow = 0;

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		CSupplier* pSupplier = dynamic_cast<CSupplier*>(m_ahChildren[i].GetPointer());
		if (pSupplier)
			pSupplier->CalculateDataFlow();
	}
}

float CSupplier::GetChildEfficiency()
{
	size_t iConsumingChildren = 0;
	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		if (m_ahChildren[i] == NULL)
			continue;

		if (!dynamic_cast<CStructure*>(m_ahChildren[i].GetPointer()))
			continue;

		if (dynamic_cast<CSupplier*>(m_ahChildren[i].GetPointer()))
			continue;

		iConsumingChildren++;
	}

	if (iConsumingChildren <= 2)
		return 1;

	// 3 == 0.5, 4 == 0.25, 5 == .2, and so on.
	return 1/((float)iConsumingChildren-1);
}

void CSupplier::StartTurn()
{
	BaseClass::StartTurn();

	if (!IsConstructing())
		m_iDataStrength += (size_t)GetDataFlowRate();

	UpdateTendrils();
}

void CSupplier::PostRender()
{
	CRenderingContext r(Game()->GetRenderer());
	if (DigitanksGame()->ShouldRenderFogOfWar())
		r.UseFrameBuffer(DigitanksGame()->GetDigitanksRenderer()->GetVisibilityMaskedBuffer());
	r.SetDepthMask(false);
	r.BindTexture(s_iTendrilBeam);

	GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();
	glUseProgram(iScrollingTextureProgram);

	GLuint flTime = glGetUniformLocation(iScrollingTextureProgram, "flTime");
	glUniform1f(flTime, Game()->GetGameTime());

	GLuint iTexture = glGetUniformLocation(iScrollingTextureProgram, "iTexture");
	glUniform1f(iTexture, 0);

	glCallList((GLuint)m_iTendrilsCallList);

	glUseProgram(0);
}

void CSupplier::UpdateTendrils()
{
	if (IsConstructing())
		return;

	size_t iRadius = (size_t)GetDataFlowRadius();
	while (m_aTendrils.size() < iRadius)
	{
		m_aTendrils.push_back(CTendril());
		CTendril* pTendril = &m_aTendrils[m_aTendrils.size()-1];
		pTendril->m_flLength = (float)m_aTendrils.size() + GetBoundingRadius();
		pTendril->m_vecEndPoint = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(EAngle(0, RandomFloat(0, 360), 0)) * pTendril->m_flLength);
		pTendril->m_flScale = RandomFloat(3, 7);
		pTendril->m_flOffset = RandomFloat(0, 1);
		pTendril->m_flSpeed = RandomFloat(0.5f, 2);
	}

	if (m_iTendrilsCallList)
		glDeleteLists((GLuint)m_iTendrilsCallList, 1);

	m_iTendrilsCallList = glGenLists(1);

	glNewList((GLuint)m_iTendrilsCallList, GL_COMPILE);
	for (size_t i = 0; i < m_aTendrils.size(); i++)
	{
		CTendril* pTendril = &m_aTendrils[i];

		Vector vecDestination = pTendril->m_vecEndPoint;

		Vector vecPath = vecDestination - GetOrigin();
		vecPath.y = 0;

		float flDistance = vecPath.Length2D();
		Vector vecDirection = vecPath.Normalized();
		size_t iSegments = (size_t)(flDistance/2);

		Color clrTeam = GetTeam()->GetColor();

		GLuint iScrollingTextureProgram = (GLuint)CShaderLibrary::GetScrollingTextureProgram();

		GLuint flSpeed = glGetUniformLocation(iScrollingTextureProgram, "flSpeed");
		glUniform1f(flSpeed, pTendril->m_flSpeed);

		CRopeRenderer oRope(Game()->GetRenderer(), s_iTendrilBeam, DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin()) + Vector(0, 1, 0));
		oRope.SetColor(clrTeam);
		oRope.SetTextureScale(pTendril->m_flScale);
		oRope.SetTextureOffset(pTendril->m_flOffset);
		oRope.SetForward(Vector(0, -1, 0));

		for (size_t i = 1; i < iSegments; i++)
		{
			float flCurrentDistance = ((float)i*flDistance)/iSegments;
			oRope.AddLink(DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection*flCurrentDistance) + Vector(0, 1, 0));
		}

		oRope.Finish(DigitanksGame()->GetTerrain()->SetPointHeight(vecDestination) + Vector(0, 1, 0));
	}
	glEndList();
}

void CSupplier::AddChild(CStructure* pChild)
{
	m_ahChildren.push_back(pChild);
}

void CSupplier::OnDeleted(CBaseEntity* pEntity)
{
	BaseClass::OnDeleted(pEntity);

	for (size_t i = 0; i < m_ahChildren.size(); i++)
	{
		if (m_ahChildren[i] == pEntity)
			m_ahChildren.erase(m_ahChildren.begin()+i);
	}
}

CSupplier* CSupplier::FindClosestSupplier(CBaseEntity* pUnit)
{
	CSupplier* pClosest = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		if (pEntity == pUnit)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pUnit->GetTeam())
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetOrigin() - pUnit->GetOrigin()).Length() < (pClosest->GetOrigin() - pUnit->GetOrigin()).Length())
			pClosest = pSupplier;
	}

	return pClosest;
}

CSupplier* CSupplier::FindClosestSupplier(Vector vecPoint, CTeam* pTeam)
{
	CSupplier* pClosest = NULL;

	for (size_t i = 0; i < CBaseEntity::GetNumEntities(); i++)
	{
		CBaseEntity* pEntity = CBaseEntity::GetEntityNumber(i);
		if (!pEntity)
			continue;

		CSupplier* pSupplier = dynamic_cast<CSupplier*>(pEntity);
		if (!pSupplier)
			continue;

		if (pSupplier->GetTeam() != pTeam)
			continue;

		if (pSupplier->IsConstructing())
			continue;

		if (!pClosest)
		{
			pClosest = pSupplier;
			continue;
		}

		if ((pSupplier->GetOrigin() - vecPoint).Length() < (pClosest->GetOrigin() - vecPoint).Length())
			pClosest = pSupplier;
	}

	return pClosest;
}

float CSupplier::VisibleRange() const
{
	return GetDataFlowRadius() + 5;
}
