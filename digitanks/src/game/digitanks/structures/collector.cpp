#include "collector.h"

#include <renderer/renderer.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

#include <digitanks/digitanksteam.h>
#include <digitanks/digitanksgame.h>

REGISTER_ENTITY(CCollector);

NETVAR_TABLE_BEGIN(CCollector);
	NETVAR_DEFINE(CEntityHandle<CResource>, m_hResource);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CCollector);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, CEntityHandle<CResource>, m_hResource);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CCollector);
INPUTS_TABLE_END();

void CCollector::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/psu.obj");
}

void CCollector::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/psu.obj");
}

void CCollector::ClientSpawn()
{
	BaseClass::ClientSpawn();

	if (GameNetwork()->IsHost() && !GetResource())
	{
		SetResource(CResource::FindClosestResource(GetOrigin(), GetResourceType()));
		if (GetResource())
			GetResource()->SetCollector(this);
		else
			Delete();
	}
}

void CCollector::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";

	s += L"POWER SUPPLY UNIT\n";
	s += L"Resource collector\n \n";

	if (GetTeam())
	{
		s += L"Team: " + GetTeam()->GetTeamName() + L"\n";
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += L" Friendly\n \n";
		else
			s += L" Hostile\n \n";
	}
	else
	{
		s += L"Team: Neutral\n \n";
	}

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToConstruct());
		return;
	}

	if (GetSupplier() && m_hSupplyLine != NULL)
	{
		s += p.sprintf(L"Power: %.1f/turn\n", GetPowerProduced());
		s += p.sprintf(L"Efficiency: %d\n", (int)(m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity() * 100));
		return;
	}
}

float CCollector::GetPowerProduced() const
{
	if (m_hSupplyLine == NULL || GetSupplier() == NULL)
		return 0;

	return 1.2f * m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity();
}

REGISTER_ENTITY(CBattery);

NETVAR_TABLE_BEGIN(CBattery);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CBattery);
SAVEDATA_TABLE_END();

INPUTS_TABLE_BEGIN(CBattery);
INPUTS_TABLE_END();

void CBattery::Spawn()
{
	BaseClass::Spawn();

	SetModel(L"models/structures/battery.obj");
}

void CBattery::Precache()
{
	BaseClass::Precache();

	PrecacheModel(L"models/structures/battery.obj");
}

void CBattery::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();
	eastl::string16 p;

	if (!IsConstructing() && !IsUpgrading() && CanStructureUpgrade())
	{
		pHUD->SetButtonTexture(0, "PSU");

		if (UpgradeCost() <= GetDigitanksTeam()->GetPower())
		{
			pHUD->SetButtonListener(0, CHUD::BeginUpgrade);
			pHUD->SetButtonColor(0, Color(150, 150, 150));
		}

		eastl::string16 s;
		s += L"UPGRADE TO POWER SUPPLY UNIT\n \n";
		s += L"Power Supply Units provide 2 additional Power per turn. Upgrading will make this structure inactive until the upgrade is complete.\n \n";
		s += p.sprintf(L"Turns to upgrade: %d Turns\n \n", GetTurnsToUpgrade());
		s += L"Shortcut: Q";

		pHUD->SetButtonInfo(0, s);
		pHUD->SetButtonTooltip(0, L"Upgrade To PSU");
	}
}

void CBattery::UpdateInfo(eastl::string16& s)
{
	eastl::string16 p;
	s = L"";
	s += L"CAPACITOR\n";
	s += L"Resource collector\n \n";

	if (GetTeam())
	{
		s += L"Team: " + GetTeam()->GetTeamName() + L"\n";
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += L" Friendly\n \n";
		else
			s += L" Hostile\n \n";
	}
	else
	{
		s += L"Team: Neutral\n \n";
	}

	if (IsConstructing())
	{
		s += L"(Constructing)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToConstruct());
		return;
	}

	if (IsUpgrading())
	{
		s += L"(Upgrading to Power Supply Unit)\n";
		s += p.sprintf(L"Turns left: %d\n", GetTurnsRemainingToUpgrade());
		return;
	}

	if (m_hSupplier != NULL && m_hSupplyLine != NULL)
	{
		s += p.sprintf(L"Power Supplied: %.1f\n", GetPowerProduced());
		s += p.sprintf(L"Efficiency: %d\n", (int)(m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity() * 100));
		return;
	}
}

bool CBattery::CanStructureUpgrade()
{
	if (!GetDigitanksTeam())
		return false;

	return GetDigitanksTeam()->CanBuildPSUs();
}

void CBattery::UpgradeComplete()
{
	if (!GameNetwork()->IsHost())
		return;

	CCollector* pCollector = GameServer()->Create<CCollector>("CCollector");
	pCollector->SetConstructing(false);
	pCollector->SetOrigin(GetOrigin());
	GetTeam()->AddEntity(pCollector);
	pCollector->SetSupplier(GetSupplier());
	pCollector->SetResource(GetResource());
	GetResource()->SetCollector(pCollector);
	pCollector->CalculateVisibility();

	Delete();

	GetDigitanksTeam()->AddActionItem(pCollector, ACTIONTYPE_UPGRADE);
}

float CBattery::GetPowerProduced() const
{
	if (m_hSupplyLine == NULL || m_hSupplier == NULL)
		return 0;

	return 0.5f * m_hSupplier->GetChildEfficiency() * m_hSupplyLine->GetIntegrity();
}
