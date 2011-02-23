#include <digitanks/units/digitank.h>

#include <sstream>

#include <GL/glew.h>
#include <maths.h>
#include <mtrand.h>
#include <strutils.h>
#include <models/models.h>
#include <renderer/renderer.h>
#include <renderer/particles.h>
#include <glgui/glgui.h>
#include <shaders/shaders.h>

#include <digitanks/digitanksgame.h>
#include <digitanks/dt_camera.h>
#include "ui/digitankswindow.h"
#include "ui/instructor.h"
#include <digitanks/powerup.h>
#include "ui/debugdraw.h"
#include "ui/hud.h"
#include <digitanks/structures/structure.h>
#include <digitanks/supplyline.h>
#include <digitanks/weapons/projectile.h>
#include <digitanks/weapons/cameraguided.h>
#include <digitanks/weapons/laser.h>
#include <digitanks/weapons/missiledefense.h>
#include <digitanks/units/scout.h>
#include <digitanks/units/standardtank.h>
#include <digitanks/units/maintank.h>
#include <digitanks/units/mechinf.h>

size_t CDigitank::s_iAimBeam = 0;
size_t CDigitank::s_iAutoMove = 0;
size_t CDigitank::s_iSupportGlow = 0;

const char* CDigitank::s_apszTankLines[] =
{
	"^.^",	// TANKLINE_CUTE
	"<3",	// TANKLINE_LOVE
	":D!",	// TANKLINE_HAPPY
	"\\o/",	// TANKLINE_CHEER
	">D",	// TANKLINE_EVIL
	">.<",	// TANKLINE_SQUINT
	"8)",	// TANKLINE_COOL
	"X(",	// TANKLINE_DEAD
	"x.x",	// TANKLINE_DEAD2
	"#(",	// TANKLINE_DEAD3
	":/",	// TANKLINE_FROWN
	":(",	// TANKLINE_SAD
	"zzz",	// TANKLINE_ASLEEP
	"??",	// TANKLINE_CONFUSED
	"...",	// TANKLINE_DOTDOTDOT
	"!?",	// TANKLINE_SURPRISED
	"!!",	// TANKLINE_THRILLED
	":P",	// TANKLINE_TONGUE
};

eastl::map<size_t, eastl::vector<size_t> > g_aiSpeechLines;

REGISTER_ENTITY(CDigitank);

NETVAR_TABLE_BEGIN(CDigitank);
	NETVAR_DEFINE(float, m_flStartingPower);
	NETVAR_DEFINE(float, m_flTotalPower);
	NETVAR_DEFINE(float, m_flAttackPower);
	NETVAR_DEFINE(float, m_flDefensePower);
	NETVAR_DEFINE(float, m_flMovementPower);

	NETVAR_DEFINE(float, m_flBonusAttackPower);
	NETVAR_DEFINE(float, m_flBonusDefensePower);
	NETVAR_DEFINE(float, m_flBonusMovementPower);
	NETVAR_DEFINE_CALLBACK(size_t, m_iBonusPoints, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(float, m_flRangeBonus);

	NETVAR_DEFINE(float, m_flShieldStrength);

	NETVAR_DEFINE(Vector, m_vecPreviousOrigin);

	NETVAR_DEFINE(float, m_flPreviousTurn);

	NETVAR_DEFINE(Vector, m_vecLastAim);

	NETVAR_DEFINE_CALLBACK(bool, m_bHasCloak, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(bool, m_bCloaked);

	NETVAR_DEFINE_CALLBACK(bool, m_bGoalMovePosition, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(Vector, m_vecGoalMovePosition);

	NETVAR_DEFINE_CALLBACK(bool, m_bLostConcealment, &CDigitanksGame::UpdateHUD);

	NETVAR_DEFINE_CALLBACK(bool, m_bFortified, &CDigitanksGame::UpdateHUD);
	NETVAR_DEFINE(size_t, m_iFortifyLevel);

	NETVAR_DEFINE_CALLBACK(bool, m_bSentried, &CDigitanksGame::UpdateHUD);

	NETVAR_DEFINE(size_t, m_iTurnsDisabled);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CDigitank);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flStartingPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flTotalPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flAttackPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flDefensePower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flMovementPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusAttackPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusDefensePower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flBonusMovementPower);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iBonusPoints);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flRangeBonus);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flMaxShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flShieldStrength);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flStartedRock);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flRockIntensity);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecRockDirection);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewMove);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecPreviousOrigin);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flStartedMove);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iMoveType);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flPreviewTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, float, m_flPreviousTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flStartedTurn);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bPreviewAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecPreviewAim);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecLastAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bDisplayAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecDisplayAim);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flDisplayAimRadius);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CBaseEntity>, m_hPreviewCharge);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flBeginCharge);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flEndCharge);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CBaseEntity>, m_hChargeTarget);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bHasCloak);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bCloaked);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bGoalMovePosition);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, Vector, m_vecGoalMovePosition);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFiredWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bActionTaken);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bLostConcealment);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFireWeaponTime);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iFireWeapons);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<CBaseWeapon>, m_hWeapon);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLastSpeech);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextIdle);
	//size_t						m_iTurretModel;	// Set in Spawn()
	//size_t						m_iShieldModel;	// Set in Spawn()
	//size_t						m_iHoverParticles;	// Dynamic
	//size_t						m_iSmokeParticles;	// Dynamic
	//size_t						m_iFireParticles;	// Dynamic
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bFortified);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iFortifyLevel);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flFortifyTime);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, bool, m_bSentried);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CSupplier>, m_hSupplier);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, CEntityHandle<class CSupplyLine>, m_hSupplyLine);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flBobOffset);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iAirstrikes);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iMissileDefenses);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bInAttackTeam);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bFortifyPoint);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecFortifyPoint);
	SAVEDATA_DEFINE(CSaveData::DATA_NETVAR, size_t, m_iTurnsDisabled);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextThink);
SAVEDATA_TABLE_END();

CDigitank::~CDigitank()
{
	if (m_iHoverParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iHoverParticles);
	if (m_iSmokeParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iSmokeParticles);
	if (m_iFireParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iFireParticles);
}

void CDigitank::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem(L"tank-fire");
	PrecacheParticleSystem(L"promotion");
	PrecacheParticleSystem(L"tank-hover");
	PrecacheParticleSystem(L"digitank-smoke");
	PrecacheParticleSystem(L"digitank-fire");
	PrecacheParticleSystem(L"charge-burst");
	PrecacheParticleSystem(L"charge-charge");
	PrecacheSound(L"sound/tank-fire.wav");
	PrecacheSound(L"sound/shield-damage.wav");
	PrecacheSound(L"sound/tank-damage.wav");
	PrecacheSound(L"sound/tank-active.wav");
	PrecacheSound(L"sound/tank-active2.wav");
	PrecacheSound(L"sound/tank-move.wav");
	PrecacheSound(L"sound/tank-aim.wav");
	PrecacheSound(L"sound/tank-promoted.wav");

	s_iAimBeam = CRenderer::LoadTextureIntoGL(L"textures/beam-pulse.png");
	s_iAutoMove = CRenderer::LoadTextureIntoGL(L"textures/auto-move.png");
	s_iSupportGlow = CRenderer::LoadTextureIntoGL(L"textures/particles/support.png");

	SetupSpeechLines();
}

void CDigitank::SetupSpeechLines()
{
	if (g_aiSpeechLines.size())
		return;

	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_SELECTED].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_MOVED].push_back(TANKLINE_CHEER);
	g_aiSpeechLines[TANKSPEECH_MOVED].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_ATTACK].push_back(TANKLINE_EVIL);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_SQUINT);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_SURPRISED);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_DEAD);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_DEAD2);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_DEAD3);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_FROWN);
	g_aiSpeechLines[TANKSPEECH_DAMAGED].push_back(TANKLINE_SAD);
	g_aiSpeechLines[TANKSPEECH_KILL].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_KILL].push_back(TANKLINE_EVIL);
	g_aiSpeechLines[TANKSPEECH_KILL].push_back(TANKLINE_THRILLED);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_FROWN);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_SAD);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_SURPRISED);
	g_aiSpeechLines[TANKSPEECH_MISSED].push_back(TANKLINE_DOTDOTDOT);
	g_aiSpeechLines[TANKSPEECH_IDLE].push_back(TANKLINE_ASLEEP);
	g_aiSpeechLines[TANKSPEECH_IDLE].push_back(TANKLINE_CONFUSED);
	g_aiSpeechLines[TANKSPEECH_IDLE].push_back(TANKLINE_DOTDOTDOT);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_LOVE);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_PROMOTED].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_HAPPY);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_LOVE);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_THRILLED);
	g_aiSpeechLines[TANKSPEECH_PARTY].push_back(TANKLINE_CHEER);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_CUTE);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_CHEER);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_EVIL);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_COOL);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_SURPRISED);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_THRILLED);
	g_aiSpeechLines[TANKSPEECH_TAUNT].push_back(TANKLINE_TONGUE);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_ASLEEP);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_SQUINT);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_DEAD);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_DEAD2);
	g_aiSpeechLines[TANKSPEECH_DISABLED].push_back(TANKLINE_DEAD3);
}

void CDigitank::Spawn()
{
	BaseClass::Spawn();

	SetCollisionGroup(CG_ENTITY);

	m_flStartingPower = 10;
	m_flAttackPower = 0;
	m_flDefensePower = 10;
	m_flMovementPower = 0;
	m_flTotalPower = 0;
	m_flBonusAttackPower = m_flBonusDefensePower = m_flBonusMovementPower = 0;
	m_flRangeBonus = 0;
	m_iBonusPoints = 0;
	m_flPreviewTurn = 0;
	m_bPreviewAim = false;
	m_bGoalMovePosition = false;
	m_bFiredWeapon = false;
	m_bActionTaken = false;
	m_flStartedMove = 0;
	m_flStartedTurn = 0;
	m_bFortified = false;
	m_bSentried = false;
	m_flMaxShieldStrength = 150;
	m_flShieldStrength = 150;
	m_bNeedsOrdersDirty = true;
	m_flFireWeaponTime = 0;
	m_iFireWeapons = 0;
	m_flLastSpeech = 0;
	m_flNextIdle = 10.0f;
	m_iTurretModel = m_iShieldModel = ~0;
	m_iHoverParticles = ~0;
	m_iSmokeParticles = ~0;
	m_iFireParticles = ~0;
	m_bInAttackTeam = false;
	m_bFortifyPoint = false;
	m_flFortifyTime = 0;
	m_flBobOffset = RandomFloat(0, 10);
	m_flStartedRock = -100;
	m_flBeginCharge = -1;
	m_flEndCharge = -1;
	m_iAirstrikes = 0;
	m_iMissileDefenses = 0;
	m_flNextMissileDefense = 0;
	m_iTurnsDisabled = 0;
	m_flCurrentTurretYaw = 0;
	m_flGoalTurretYaw = 0;
	m_flGlowYaw = 0;
	m_flNextThink = GameServer()->GetGameTime();
	m_flShieldPulse = 0;
}

float CDigitank::GetBaseAttackPower(bool bPreview)
{
	if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && GetDigitanksTeam()->IsSelected(this) && bPreview && DigitanksGame()->GetControlMode() == MODE_AIM)
		return GetWeaponEnergy();

	return m_flAttackPower;
}

float CDigitank::GetBaseDefensePower(bool bPreview)
{
	float flIntegrity = 1;
	if (DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
	{
		if (m_hSupplier == NULL)
			return 0;

		if (m_hSupplyLine == NULL)
			return 0;

		flIntegrity = m_hSupplyLine->GetIntegrity();
	}

	if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam() && GetDigitanksTeam()->IsSelected(this) && bPreview && DigitanksGame()->GetControlMode() == MODE_AIM)
		return m_flTotalPower - GetWeaponEnergy();

	// Any unallocated power will go into defense.
	return (m_flDefensePower + m_flTotalPower) * flIntegrity;
}

float CDigitank::GetAttackPower(bool bPreview)
{
	return GetBaseAttackPower(bPreview)+GetBonusAttackPower(bPreview);
}

float CDigitank::GetDefensePower(bool bPreview)
{
	Vector vecOrigin = GetOrigin();

	if (bPreview)
		vecOrigin = m_vecPreviewMove;

	if (IsDisabled())
		return 0;

	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(vecOrigin.x), CTerrain::WorldToArraySpace(vecOrigin.z), TB_WATER))
		return 0;

	float flDefenseBonus = 0;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(vecOrigin.x), CTerrain::WorldToArraySpace(vecOrigin.z), TB_TREE))
		flDefenseBonus = 5;

	return GetBaseDefensePower(bPreview)+GetBonusDefensePower(bPreview)+flDefenseBonus;
}

float CDigitank::GetTotalAttackPower()
{
	return m_flStartingPower.Get() + GetBonusAttackPower();
}

float CDigitank::GetTotalDefensePower()
{
	return m_flStartingPower.Get() + GetBonusDefensePower();
}


float CDigitank::GetMaxMovementEnergy() const
{
	float flNetworkBonus = 0;

	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flNetworkBonus = 8.0f;

	return GetStartingPower() + GetBonusMovementEnergy() + flNetworkBonus;
}

float CDigitank::GetMaxMovementDistance() const
{
	float flDistance = GetMaxMovementEnergy() * GetTankSpeed();

	if (flDistance < 0)
		return 0;

	return flDistance;
}

float CDigitank::GetUsedMovementEnergy(bool bPreview) const
{
	if (bPreview)
	{
		float flPreviewPower = GetPreviewMoveTurnPower();
		float flRemainingPower = GetRemainingMovementEnergy();
		if (flPreviewPower > flRemainingPower)
			return GetMaxMovementEnergy();
		else
			return m_flMovementPower.Get() + flPreviewPower;
	}

	return m_flMovementPower.Get();
}

float CDigitank::GetRemainingMovementEnergy(bool bPreview) const
{
	return GetMaxMovementEnergy() - GetUsedMovementEnergy(bPreview);
}

float CDigitank::GetRemainingMovementDistance() const
{
	float flDistance = GetRemainingMovementEnergy() * GetTankSpeed();

	if (flDistance < 0)
		return 0;

	return flDistance;
}

float CDigitank::GetRemainingTurningDistance() const
{
	return GetRemainingMovementEnergy() * TurnPerPower();
}

float CDigitank::GetBonusAttackScale(bool bPreview)
{
	return m_flAttackPower/m_flStartingPower;
}

float CDigitank::GetBonusDefenseScale(bool bPreview)
{
	return m_flDefensePower/m_flStartingPower;
}

float CDigitank::GetBonusAttackPower(bool bPreview)
{
	return (m_flBonusAttackPower + GetSupportAttackPowerBonus())*GetBonusAttackScale(bPreview);
}

float CDigitank::GetBonusDefensePower(bool bPreview)
{
	return (m_flBonusDefensePower + GetSupportDefensePowerBonus())*GetBonusDefenseScale(bPreview);
}

float CDigitank::GetSupportAttackPowerBonus()
{
	if (m_hSupplier == NULL)
		return 0;

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = (float)m_hSupplier->EnergyBonus() * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

float CDigitank::GetSupportDefensePowerBonus()
{
	if (m_hSupplier == NULL)
		return 0;

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = (float)m_hSupplier->EnergyBonus() * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

float CDigitank::GetSupportHealthRechargeBonus() const
{
	if (m_hSupplier == NULL)
		return 0.0f;

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = m_hSupplier->RechargeBonus()/5 * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

float CDigitank::GetSupportShieldRechargeBonus() const
{
	if (m_hSupplier == NULL)
		return 0.0f;

	if (m_hSupplyLine == NULL)
		return 0;

	float flBonus = 0;
	if (CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
		flBonus = m_hSupplier->RechargeBonus() * m_hSupplyLine->GetIntegrity();

	return flBonus;
}

float CDigitank::GetPreviewMoveTurnPower() const
{
	float flMovePower = 0;
	float flTurnPower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		flMovePower = GetPreviewBaseMovePower();

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		flTurnPower = GetPreviewBaseTurnPower();

	float flPower = flMovePower + flTurnPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewMovePower() const
{
	float flMovePower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_MOVE)
		flMovePower = GetPreviewBaseMovePower();

	float flPower = flMovePower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewTurnPower() const
{
	float flTurnPower = 0;

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
		flTurnPower = GetPreviewBaseTurnPower();

	float flPower = flTurnPower;
	if (flPower < 0)
		return 0;
	return flPower;
}

float CDigitank::GetPreviewBaseMovePower() const
{
	bool bHalfMovement = false;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(m_vecPreviewMove.x), CTerrain::WorldToArraySpace(m_vecPreviewMove.z), TB_TREE))
		bHalfMovement = true;
	if (DigitanksGame()->GetTerrain()->GetBit(CTerrain::WorldToArraySpace(m_vecPreviewMove.x), CTerrain::WorldToArraySpace(m_vecPreviewMove.z), TB_WATER))
		bHalfMovement = true;

	if (bHalfMovement)
		return (m_vecPreviewMove - GetOrigin()).Length() / (GetTankSpeed()*SlowMovementFactor());
	else
		return (m_vecPreviewMove - GetOrigin()).Length() / GetTankSpeed();
}

float CDigitank::GetPreviewBaseTurnPower() const
{
	return fabs(AngleDifference(m_flPreviewTurn, GetAngles().y)/TurnPerPower());
}

bool CDigitank::IsPreviewMoveValid() const
{
	if (GetPreviewBaseMovePower() > GetRemainingMovementEnergy())
		return false;

	if (DigitanksGame()->GetTerrain()->IsPointOverHole(GetPreviewMove()))
		return false;

	return DigitanksGame()->GetTerrain()->IsPointOnMap(GetPreviewMove());
}

float CDigitank::GetShieldStrength()
{
	if (GetShieldMaxStrength() == 0)
		return 0;

	return m_flShieldStrength/GetShieldMaxStrength() * GetDefenseScale(true);
}

float CDigitank::GetShieldBlockRadius()
{
	if (GetShieldMaxStrength() == 0)
		return GetBoundingRadius();

	if (GetShieldStrength() == 0)
		return GetBoundingRadius();

	return RenderShieldScale();
}

float CDigitank::GetShieldValue()
{
	return m_flShieldStrength;
}

void CDigitank::SetShieldValue(float flValue)
{
	m_flShieldStrength = flValue;
}

void CDigitank::StartTurn()
{
	BaseClass::StartTurn();

	DirtyNeedsOrders();

	ManageSupplyLine();

	if (CNetwork::IsHost() && m_bFortified)
	{
		if (m_iFortifyLevel < 5)
			m_iFortifyLevel++;
	}

	float flShieldStrength = GetShieldValue();
	SetShieldValue(Approach(m_flMaxShieldStrength, flShieldStrength, ShieldRechargeRate()));

	if (flShieldStrength - GetShieldValue() < 0)
		DigitanksGame()->OnTakeShieldDamage(this, NULL, NULL, flShieldStrength - GetShieldValue(), true, false);

	m_vecPreviewMove = GetOrigin();
	m_flPreviewTurn = GetAngles().y;

	if (CNetwork::IsHost())
	{
		m_flTotalPower = m_flStartingPower;
		m_flMovementPower = m_flAttackPower = m_flDefensePower = 0;
	}

	m_bActionTaken = false;
	m_bFiredWeapon = false;
	m_bLostConcealment = false;

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CDigitank* pClosestEnemy = FindClosestVisibleEnemyTank();

	if (HasGoalMovePosition() && pClosestEnemy)
	{
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_AUTOMOVEENEMY);
	}
	else
	// Artillery gets unit orders even if fortified but infantry doesn't.
	if (!HasGoalMovePosition() && (!IsFortified() || IsArtillery()) && !IsSentried())
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITORDERS);
	else
	// Notify if infantry can see an enemy they can shoot.
	if (IsInfantry() && IsFortified())
	{
		CDigitank* pClosestEnemyTank = FindClosestVisibleEnemyTank(true);
		if (pClosestEnemyTank && pClosestEnemyTank->GetVisibility() > 0.3f)
			DigitanksGame()->AddActionItem(this, ACTIONTYPE_FORTIFIEDENEMY);
	}

	if (HasGoalMovePosition())
		MoveTowardsGoalMovePosition();

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == GetDigitanksTeam())
	{
		size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();
		if (iTutorial == CInstructor::TUTORIAL_ENTERKEY)
			DigitanksWindow()->GetInstructor()->NextTutorial();
	}
}

void CDigitank::EndTurn()
{
	BaseClass::EndTurn();

	if (CNetwork::IsHost())
	{
		m_flDefensePower = m_flTotalPower;
		m_flTotalPower = 0;

		if (TakesLavaDamage() && DigitanksGame()->GetTerrain()->IsPointOverLava(GetOrigin()))
			TakeDamage(NULL, NULL, DAMAGE_BURN, DigitanksGame()->LavaDamage(), false);
	}

	if (m_iTurnsDisabled)
		m_iTurnsDisabled--;
}

void CDigitank::ManageSupplyLine()
{
	if (!CNetwork::IsHost())
		return;

	// Scouts don't get supply lines. They give away your base's location in the early game!
	if (IsScout())
		return;

	CSupplier* pSupplier = CSupplier::FindClosestSupplier(this);
	CSupplyLine* pSupplyLine = m_hSupplyLine;

	if (pSupplyLine == NULL && pSupplier != NULL)
		pSupplyLine = GameServer()->Create<CSupplyLine>("CSupplyLine");

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = pSupplier?pSupplier->GetHandle():~0;
	p.ui3 = pSupplyLine?pSupplyLine->GetHandle():~0;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "ManageSupplyLine", &p);

	ManageSupplyLine(&p);
}

void CDigitank::ManageSupplyLine(CNetworkParameters* p)
{
	m_hSupplier = CEntityHandle<CSupplier>(p->ui2);
	m_hSupplyLine = CEntityHandle<CSupplyLine>(p->ui3);

	if (m_hSupplyLine != NULL && m_hSupplier != NULL)
		m_hSupplyLine->SetEntities(m_hSupplier, this);
}

CDigitank* CDigitank::FindClosestVisibleEnemyTank(bool bInRange)
{
	CDigitank* pClosestEnemy = NULL;
	while (true)
	{
		pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(GetOrigin(), pClosestEnemy);

		if (!pClosestEnemy)
			break;

		if (pClosestEnemy->GetTeam() == GetTeam())
			continue;

		if (bInRange)
		{
			if (!IsInsideMaxRange(pClosestEnemy->GetOrigin()))
				return NULL;
		}
		else
		{
			if ((pClosestEnemy->GetOrigin() - GetOrigin()).Length() > VisibleRange()+DigitanksGame()->FogPenetrationDistance())
				return NULL;
		}

		break;
	}

	return pClosestEnemy;
}

void CDigitank::SetPreviewMove(Vector vecPreviewMove)
{
	if (IsFortified() && !CanMoveFortified())
		return;

	m_vecPreviewMove = vecPreviewMove;
}

void CDigitank::ClearPreviewMove()
{
	m_vecPreviewMove = GetOrigin();
}

void CDigitank::SetPreviewTurn(float flPreviewTurn)
{
	if (IsFortified() && !CanTurnFortified())
		return;

	m_flPreviewTurn = flPreviewTurn;
}

void CDigitank::ClearPreviewTurn()
{
	m_flPreviewTurn = GetAngles().y;
}

void CDigitank::SetPreviewAim(Vector vecPreviewAim)
{
	if (CanFortify() && !IsFortified() && !CanAimMobilized())
		return;

	if (DigitanksGame()->GetAimType() == AIM_NORANGE)
	{
		// Special weapons can be aimed anywhere the player can see.
		m_vecPreviewAim = vecPreviewAim;
		m_bPreviewAim = GetDigitanksTeam()->GetVisibilityAtPoint(m_vecPreviewAim) > 0;
		return;
	}

	m_bPreviewAim = true;

	while (!IsInsideMaxRange(vecPreviewAim))
	{
		Vector vecDirection = vecPreviewAim - GetOrigin();
		vecDirection.y = 0;
		vecPreviewAim = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection.Normalized() * vecDirection.Length2D() * 0.99f);
	}

	if ((vecPreviewAim - GetOrigin()).Length2DSqr() < GetMinRange()*GetMinRange())
	{
		vecPreviewAim = GetOrigin() + (vecPreviewAim - GetOrigin()).Normalized() * GetMinRange() * 1.01f;
		vecPreviewAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecPreviewAim.x, vecPreviewAim.z);
	}

	if (fabs(AngleDifference(GetAngles().y, VectorAngles((vecPreviewAim-GetOrigin()).Normalized()).y)) > FiringCone())
	{
		m_bPreviewAim = false;
		return;
	}

	m_vecPreviewAim = vecPreviewAim;
}

void CDigitank::ClearPreviewAim()
{
	m_bPreviewAim = false;
	m_vecPreviewAim = GetOrigin();
}

bool CDigitank::IsPreviewAimValid()
{
	if (!m_bPreviewAim)
		return false;

	if (!IsInsideMaxRange(GetPreviewAim()))
		return false;

	if ((GetPreviewAim() - GetOrigin()).LengthSqr() < GetMinRange()*GetMinRange())
		return false;

	return true;
}

bool CDigitank::CanCharge() const
{
	for (size_t i = 0; i < m_aeWeapons.size(); i++)
		if (m_aeWeapons[i] == WEAPON_CHARGERAM)
			return true;

	return false;
}

float CDigitank::ChargeRadius() const
{
	return BaseChargeRadius() * RemapValClamped(m_flMovementPower.Get(), 0, m_flStartingPower.Get(), 1.0f, 0.5f);
}

void CDigitank::SetPreviewCharge(CBaseEntity* pChargeTarget)
{
	if (!pChargeTarget)
	{
		m_hPreviewCharge = NULL;
		return;
	}

	if (dynamic_cast<CTerrain*>(pChargeTarget))
	{
		m_hPreviewCharge = NULL;
		return;
	}

	CDigitanksEntity* pDTEnt = dynamic_cast<CDigitanksEntity*>(pChargeTarget);
	if (pDTEnt && !pDTEnt->IsRammable())
		return;

	if (pChargeTarget->GetTeam() == GetTeam())
		return;

	if (pChargeTarget->Distance(GetOrigin()) > ChargeRadius())
		return;

	m_hPreviewCharge = pChargeTarget;
}

void CDigitank::ClearPreviewCharge()
{
	m_hPreviewCharge = NULL;
}

Vector CDigitank::GetChargePosition(CBaseEntity* pTarget) const
{
	float flTargetSize = pTarget->GetBoundingRadius() + GetBoundingRadius();
	Vector vecChargeDirection = (pTarget->GetOrigin() - GetOrigin()).Normalized();
	Vector vecChargePosition = pTarget->GetOrigin() - vecChargeDirection*flTargetSize;
	vecChargePosition.y = FindHoverHeight(vecChargePosition);
	return vecChargePosition;
}

bool CDigitank::IsInsideMaxRange(Vector vecPoint)
{
	if (GetCurrentWeapon() == WEAPON_CHARGERAM)
		return (vecPoint - GetOrigin()).LengthSqr() < ChargeRadius()*ChargeRadius();

	Vector vecDirection = vecPoint - GetOrigin();
	float flPreviewDistanceSqr = vecDirection.LengthSqr();
	float flPreviewDistance2DSqr = vecDirection.Length2DSqr();
	float flHeightToTank = vecDirection.y;
	if (flHeightToTank*flHeightToTank > ((flPreviewDistanceSqr/2) * (flPreviewDistanceSqr/2)))
	{
		if (flPreviewDistanceSqr > GetMaxRange()*GetMaxRange())
			return false;
		else
			return true;
	}
	else
	{
		float flMaxRange = GetMaxRange()*1.115f-(flHeightToTank/2);
		if (flPreviewDistance2DSqr > flMaxRange*flMaxRange)
			return false;
		else
			return true;
	}
}

float CDigitank::FindAimRadius(Vector vecPoint, float flMin)
{
	Vector vecDirection = vecPoint - GetOrigin();
	float flPreviewDistanceSqr = vecDirection.LengthSqr();
	float flPreviewDistance2DSqr = vecDirection.Length2DSqr();
	float flHeightToTank = vecDirection.y;
	if (flHeightToTank*flHeightToTank > ((flPreviewDistanceSqr/2) * (flPreviewDistanceSqr/2)))
		return RemapValClamped(flPreviewDistanceSqr, GetEffRange()*GetEffRange(), GetMaxRange()*GetMaxRange(), flMin, MaxRangeRadius());
	else
	{
		float flMaxRange = GetMaxRange()*1.115f-(flHeightToTank/2);
		float flEffRange = GetEffRange()*1.115f-(flHeightToTank/2);
		return RemapValClamped(flPreviewDistance2DSqr, flEffRange*flEffRange, flMaxRange*flMaxRange, flMin, MaxRangeRadius());
	}
}

void CDigitank::RockTheBoat(float flIntensity, Vector vecDirection)
{
	if (IsFortified() || IsFortifying())
		return;

	m_flStartedRock = GameServer()->GetGameTime();
	m_flRockIntensity = flIntensity;
	m_vecRockDirection = vecDirection;
}

bool CDigitank::IsRocking() const
{
	float flTransitionTime = 1;

	float flTimeSinceRock = GameServer()->GetGameTime() - m_flStartedRock;
	if (m_flStartedRock && flTimeSinceRock < flTransitionTime)
		return true;

	return false;
}

void CDigitank::Move()
{
	if (IsFortified() && !CanMoveFortified())
		return;

	if (!IsPreviewMoveValid())
		return;

	if (DigitanksGame()->GetTerrain()->IsPointOverHole(m_vecPreviewMove))
		return;

	if (!DigitanksGame()->GetTerrain()->IsPointOnMap(m_vecPreviewMove))
		return;

	if (IsDisabled())
		return;

	Speak(TANKSPEECH_MOVED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "Move", GetHandle(), m_vecPreviewMove.x, m_vecPreviewMove.y, m_vecPreviewMove.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_vecPreviewMove.x;
	p.fl3 = m_vecPreviewMove.y;
	p.fl4 = m_vecPreviewMove.z;
	Move(&p);
}

void CDigitank::Move(CNetworkParameters* p)
{
	m_vecPreviewMove = Vector(p->fl2, p->fl3, p->fl4);

	float flMovePower = GetPreviewBaseMovePower();

	Vector vecStart = GetOrigin();
	Vector vecEnd = m_vecPreviewMove;

	Move(m_vecPreviewMove);

	Turn(EAngle(0, VectorAngles(vecEnd-vecStart).y, 0));

	if (m_iHoverParticles != ~0)
		CParticleSystemLibrary::StopInstance(m_iHoverParticles);

	if (GetVisibility() > 0)
	{
		EmitSound(L"sound/tank-move.wav");
		SetSoundVolume(L"sound/tank-move.wav", 0.5f);

		m_iHoverParticles = CParticleSystemLibrary::AddInstance(L"tank-hover", GetOrigin());
		if (m_iHoverParticles != ~0)
			CParticleSystemLibrary::GetInstance(m_iHoverParticles)->FollowEntity(this);
	}

	// Am I waiting to fire something? Fire now. Shoot and scoot baby!
	if (m_flFireWeaponTime)
		m_flFireWeaponTime = GameServer()->GetGameTime();

	if (CNetwork::IsHost())
	{
		m_flMovementPower += flMovePower;
	}

	if (CNetwork::IsHost() && CanGetPowerups())
	{
		for (size_t i = 0; i < GameServer()->GetMaxEntities(); i++)
		{
			CBaseEntity* pEntity = CBaseEntity::GetEntity(i);

			if (!pEntity)
				continue;

			CPowerup* pPowerup = dynamic_cast<CPowerup*>(pEntity);

			if (!pPowerup)
				continue;

			Vector vecDistance = pPowerup->GetOrigin() - GetRealOrigin();
			vecDistance.y = 0;
			if (vecDistance.Length() < pPowerup->GetBoundingRadius() + GetBoundingRadius())
			{
				pPowerup->Delete();

				switch (pPowerup->GetPowerupType())
				{
				case POWERUP_BONUS:
				default:
					GiveBonusPoints(1);
					break;

				case POWERUP_AIRSTRIKE:
					m_iAirstrikes++;
					break;

				case POWERUP_MISSILEDEFENSE:
					m_iMissileDefenses += 3;
					break;

				case POWERUP_TANK:
				{
					CDigitank* pTank;
					if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
						pTank = GameServer()->Create<CStandardTank>("CStandardTank");
					else
					{
						switch(RandomInt(0, 4))
						{
						default:
						case 0:
						case 1:
							pTank = GameServer()->Create<CScout>("CScout");
							break;

						case 2:
						case 3:
							pTank = GameServer()->Create<CMechInfantry>("CMechInfantry");
							break;

						case 4:
							pTank = GameServer()->Create<CMainBattleTank>("CMainBattleTank");
							break;
						}
					}

					GetTeam()->AddEntity(pTank);

					Vector vecTank = m_vecOrigin.Get() - GetOrigin().Normalized() * (GetBoundingRadius()*2);
					vecTank.y = pTank->FindHoverHeight(vecTank);
					EAngle angTank = VectorAngles(-vecTank.Normalized());

					pTank->SetOrigin(vecTank);
					pTank->SetAngles(angTank);
					pTank->StartTurn();

					pTank->CalculateVisibility();
				}
				}

				DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_POWERUP);
			}
		}
	}

	for (size_t i = 0; i < Game()->GetNumTeams(); i++)
	{
		if (Game()->GetTeam(i))
			DigitanksGame()->GetDigitanksTeam(i)->CalculateVisibility();
	}

	InterceptSupplyLines();

	CDigitank* pClosestEnemy = FindClosestVisibleEnemyTank();
	if (HasGoalMovePosition() && pClosestEnemy)
	{
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_AUTOMOVEENEMY);
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	m_flGoalTurretYaw = 0;

	DirtyNeedsOrders();
	DigitanksGame()->HandledActionItem(this);

	DigitanksWindow()->GetHUD()->UpdateTurnButton();
}

bool CDigitank::IsMoving()
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceMove = GameServer()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
		return true;

	return false;
}

void CDigitank::Move(Vector vecNewPosition, int iMoveType)
{
	m_vecPreviousOrigin = GetOrigin();
	m_flStartedMove = GameServer()->GetGameTime();
	SetOrigin(vecNewPosition);
	m_iMoveType = iMoveType;

	if (TakesLavaDamage() && DigitanksGame()->GetTerrain()->IsPointOverLava(vecNewPosition))
		TakeDamage(NULL, NULL, DAMAGE_BURN, DigitanksGame()->LavaDamage(), false);

	if (IsSentried())
		Sentry();
}

void CDigitank::Turn()
{
	if (IsFortified() && !CanTurnFortified())
		return;

	if (IsDisabled())
		return;

	float flMovePower = GetPreviewBaseTurnPower();

	if (flMovePower > GetRemainingMovementEnergy())
		return;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "Turn", GetHandle(), m_flPreviewTurn);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_flPreviewTurn;
	Turn(&p);
}

void CDigitank::Turn(CNetworkParameters* p)
{
	m_flPreviewTurn = p->fl2;

	Turn(EAngle(0, m_flPreviewTurn, 0));

	float flMovePower = GetPreviewBaseTurnPower();

	if (CNetwork::IsHost())
	{
		m_flMovementPower += flMovePower;
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	m_flGoalTurretYaw = 0;

	DirtyNeedsOrders();

	DigitanksWindow()->GetHUD()->UpdateTurnButton();
}

void CDigitank::Turn(EAngle angNewTurn)
{
	m_flPreviousTurn = GetAngles().y;
	m_flStartedTurn = GameServer()->GetGameTime();
	SetAngles(angNewTurn);

	if (IsSentried())
		Sentry();
}

void CDigitank::SetGoalMovePosition(const Vector& vecPosition)
{
	if (IsFortified() && !CanMoveFortified())
		return;

	if (fabs(vecPosition.x) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (fabs(vecPosition.z) > DigitanksGame()->GetTerrain()->GetMapSize())
		return;

	if (CNetwork::IsHost())
	{
		CNetworkParameters p;
		p.fl2 = vecPosition.x;
		p.fl3 = vecPosition.y;
		p.fl4 = vecPosition.z;
		SetGoalMovePosition(&p);
	}
	else
		CNetwork::CallFunction(NETWORK_TOSERVER, "SetGoalMovePosition", GetHandle(), vecPosition.x, vecPosition.y, vecPosition.z);
}

void CDigitank::SetGoalMovePosition(CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	m_bGoalMovePosition = true;
	m_vecGoalMovePosition = Vector(p->fl2, p->fl3, p->fl4);

	MoveTowardsGoalMovePosition();
}

void CDigitank::MoveTowardsGoalMovePosition()
{
	if (!CNetwork::IsHost())
		return;

	CNetwork::SetRunningClientFunctions(false);

	Vector vecGoal = GetGoalMovePosition();
	Vector vecOrigin = GetOrigin();
	Vector vecMove = vecGoal - vecOrigin;

	Vector vecNewPosition = GetOrigin() + vecMove;

	do
	{
		vecMove = vecMove * 0.95f;

		if (vecMove.Length() < 1)
			break;

		vecNewPosition = GetOrigin() + vecMove;
		vecNewPosition.y = FindHoverHeight(vecNewPosition);

		SetPreviewMove(vecNewPosition);
	}
	while (!IsPreviewMoveValid());

	SetPreviewMove(vecNewPosition);
	Move();

	if ((GetRealOrigin() - GetGoalMovePosition()).Length2DSqr() < 1)
	{
		CancelGoalMovePosition();
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITAUTOMOVE);
		return;
	}
}

void CDigitank::CancelGoalMovePosition()
{
	if (CNetwork::IsHost())
		CancelGoalMovePosition(NULL);
	else
		CNetwork::CallFunction(NETWORK_TOSERVER, "CancelGoalMovePosition", GetHandle());
}

void CDigitank::CancelGoalMovePosition(CNetworkParameters* p)
{
	m_bGoalMovePosition = false;
}

void CDigitank::Fortify()
{
	if (!CanFortify())
		return;

	if (IsDisabled())
		return;

	CNetwork::CallFunction(NETWORK_TOEVERYONE, "Fortify", GetHandle());

	CNetworkParameters p;
	p.ui1 = GetHandle();
	Fortify(&p);
}

void CDigitank::Fortify(CNetworkParameters* p)
{
	m_flFortifyTime = GameServer()->GetGameTime();

	m_flMovementPower = GetMaxMovementEnergy();

	if (m_bFortified)
	{
		m_bFortified = false;
		DigitanksWindow()->GetHUD()->UpdateTurnButton();
		return;
	}

	if (IsSentried())
		Sentry();

	m_bFortified = true;

	m_iFortifyLevel = 0;

	OnFortify();

	DirtyNeedsOrders();

	DigitanksWindow()->GetHUD()->UpdateTurnButton();

	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_FORTIFYING);
	DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_DEPLOYING, true);
}

bool CDigitank::CanAim() const
{
	return AllowControlMode(MODE_AIM);
}

void CDigitank::Sentry()
{
	if (!CanSentry())
		return;

	if (IsDisabled())
		return;

	CNetwork::CallFunction(NETWORK_TOEVERYONE, "Sentry", GetHandle());

	CNetworkParameters p;
	p.ui1 = GetHandle();
	Sentry(&p);
}

void CDigitank::Sentry(CNetworkParameters* p)
{
	if (m_bSentried)
	{
		m_bSentried = false;
		DigitanksWindow()->GetHUD()->UpdateTurnButton();
		return;
	}

	m_bSentried = true;

	CancelGoalMovePosition();

	DirtyNeedsOrders();

	DigitanksWindow()->GetHUD()->UpdateTurnButton();
}

void CDigitank::Charge()
{
	if (IsFortified() && !CanTurnFortified())
		return;

	if (ChargeEnergy() > m_flTotalPower)
		return;

	if (m_bFiredWeapon || m_bActionTaken)
		return;

	if (m_hPreviewCharge == NULL)
		return;

	if (IsDisabled())
		return;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "Charge", GetHandle(), m_hPreviewCharge->GetHandle());

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = m_hPreviewCharge->GetHandle();
	Charge(&p);

	DigitanksGame()->SetControlMode(MODE_NONE);
}

void CDigitank::Charge(class CNetworkParameters* p)
{
	CEntityHandle<CBaseEntity> hTarget = p->ui2;

	m_hChargeTarget = hTarget;

	if (hTarget == NULL)
		return;

	Vector vecChargeDirection = (hTarget->GetOrigin() - GetOrigin()).Normalized();

	Turn(VectorAngles(vecChargeDirection));

	float flDistanceToTarget = hTarget->Distance(GetOrigin());
	if (flDistanceToTarget < 10)
	{
		Vector vecChargeStart = hTarget->GetOrigin() - vecChargeDirection * 20;
		vecChargeStart.y = FindHoverHeight(vecChargeStart);
		Move(vecChargeStart);
	}

	if (CNetwork::IsHost())
	{
		m_flTotalPower -= ChargeEnergy();
		m_flAttackPower += ChargeEnergy();
		m_flMovementPower += ChargeEnergy();
	}

	m_flGoalTurretYaw = -180;

	m_bActionTaken = true;

	DirtyNeedsOrders();

	m_flBeginCharge = GameServer()->GetGameTime() + GetTransitionTime();

	if (IsSentried())
		Sentry();

	size_t iInstance = CParticleSystemLibrary::AddInstance(L"charge-charge", GetOrigin());
	CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(iInstance);
	if (pInstance)
		pInstance->FollowEntity(this);
}

void CDigitank::Cloak()
{
	m_bCloaked = true;

	DigitanksWindow()->GetHUD()->Layout();
}

void CDigitank::Uncloak()
{
	m_bCloaked = false;

	DigitanksWindow()->GetHUD()->Layout();
}

float CDigitank::GetCloakConcealment() const
{
	if (IsCloaked())
		return 0.5f;

	return 0;
}

bool CDigitank::MovesWith(CDigitank* pOther) const
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	// Not fortified tanks.
	if (IsFortified() || IsFortifying())
		return false;

	// Only same class tanks.
	if (GetUnitType() != pOther->GetUnitType())
		return false;

	if (!GetDigitanksTeam()->IsSelected(this))
		return false;

	return true;
}

bool CDigitank::TurnsWith(CDigitank* pOther) const
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	if (!GetDigitanksTeam()->IsSelected(this))
		return false;

	// Only same class tanks.
	if (GetUnitType() != pOther->GetUnitType())
		return false;

	return true;
}

bool CDigitank::AimsWith(CDigitank* pOther) const
{
	if (!pOther)
		return false;

	if (this == pOther)
		return true;

	// Only same team.
	if (GetTeam() != pOther->GetTeam())
		return false;

	if (!GetDigitanksTeam()->IsSelected(this))
		return false;

	if (!CanAim())
		return false;

	if (!IsFortified() && !CanAimMobilized())
		return false;

	return true;
}

void CDigitank::Think()
{
	BaseClass::Think();

	if (GameServer()->GetGameTime() < m_flNextThink)
		return;

	m_flNextThink = GameServer()->GetGameTime() + 0.1f;

	if (!IsMoving() && IsAlive())
	{
		if (!DigitanksGame()->GetTerrain()->IsPointOnMap(GetOrigin()) || DigitanksGame()->GetTerrain()->IsPointOverHole(GetOrigin()))
			Kill();
		else
		{
			float flHoverHeight = FindHoverHeight(GetOrigin());
			if (fabs(GetOrigin().y - flHoverHeight) > 1.0f)
				Move(Vector(GetOrigin().x, flHoverHeight, GetOrigin().z));
		}
	}

	m_bDisplayAim = false;

	bool bAimMode = DigitanksGame()->GetControlMode() == MODE_AIM && DigitanksGame()->GetAimType() == AIM_NORMAL;
	bool bShowThisTank = m_bFiredWeapon;
	if (bAimMode && GetDigitanksTeam()->IsSelected(this) && AimsWith(GetDigitanksTeam()->GetPrimarySelectionTank()))
		bShowThisTank = true;
	if (GetDigitanksTeam() != DigitanksGame()->GetCurrentTeam())
		bShowThisTank = false;

	if (bShowThisTank)
	{
		Vector vecMouseAim;
		CBaseEntity* pHit = NULL;
		bool bMouseOK = DigitanksWindow()->GetMouseGridPosition(vecMouseAim, &pHit);

		Vector vecTankAim;
		if (m_bFiredWeapon)
			vecTankAim = m_vecLastAim.Get();

		if (bMouseOK && bAimMode)
		{
			if (AimsWith(GetDigitanksTeam()->GetPrimarySelectionTank()) && GetDigitanksTeam()->IsSelected(this))
			{
				if (pHit && dynamic_cast<CDigitanksEntity*>(pHit) && pHit->GetTeam() && pHit->GetTeam() != GetTeam())
					vecTankAim = pHit->GetOrigin();
				else
					vecTankAim = vecMouseAim;
			}
		}

		if (m_bFiredWeapon || bMouseOK)
		{
			while (!IsInsideMaxRange(vecTankAim))
			{
				Vector vecDirection = vecTankAim - GetOrigin();
				vecDirection.y = 0;
				vecTankAim = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + vecDirection.Normalized() * vecDirection.Length2D() * 0.99f);
			}

			if ((vecTankAim - GetOrigin()).Length() < GetMinRange())
			{
				vecTankAim = GetOrigin() + (vecTankAim - GetOrigin()).Normalized() * GetMinRange() * 1.01f;
				vecTankAim.y = DigitanksGame()->GetTerrain()->GetHeight(vecTankAim.x, vecTankAim.z);
			}

			if (fabs(AngleDifference(GetAngles().y, VectorAngles((vecTankAim-GetOrigin()).Normalized()).y)) > FiringCone())
				m_bDisplayAim = false;
			else
			{
				float flRadius = FindAimRadius(vecTankAim);
				DigitanksGame()->AddTankAim(vecTankAim, flRadius, GetDigitanksTeam()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM);

				m_bDisplayAim = true;
				m_vecDisplayAim = vecTankAim;
				m_flDisplayAimRadius = flRadius;
			}
		}
	}

	if (m_flFireWeaponTime && GameServer()->GetGameTime() > m_flFireWeaponTime)
	{
		m_iFireWeapons--;
		FireWeapon();

		if (m_iFireWeapons)
			m_flFireWeaponTime = GameServer()->GetGameTime() + CProjectile::GetWeaponFireInterval(GetCurrentWeapon());
		else
			m_flFireWeaponTime = 0;
	}

	if (m_iHoverParticles != ~0)
	{
		float flTransitionTime = GetTransitionTime();
		float flTimeSinceMove = GameServer()->GetGameTime() - m_flStartedMove;
		float flTimeSinceTurn = GameServer()->GetGameTime() - m_flStartedTurn;
		if (m_flStartedMove && flTimeSinceMove > flTransitionTime || m_flStartedTurn && flTimeSinceTurn > flTransitionTime)
		{
			CParticleSystemLibrary::StopInstance(m_iHoverParticles);
			m_iHoverParticles = ~0;
		}
	}

	if (IsAlive() && GameServer()->GetGameTime() > m_flNextIdle)
	{
		// A little bit less often if we're not on the current team.
		if (DigitanksGame()->GetCurrentTeam() == GetTeam() && rand()%2 == 0 || rand()%4 == 0)
		{
			if (DigitanksGame()->IsPartyMode())
				Speak(TANKSPEECH_PARTY);
			else if (IsDisabled())
				Speak(TANKSPEECH_DISABLED);
			else
				Speak(TANKSPEECH_IDLE);
		}

		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}

	if (m_flBeginCharge > 0 && GameServer()->GetGameTime() > m_flBeginCharge)
	{
		m_flBeginCharge = -1;

		if (m_hChargeTarget != NULL)
		{
			Move(GetChargePosition(m_hChargeTarget), 1);
			m_flEndCharge = GameServer()->GetGameTime() + GetTransitionTime();

			m_flGoalTurretYaw = -180;
		}
		else
			CParticleSystemLibrary::StopInstances(L"charge-charge");
	}

	if (m_flEndCharge > 0 && GameServer()->GetGameTime() > m_flEndCharge)
	{
		m_flEndCharge = -1;

		if (m_hChargeTarget != NULL)
		{
			m_hChargeTarget->TakeDamage(this, this, DAMAGE_COLLISION, ChargeDamage(), true);

			Vector vecPushDirection = (m_hChargeTarget->GetOrigin() - GetOrigin()).Normalized();

			CDigitank* pDigitank = dynamic_cast<CDigitank*>(m_hChargeTarget.GetPointer());
			if (pDigitank)
			{
				if (pDigitank->IsFortified())
				{
					TakeDamage(this, this, DAMAGE_COLLISION, ChargeDamage()/2, true);
					Move(GetOrigin() - vecPushDirection * ChargePushDistance()/2, 2);
					RockTheBoat(1, -vecPushDirection);
				}
				else
				{
					pDigitank->Move(pDigitank->GetOrigin() + vecPushDirection * ChargePushDistance(), 2);
					pDigitank->RockTheBoat(1, vecPushDirection);
				}

				Vector vecLookAt = (GetOrigin() - pDigitank->GetOrigin()).Normalized();
				pDigitank->m_flGoalTurretYaw = atan2(vecLookAt.z, vecLookAt.x) * 180/M_PI - pDigitank->GetRenderAngles().y;

				CParticleSystemLibrary::AddInstance(L"charge-burst", GetOrigin() + vecLookAt*3, GetAngles());
			}

			RockTheBoat(1, vecPushDirection);
			Turn(EAngle(0, GetAngles().y, 0));

			m_flGoalTurretYaw = 0;

			CParticleSystemLibrary::StopInstances(L"charge-charge");
		}
	}

	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceMove = GameServer()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove > flTransitionTime)
	{
		// We were moving but now we're done.
		DigitanksGame()->GetTerrain()->CalculateVisibility();

		m_flStartedMove = 0.0;
	}

	if (IsAlive())
	{
		bool bRunFire = GetHealth() < GetTotalHealth()/3;
		bool bRunSmoke = !bRunFire && GetHealth() <= GetTotalHealth()*4/5;

		if (m_iSmokeParticles != ~0)
		{
			if (GetVisibility() <= 0.1f || !bRunSmoke)
			{
				CParticleSystemLibrary::Get()->StopInstance(m_iSmokeParticles);
				m_iSmokeParticles = ~0;
			}

			CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_iSmokeParticles);
			if (!pInstance)
				m_iSmokeParticles = ~0;
		}
		else
		{
			if (GetVisibility() > 0 && bRunSmoke)
			{
				m_iSmokeParticles = CParticleSystemLibrary::AddInstance(L"digitank-smoke", GetOrigin());
				CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_iSmokeParticles);
				pInstance->FollowEntity(this);
			}
		}

		if (m_iFireParticles != ~0)
		{
			if (GetVisibility() <= 0.1f || !bRunFire)
			{
				CParticleSystemLibrary::Get()->StopInstance(m_iFireParticles);
				m_iFireParticles = ~0;
			}

			CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_iFireParticles);
			if (!pInstance)
				m_iFireParticles = ~0;
		}
		else
		{
			if (GetVisibility() > 0 && bRunFire)
			{
				m_iFireParticles = CParticleSystemLibrary::AddInstance(L"digitank-fire", GetOrigin());
				CSystemInstance* pInstance = CParticleSystemLibrary::Get()->GetInstance(m_iFireParticles);
				pInstance->FollowEntity(this);
			}
		}
	}
}

void CDigitank::OnCurrentSelection()
{
	BaseClass::OnCurrentSelection();

	if (GetDigitanksTeam() != DigitanksGame()->GetCurrentLocalDigitanksTeam())
	{
		DigitanksGame()->SetControlMode(MODE_NONE);
		return;
	}

	if (GetVisibility() > 0)
	{
		if (rand()%2 == 0)
			EmitSound(L"sound/tank-active.wav");
		else
			EmitSound(L"sound/tank-active2.wav");
	}

	Speak(TANKSPEECH_SELECTED);

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	// So the escape key works.
	if (DigitanksWindow()->GetInstructor()->GetCurrentTutorial() != CInstructor::TUTORIAL_THEEND_BASICS)
	{
		CDigitank* pClosestEnemy = NULL;
		while (true)
		{
			pClosestEnemy = CBaseEntity::FindClosest<CDigitank>(GetOrigin(), pClosestEnemy);

			if (!pClosestEnemy)
				break;

			if (pClosestEnemy->GetTeam() == GetTeam())
				continue;

			if ((pClosestEnemy->GetOrigin() - GetOrigin()).Length() > VisibleRange())
			{
				pClosestEnemy = NULL;
				break;
			}

			break;
		}

		DigitanksGame()->SetControlMode(MODE_NONE);
	}
}

bool CDigitank::AllowControlMode(controlmode_t eMode) const
{
	if (eMode == MODE_MOVE)
		return true;

	if (eMode == MODE_TURN)
		return true;

	if (eMode == MODE_AIM)
		return true;

	return BaseClass::AllowControlMode(eMode);
}

void CDigitank::OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode)
{
	if (eOldMode == MODE_MOVE)
		ClearPreviewMove();

	if (eOldMode == MODE_TURN)
		ClearPreviewTurn();

	if (eOldMode == MODE_AIM)
	{
		ClearPreviewAim();
		ClearPreviewCharge();
		DigitanksGame()->SetAimType(AIM_NONE);
	}

	if (eNewMode == MODE_AIM)
	{
		if (IsArtillery())
		{
			Vector vecCenter = DigitanksGame()->GetTerrain()->SetPointHeight(GetOrigin() + AngleVector(GetAngles()) * GetMaxRange()/2);
			DigitanksGame()->GetDigitanksCamera()->SetTarget(vecCenter);
		}
	}
}

float CDigitank::GetPowerBar1Value()
{
	return GetAttackPower(true) / m_flStartingPower;
}

float CDigitank::GetPowerBar2Value()
{
	return GetDefensePower(true) / m_flStartingPower;
}

float CDigitank::GetPowerBar3Value()
{
	return GetRemainingMovementEnergy(true) / m_flStartingPower;
}

float CDigitank::GetPowerBar1Size()
{
	float flPower = GetAttackPower(true) / GetTotalAttackPower();

	if (flPower > 1)
		return 1;

	if (flPower < 0)
		return 0;

	return flPower;
}

float CDigitank::GetPowerBar2Size()
{
	float flPower = GetDefensePower(true) / GetTotalDefensePower();

	if (flPower > 1)
		return 1;

	if (flPower < 0)
		return 0;

	return flPower;
}

float CDigitank::GetPowerBar3Size()
{
	float flPower = GetUsedMovementEnergy(true) / GetMaxMovementEnergy();

	if (flPower > 1)
		return 1;

	if (flPower < 0)
		return 0;

	return flPower;
}

void CDigitank::DirtyNeedsOrders()
{
	m_bNeedsOrdersDirty = true;
}

bool CDigitank::NeedsOrders()
{
	if (!m_bNeedsOrdersDirty)
		return m_bNeedsOrders;

	bool bNeedsToMove = true;
	if (GetUsedMovementEnergy() > 0)
		bNeedsToMove = false;
	else
	{
		if (IsFortified() || IsFortifying())
			bNeedsToMove = false;

		if (IsSentried())
			bNeedsToMove = false;
	}

	bool bNeedsToAttack = true;
	if (HasFiredWeapon())
		bNeedsToAttack = false;
	else if (!IsScout())
	{
		CDigitanksEntity* pClosestEnemy = NULL;
		while (true)
		{
			pClosestEnemy = CBaseEntity::FindClosest<CDigitanksEntity>(GetOrigin(), pClosestEnemy);

			if (pClosestEnemy)
			{
				if (!IsInsideMaxRange(pClosestEnemy->GetOrigin()))
				{
					pClosestEnemy = NULL;
					break;
				}

				if (pClosestEnemy->GetTeam() == GetTeam())
					continue;

				if (!pClosestEnemy->GetTeam())
					continue;

				if (pClosestEnemy->GetVisibility() == 0)
					continue;

				if (IsArtillery() && fabs(AngleDifference(GetAngles().y, VectorAngles((pClosestEnemy->GetOrigin()-GetOrigin()).Normalized()).y)) > FiringCone())
					continue;
			}
			break;
		}

		if (!pClosestEnemy)
			bNeedsToAttack = false;
	}
	else
	{
		// I'm a scout. BAM!
		CSupplyLine* pClosest = dynamic_cast<CScout*>(this)->FindClosestEnemySupplyLine(true);

		if (!pClosest)
			bNeedsToAttack = false;
	}

	if (IsArtillery() && !IsFortified())
		bNeedsToAttack = false;

	if (m_bFiredWeapon)
		bNeedsToMove = false;

	if (m_bActionTaken)
		bNeedsToMove = false;

	m_bNeedsOrders = bNeedsToMove || bNeedsToAttack;
	m_bNeedsOrdersDirty = false;

	return m_bNeedsOrders;
}

void CDigitank::SetupMenu(menumode_t eMenuMode)
{
	CHUD* pHUD = DigitanksWindow()->GetHUD();

	if (IsDisabled())
		return;

	if (eMenuMode == MENUMODE_MAIN)
	{
		if (HasGoalMovePosition())
		{
			pHUD->SetButtonListener(0, CHUD::CancelAutoMove);
			pHUD->SetButtonColor(0, Color(100, 100, 100));
			pHUD->SetButtonTexture(0, 192, 64);
			pHUD->SetButtonInfo(0, L"CANCEL AUTO MOVE\n \nCancel this unit's auto move command.\n \nShortcut: Q");
			pHUD->SetButtonTooltip(0, L"Cancel Auto-Move");
		}
		else if (!IsFortified() && !IsFortifying())
		{
			pHUD->SetButtonListener(0, CHUD::Move);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_MOVE)
				pHUD->SetButtonColor(0, Color(150, 150, 0));
			else
				pHUD->SetButtonColor(0, Color(100, 100, 100));

			if (DigitanksGame()->GetControlMode() == MODE_MOVE)
				pHUD->SetButtonTexture(0, 192, 64);
			else
				pHUD->SetButtonTexture(0, 128, 128);

			pHUD->SetButtonInfo(0, L"MOVE UNIT\n \nGo into Move mode. Click inside the yellow area to move this unit.\n \nShortcut: Q");
			pHUD->SetButtonTooltip(0, L"Move");
		}

		if (GetRemainingMovementEnergy() < 1)
		{
			pHUD->SetButtonListener(1, NULL);
			pHUD->SetButtonColor(1, Color(100, 100, 100));
			pHUD->SetButtonTexture(1, 64, 192);
			pHUD->SetButtonInfo(1, L"ROTATE UNIT\n \nGo into Rotate mode. Click any spot on the terrain to have this unit face that spot.\n \nNOT ENOUGH ENERGY\n \nShortcut: W");
			pHUD->SetButtonTooltip(1, L"Rotate");
		}
		else if (!IsScout() && (!IsFortified() && !IsFortifying() || CanTurnFortified()))
		{
			pHUD->SetButtonListener(1, CHUD::Turn);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_TURN)
				pHUD->SetButtonColor(1, Color(150, 150, 0));
			else
				pHUD->SetButtonColor(1, Color(100, 100, 100));

			if (DigitanksGame()->GetControlMode() == MODE_TURN)
				pHUD->SetButtonTexture(1, 192, 64);
			else
				pHUD->SetButtonTexture(1, 64, 192);
			pHUD->SetButtonInfo(1, L"ROTATE UNIT\n \nGo into Rotate mode. Click any spot on the terrain to have this unit face that spot.\n \nShortcut: W");
			pHUD->SetButtonTooltip(1, L"Rotate");
		}

		if (CanSentry() && !IsFortified() && DigitanksGame()->GetGameType() == GAMETYPE_STANDARD)
		{
			if (IsSentried())
			{
				pHUD->SetButtonTexture(5, 64, 128);
				pHUD->SetButtonInfo(5, L"MOBILIZE\n \nCancel the 'Hold Position' order.\n \nShortcut: A");
				pHUD->SetButtonTooltip(5, L"Mobilize");
			}
			else
			{
				pHUD->SetButtonTexture(5, 0, 192);
				pHUD->SetButtonInfo(5, L"HOLD POSITION\n \nThis unit will hold position and not require orders until told otherwise.\n \nShortcut: A");
				pHUD->SetButtonTooltip(5, L"Hold Position");
			}
			pHUD->SetButtonListener(5, CHUD::Sentry);
			pHUD->SetButtonColor(5, Color(0, 0, 150));
		}

		if (HasCloak() && !HasFiredWeapon())
		{
			if (IsCloaked())
			{
				pHUD->SetButtonInfo(6, L"DEACTIVATE CLOAKING DEVICE\n \nClick to turn off this tank's cloaking device.\n \nShortcut: S");
				pHUD->SetButtonTooltip(6, L"Uncloak");
				pHUD->SetButtonTexture(6, 320, 192);
			}
			else
			{
				pHUD->SetButtonInfo(6, L"ACTIVATE CLOAKING DEVICE\n \nThis tank has a cloaking device available. Click to activate it.\n \nShortcut: S");
				pHUD->SetButtonTooltip(6, L"Cloak");
				pHUD->SetButtonTexture(6, 256, 192);
			}
			pHUD->SetButtonListener(6, CHUD::Cloak);
			pHUD->SetButtonColor(6, Color(0, 0, 150));
		}

		if (GetNumWeapons() > 1)
		{
			pHUD->SetButtonTexture(7, 256, 64);
			pHUD->SetButtonInfo(7, L"CHOOSE WEAPON\n \nThis tank has multiple weapons available. Click to choose a weapon.\n \nShortcut: D");
			pHUD->SetButtonTooltip(7, L"Choose Weapon");
			pHUD->SetButtonListener(7, CHUD::ChooseWeapon);
			pHUD->SetButtonColor(7, Color(100, 100, 100));

			if (HasFiredWeapon())
			{
				pHUD->SetButtonColor(7, glgui::g_clrBox);
				pHUD->SetButtonListener(7, NULL);
			}
		}

		if (CanFortify())
		{
			if (IsFortified() || IsFortifying())
			{
				pHUD->SetButtonTexture(8, 64, 128);
				pHUD->SetButtonInfo(8, L"MOBILIZE\n \nAllows this unit to move again.\n \nShortcut: F");
				pHUD->SetButtonTooltip(8, L"Mobilize");
			}
			else
			{
				if (IsMobileCPU())
				{
					pHUD->SetButtonTexture(8, 384, 192);
					pHUD->SetButtonInfo(8, L"DEPLOY UNIT\n \nHave this MCP deploy and create a CPU. The CPU will be the center of operations for your base. This action cannot be undone.\n \nShortcut: F");
					pHUD->SetButtonTooltip(8, L"Deploy");
				}
				else if (IsArtillery())
				{
					pHUD->SetButtonTexture(8, 320, 64);
					pHUD->SetButtonInfo(8, L"DEPLOY UNIT\n \nHave this artillery deploy. Artillery must be deployed before they can be fired.\n \nShortcut: F");
					pHUD->SetButtonTooltip(8, L"Deploy");
				}
				else
				{
					pHUD->SetButtonTexture(8, 0, 128);
					pHUD->SetButtonInfo(8, L"FORTIFY UNIT\n \nHave this unit fortify his position mode. Offers combat bonuses that accumulate over the next few turns.\n \nShortcut: F");
					pHUD->SetButtonTooltip(8, L"Fortify");
				}
			}
			pHUD->SetButtonListener(8, CHUD::Fortify);
			pHUD->SetButtonColor(8, Color(0, 0, 150));
		}

		if ((CanAimMobilized() || IsFortified()) && !m_bFiredWeapon && !m_bActionTaken && m_eWeapon != WEAPON_NONE)
		{
			pHUD->SetButtonListener(2, CHUD::Aim);

			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonColor(2, Color(150, 0, 0));
			else
				pHUD->SetButtonColor(2, Color(100, 100, 100));

			pHUD->SetButtonTexture(2, 0, 0);

			if (DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonTexture(2, 192, 64);

			eastl::string16 s;
			if (IsInfantry())
				s += L"AIM AND FIRE MOUNTED GUN\n \nClick to enter Aim mode. Click any spot on the terrain to fire on that location.";
			else if (IsScout())
				s += L"AIM AND FIRE TORPEDO\n \nClick to enter Aim mode. Click any spot on the terrain to fire on that location.\n \nTorpedos contain a special EMP that can disable enemy tanks and sever support lines. It can't hit other Rogues and only does physical damage to structures or units without shields.";
			else
				s += L"AIM AND FIRE CANON\n \nClick to enter Aim mode. Click any spot on the terrain to fire on that location.";

			if (m_flTotalPower < GetWeaponEnergy())
			{
				pHUD->SetButtonColor(2, glgui::g_clrBox);
				s += L"\n \nNOT ENOUGH ENERGY";

				pHUD->SetButtonListener(2, NULL);
			}

			s += L"\n \nShortcut: E";

			pHUD->SetButtonInfo(2, s);
			pHUD->SetButtonTooltip(2, L"Aim");
		}

		if (HasBonusPoints())
		{
			pHUD->SetButtonListener(4, CHUD::Promote);
			pHUD->SetButtonTexture(4, 192, 128);
			pHUD->SetButtonColor(4, glgui::g_clrBox);
			pHUD->SetButtonInfo(4, L"UPGRADE UNIT\n \nThis unit has upgrades available. Click to open the upgrades menu.\n \nShortcut: T");
			pHUD->SetButtonTooltip(4, L"Upgrade");
		}

		if (m_iAirstrikes && !m_bActionTaken)
		{
			if (!DigitanksGame()->GetControlMode() || DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonColor(9, Color(150, 0, 0));
			else
				pHUD->SetButtonColor(9, Color(100, 100, 100));

			if (DigitanksGame()->GetControlMode() == MODE_AIM)
				pHUD->SetButtonTexture(9, 192, 64);
			else
				pHUD->SetButtonTexture(9, 448, 192);

			pHUD->SetButtonListener(9, CHUD::FireSpecial);
			pHUD->SetButtonInfo(9, L"CALL AN AIRSTRIKE\n \nThis unit can call an airstrike. Click to aim and fire the airstrike.\n \nShortcut: G");
			pHUD->SetButtonTooltip(9, L"Airstrike");
		}
	}
	else if (eMenuMode == MENUMODE_PROMOTE)
	{
		if (!IsScout())
		{
			pHUD->SetButtonListener(0, CHUD::PromoteAttack);
			pHUD->SetButtonTexture(0, 256, 128);
			pHUD->SetButtonColor(0, Color(150, 150, 150));

			eastl::string16 s1;
			s1 += L"UPGRADE ATTACK ENERGY\n \n";
			s1 += L"This upgrade amplifies your tank's arsenal, increasing the maximum Attack Energy available to your tank past its normal levels. With greater Attack Energy, this tank's shells will deal more damage.\n \n";
			s1 += L"Attack Energy increase: 10%\n \n";
			s1 += L"Shortcut: Q";
			pHUD->SetButtonInfo(0, s1);
			pHUD->SetButtonTooltip(0, L"Upgrade Attack");
		}

		if (!IsArtillery() && !IsScout())
		{
			pHUD->SetButtonListener(1, CHUD::PromoteDefense);
			pHUD->SetButtonTexture(1, 320, 128);
			pHUD->SetButtonColor(1, Color(150, 150, 150));

			eastl::string16 s;
			s += L"UPGRADE SHIELD ENERGY\n \n";
			s += L"This upgrade strengthens your tank's shield generator, increasing the maximum Shield Energy available to your tank past its normal levels. As a result, your tank's shields will take more damage before they fail.\n \n";
			s += L"Shield Energy increase: 10%\n \n";
			s += L"Shortcut: W";
			pHUD->SetButtonInfo(1, s);
			pHUD->SetButtonTooltip(1, L"Upgrade Shields");
		}

		pHUD->SetButtonListener(2, CHUD::PromoteMovement);
		pHUD->SetButtonTexture(2, 384, 128);
		pHUD->SetButtonColor(2, Color(150, 150, 150));

		eastl::string16 s2;
		s2 += L"UPGRADE MOVEMENT ENERGY\n \n";
		s2 += L"This upgrade overclocks your tank's engines, increasing the maximum Movement Energy available to your tank past its normal levels. With this you'll spend less energy moving your tank around.\n \n";
		s2 += L"Movement Energy increase: 10%\n \n";
		s2 += L"Shortcut: E";
		pHUD->SetButtonInfo(2, s2);
		pHUD->SetButtonTooltip(2, L"Upgrade Movement");

		pHUD->SetButtonListener(9, CHUD::GoToMain);
		pHUD->SetButtonTexture(9, 192, 64);
		pHUD->SetButtonColor(9, Color(100, 0, 0));
		pHUD->SetButtonInfo(9, L"RETURN\n \nShortcut: G");
		pHUD->SetButtonTooltip(9, L"Return");
	}
}

void CDigitank::Fire()
{
	float flDistanceSqr = (m_vecPreviewAim - GetOrigin()).LengthSqr();
	if (!IsInsideMaxRange(m_vecPreviewAim))
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	if (fabs(AngleDifference(GetAngles().y, VectorAngles((GetPreviewAim()-GetOrigin()).Normalized()).y)) > FiringCone())
		return;

	if (m_bFiredWeapon || m_bActionTaken)
		return;

	if (m_flTotalPower < GetWeaponEnergy())
		return;

	if (CanFortify() && !IsFortified() && !CanAimMobilized())
		return;

	if (IsDisabled())
		return;

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOEVERYONE, "Fire", GetHandle(), m_vecPreviewAim.x, m_vecPreviewAim.y, m_vecPreviewAim.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.fl2 = m_vecPreviewAim.x;
	p.fl3 = m_vecPreviewAim.y;
	p.fl4 = m_vecPreviewAim.z;
	Fire(&p);
}

void CDigitank::Fire(CNetworkParameters* p)
{
	m_vecPreviewAim = Vector(p->fl2, p->fl3, p->fl4);

	m_vecLastAim = m_vecPreviewAim;
	m_bFiredWeapon = true;

	float flAttackPower = GetWeaponEnergy();

	if (CNetwork::IsHost())
	{
		m_flTotalPower -= flAttackPower;
		m_flAttackPower += flAttackPower;
	}

	if (GetVisibility() > 0)
	{
		EmitSound(L"sound/tank-aim.wav");
		SetSoundVolume(L"sound/tank-aim.wav", 0.5f);
	}

	if (CNetwork::IsHost())
	{
		if (IsMoving())
			m_flFireWeaponTime += m_flStartedMove + GetTransitionTime() + FirstProjectileTime();
		else
			m_flFireWeaponTime = GameServer()->GetGameTime() + FirstProjectileTime();
		m_iFireWeapons = CProjectile::GetWeaponShells(GetCurrentWeapon());

		if (GetCurrentWeapon() == PROJECTILE_CAMERAGUIDED)
			m_flFireWeaponTime = GameServer()->GetGameTime();

		m_bLostConcealment = true;
		m_bCloaked = false;
	}

	Speak(TANKSPEECH_ATTACK);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	DirtyNeedsOrders();

	DigitanksWindow()->GetHUD()->UpdateTurnButton();

	if (IsArtillery())
		DigitanksWindow()->GetInstructor()->FinishedTutorial(CInstructor::TUTORIAL_FIRE_ARTILLERY, true);
}

void CDigitank::FireWeapon()
{
	float flDistanceSqr = (m_vecLastAim.Get() - GetOrigin()).LengthSqr();
	if (!IsInsideMaxRange(m_vecLastAim.Get()))
		return;

	if (flDistanceSqr < GetMinRange()*GetMinRange())
		return;

	Vector vecLandingSpot = m_vecLastAim.Get();

	float flFactor = FindAimRadius(m_vecLastAim.Get(), MinRangeRadius());

	float flYaw = RandomFloat(0, 360);
	float flRadius = RandomFloat(1, flFactor);

	// Don't use uniform distribution, I like how it's clustered on the target.
	vecLandingSpot += Vector(flRadius*cos(flYaw), 0, flRadius*sin(flYaw));

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	m_hWeapon = CreateWeapon();

	if (m_hWeapon == NULL)
		return;

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(m_hWeapon.GetPointer());
	if (pProjectile)
		DigitanksGame()->AddProjectileToWaitFor();

	if (CNetwork::ShouldReplicateClientFunction())
		CNetwork::CallFunction(NETWORK_TOCLIENTS, "FireWeapon", GetHandle(), m_hWeapon->GetHandle(), vecLandingSpot.x, vecLandingSpot.y, vecLandingSpot.z);

	CNetworkParameters p;
	p.ui1 = GetHandle();
	p.ui2 = m_hWeapon->GetHandle();
	p.fl3 = vecLandingSpot.x;
	p.fl4 = vecLandingSpot.y;
	p.fl5 = vecLandingSpot.z;
	FireWeapon(&p);
}

void CDigitank::FireWeapon(CNetworkParameters* p)
{
	m_hWeapon = CEntityHandle<CBaseWeapon>(p->ui2);

	Vector vecLandingSpot = Vector(p->fl3, p->fl4, p->fl5);

	m_hWeapon->SetOwner(this);

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(m_hWeapon.GetPointer());
	if (pProjectile)
		FireProjectile(pProjectile, vecLandingSpot);

	if (GetVisibility() > 0)
	{
		EmitSound(L"sound/tank-fire.wav");
	}

	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::FireProjectile(CProjectile* pProjectile, Vector vecLandingSpot)
{
	float flGravity = DigitanksGame()->GetGravity();
	float flTime;
	Vector vecForce;
	FindLaunchVelocity(GetOrigin(), vecLandingSpot, flGravity, vecForce, flTime, ProjectileCurve());

	pProjectile->SetVelocity(vecForce);
	pProjectile->SetGravity(Vector(0, flGravity, 0));
	pProjectile->SetLandingSpot(vecLandingSpot);

	if (GetVisibility() > 0)
	{
		Vector vecMuzzle = (vecLandingSpot - GetOrigin()).Normalized() * 3 + Vector(0, 3, 0);
		size_t iFire = CParticleSystemLibrary::AddInstance(L"tank-fire", GetOrigin() + vecMuzzle);
		if (iFire != ~0)
			CParticleSystemLibrary::Get()->GetInstance(iFire)->SetInheritedVelocity(vecForce);
	}

	RockTheBoat(0.8f, -vecForce.Normalized());
}

CBaseWeapon* CDigitank::CreateWeapon()
{
	if (GetCurrentWeapon() == PROJECTILE_ARTILLERY)
		return GameServer()->Create<CArtilleryShell>("CArtilleryShell");
	else if (GetCurrentWeapon() == PROJECTILE_ARTILLERY_AOE)
		return GameServer()->Create<CArtilleryAoE>("CArtilleryAoE");
	else if (GetCurrentWeapon() == PROJECTILE_ARTILLERY_ICBM)
		return GameServer()->Create<CArtilleryICBM>("CArtilleryICBM");
	else if (GetCurrentWeapon() == PROJECTILE_DEVASTATOR)
		return GameServer()->Create<CDevastator>("CDevastator");
	else if (GetCurrentWeapon() == PROJECTILE_FLAK)
		return GameServer()->Create<CInfantryFlak>("CInfantryFlak");
	else if (GetCurrentWeapon() == PROJECTILE_TREECUTTER)
		return GameServer()->Create<CTreeCutter>("CTreeCutter");
	else if (GetCurrentWeapon() == WEAPON_INFANTRYLASER)
		return GameServer()->Create<CInfantryLaser>("CInfantryLaser");
	else if (GetCurrentWeapon() == PROJECTILE_TORPEDO)
		return GameServer()->Create<CTorpedo>("CTorpedo");
	else if (GetCurrentWeapon() == PROJECTILE_SMALL)
		return GameServer()->Create<CSmallShell>("CSmallShell");
	else if (GetCurrentWeapon() == PROJECTILE_MEDIUM)
		return GameServer()->Create<CMediumShell>("CMediumShell");
	else if (GetCurrentWeapon() == PROJECTILE_LARGE)
		return GameServer()->Create<CLargeShell>("CLargeShell");
	else if (GetCurrentWeapon() == PROJECTILE_AOE)
		return GameServer()->Create<CAOEShell>("CAOEShell");
	else if (GetCurrentWeapon() == PROJECTILE_TRACTORBOMB)
		return GameServer()->Create<CTractorBomb>("CTractorBomb");
	else if (GetCurrentWeapon() == PROJECTILE_SPLOOGE)
		return GameServer()->Create<CSploogeShell>("CSploogeShell");
	else if (GetCurrentWeapon() == PROJECTILE_ICBM)
		return GameServer()->Create<CICBM>("CICBM");
	else if (GetCurrentWeapon() == PROJECTILE_EMP)
		return GameServer()->Create<CEMP>("CEMP");
	else if (GetCurrentWeapon() == PROJECTILE_GRENADE)
		return GameServer()->Create<CGrenade>("CGrenade");
	else if (GetCurrentWeapon() == PROJECTILE_EARTHSHAKER)
		return GameServer()->Create<CEarthshaker>("CEarthshaker");
	else if (GetCurrentWeapon() == PROJECTILE_CAMERAGUIDED)
		return GameServer()->Create<CCameraGuidedMissile>("CCameraGuidedMissile");
	else if (GetCurrentWeapon() == PROJECTILE_DAISYCHAIN)
		return GameServer()->Create<CDaisyChain>("CDaisyChain");
	else if (GetCurrentWeapon() == PROJECTILE_CLUSTERBOMB)
		return GameServer()->Create<CClusterBomb>("CClusterBomb");
	else if (GetCurrentWeapon() == WEAPON_LASER)
		return GameServer()->Create<CLaser>("CLaser");
	else if (GetCurrentWeapon() == WEAPON_CHARGERAM)
		return NULL;

	assert(!"Unrecognized projectile");
	return GameServer()->Create<CSmallShell>("CSmallShell");
}

float CDigitank::GetWeaponEnergy() const
{
	return CProjectile::GetWeaponEnergy(m_eWeapon);
}

void CDigitank::FireSpecial()
{
	if (m_iAirstrikes <= 0)
		return;

	if (m_bActionTaken)
		return;

	m_iAirstrikes--;

	DigitanksGame()->BeginAirstrike(GetPreviewAim());

	m_bActionTaken = true;

	DirtyNeedsOrders();

	DigitanksGame()->SetControlMode(MODE_NONE);
}

bool CDigitank::HasSpecialWeapons()
{
	return m_iAirstrikes > 0;
}

void CDigitank::FireMissileDefense(CProjectile* pTarget)
{
	if (m_iMissileDefenses <= 0)
		return;

	if (!pTarget)
		return;

	if (GameServer()->GetGameTime() < m_flNextMissileDefense)
		return;

	m_iMissileDefenses--;

	CMissileDefense* pMissileDefense = GameServer()->Create<CMissileDefense>("CMissileDefense");
	pMissileDefense->SetOwner(this);
	pMissileDefense->SetTarget(pTarget);

	// Can't fire more than one at a time. This way the missile defenses can be overloaded with multiple projectiles.
	m_flNextMissileDefense = GameServer()->GetGameTime() + 1.0f;
}

bool CDigitank::CanFireMissileDefense()
{
	if (GameServer()->GetGameTime() < m_flNextMissileDefense)
		return false;

	return m_iMissileDefenses > 0;
}

void CDigitank::ClientUpdate(int iClient)
{
	BaseClass::ClientUpdate(iClient);
}

void CDigitank::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit)
{
	if (flDamage > 0)
	{
		CancelGoalMovePosition();
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_AUTOMOVECANCELED);
	}
	else
		DigitanksGame()->AddActionItem(this, ACTIONTYPE_UNITDAMAGED);

	size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_FINISHHIM)
	{
		// BOT MUST DIE
		if (GetDigitanksTeam() != DigitanksGame()->GetCurrentLocalDigitanksTeam())
			flDamage += 500;
	}

	if (pInflictor)
	{
		Vector vecLookAt = (pInflictor->GetOrigin() - GetOrigin()).Normalized();
		m_flGoalTurretYaw = atan2(vecLookAt.z, vecLookAt.x) * 180/M_PI - GetRenderAngles().y;
	}

	Speak(TANKSPEECH_DAMAGED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	if (IsSentried())
		Sentry();

	size_t iDifficulty = DigitanksGame()->GetDifficulty();

	CProjectile* pProjectile = dynamic_cast<CProjectile*>(pInflictor);

	if (!CNetwork::IsConnected() && iDifficulty == 0)
	{
		if (DigitanksGame()->IsTeamControlledByMe(GetTeam()))
			flDamage *= 0.5f;
		else if (dynamic_cast<CDigitank*>(pAttacker) && DigitanksGame()->IsTeamControlledByMe(dynamic_cast<CDigitank*>(pAttacker)->GetTeam()))
			flDamage *= 2.0f;
	}

	Vector vecAttackOrigin;
	if (bDirectHit)
	{
		if (pAttacker)
			vecAttackOrigin = pAttacker->GetOrigin();
	}
	else if (pInflictor)
		vecAttackOrigin = pInflictor->GetOrigin();

	float flShield = GetShieldValue();

	float flDamageBlocked = flShield * GetDefenseScale();

	// Lava burns bypass shields
	if (eDamageType == DAMAGE_BURN)
		flDamageBlocked = 0;
	else if (eDamageType == DAMAGE_COLLISION)
	{
		// Looks like a charge to me. Charges bypass shields.
		flDamageBlocked = 0;

		// Force the tank damage sound, suppress the shield damage sound.
		flShield = 0;
	}

	float flShieldDamageScale = 1;
	if (pProjectile)
		flShieldDamageScale = pProjectile->ShieldDamageScale();

	if (flDamage*flShieldDamageScale - flDamageBlocked < 0)
	{
		SetShieldValue(flShield - flDamage*flShieldDamageScale / GetDefenseScale());

		if (GetVisibility() > 0)
		{
			EmitSound(L"sound/shield-damage.wav");
			SetSoundVolume(L"sound/shield-damage.wav", RemapValClamped(flDamage*flShieldDamageScale, 0, 50, 0, 0.5f));
		}

		DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamage*flShieldDamageScale, bDirectHit, true);

		m_flShieldPulse = GameServer()->GetGameTime();

		return;
	}

	if (flShieldDamageScale > 0)
		flDamage -= flDamageBlocked/flShieldDamageScale;
	else
		flDamage -= flDamageBlocked;

	if (pProjectile)
		flDamage *= pProjectile->HealthDamageScale();

	if (GetVisibility() > 0)
	{
		EmitSound(L"sound/tank-damage.wav");
		SetSoundVolume(L"sound/tank-damage.wav", RemapValClamped(flDamage, 0, 50, 0, 1));
	}

	if (flShield > 1.0f)
	{
		if (GetVisibility() > 0)
		{
			EmitSound(L"sound/shield-damage.wav");
			SetSoundVolume(L"sound/shield-damage.wav", RemapValClamped(flShield, 0, 50, 0, 1));
		}
	}

	if (eDamageType != DAMAGE_BURN)
		SetShieldValue(0);

	DigitanksGame()->OnTakeShieldDamage(this, pAttacker, pInflictor, flDamageBlocked, bDirectHit, false);

	BaseClass::TakeDamage(pAttacker, pInflictor, eDamageType, flDamage, bDirectHit);
}

void CDigitank::OnKilled(CBaseEntity* pKilledBy)
{
	size_t iTutorial = DigitanksWindow()->GetInstructor()->GetCurrentTutorial();
	if (iTutorial == CInstructor::TUTORIAL_FINISHHIM)
		DigitanksWindow()->GetInstructor()->NextTutorial();

	CDigitank* pKiller = dynamic_cast<CDigitank*>(pKilledBy);

	if (pKiller)
	{
		pKiller->GiveBonusPoints(1);
		pKiller->Speak(TANKSPEECH_KILL);
		pKiller->m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}

	if (m_hSupplyLine != NULL)
		m_hSupplyLine->Delete();

	// Make sure we can see that we got a promotion.
	DigitanksWindow()->GetHUD()->Layout();

	if (m_hFortifyDefending != NULL)
		m_hFortifyDefending->RemoveDefender(this);
}

Vector CDigitank::GetOrigin() const
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceMove = GameServer()->GetGameTime() - m_flStartedMove;
	if (m_flStartedMove && flTimeSinceMove < flTransitionTime)
	{
		float flLerp = 0;
		float flRamp = RemapVal(flTimeSinceMove, 0, flTransitionTime, 0, 1);
		if (m_iMoveType == 0)
			flLerp = SLerp(flRamp, 0.2f);
		else if (m_iMoveType == 1)
			flLerp = Lerp(flRamp, 0.2f);
		else
			flLerp = Lerp(flRamp, 0.8f);

		Vector vecNewOrigin = m_vecPreviousOrigin.Get() * (1-flLerp) + BaseClass::GetOrigin() * flLerp;

		float flHoverHeight = FindHoverHeight(vecNewOrigin);
		if (vecNewOrigin.y < flHoverHeight)
			vecNewOrigin.y = flHoverHeight;

		return vecNewOrigin;
	}

	return BaseClass::GetOrigin();
}

Vector CDigitank::GetRealOrigin() const
{
	return BaseClass::GetOrigin();
}

EAngle CDigitank::GetAngles() const
{
	float flTransitionTime = GetTransitionTime();

	if (GetVisibility() == 0)
		flTransitionTime = 0;

	float flTimeSinceTurn = GameServer()->GetGameTime() - m_flStartedTurn;
	if (m_flStartedTurn && flTimeSinceTurn < flTransitionTime)
	{
		float flLerp = SLerp(RemapVal(flTimeSinceTurn, 0, flTransitionTime, 0, 1), 0.2f);
		float flAngleDiff = AngleDifference(BaseClass::GetAngles().y, m_flPreviousTurn.Get());
		float flNewTurn = m_flPreviousTurn.Get() + flAngleDiff * flLerp;
		return EAngle(0, flNewTurn, 0);
	}

	return BaseClass::GetAngles();
}

void CDigitank::PreRender(bool bTransparent)
{
	BaseClass::PreRender(bTransparent);

	if (bTransparent && DigitanksGame()->GetGameType() == GAMETYPE_STANDARD && GetTeam() && CSupplier::GetDataFlow(GetOrigin(), GetTeam()) > 0)
	{
		CRenderer* pRenderer = GameServer()->GetRenderer();

		Vector vecForward, vecRight, vecUp;
		pRenderer->GetCameraVectors(&vecForward, &vecRight, &vecUp);

		CRenderingContext c(pRenderer);

		Vector vecOrigin = GetRenderOrigin();
		Vector vecParticleUp, vecParticleRight;
		float flRadius = GetBoundingRadius()*2;
		m_flGlowYaw += fmod(GameServer()->GetFrameTime()*10, 360);

		c.BindTexture(s_iSupportGlow);
		c.SetAlpha(0.5f);
		c.SetDepthMask(false);
		c.SetDepthTest(false);
		c.SetBlend(BLEND_ADDITIVE);
		c.SetColor(GetTeam()->GetColor());
		c.BeginRenderQuads();

			float flYaw = m_flGlowYaw*M_PI/180;
			float flSin = sin(flYaw);
			float flCos = cos(flYaw);

			vecParticleUp = (flCos*vecUp + flSin*vecRight)*flRadius;
			vecParticleRight = (flCos*vecRight - flSin*vecUp)*flRadius;

			Vector vecTL = vecOrigin - vecParticleRight + vecParticleUp;
			Vector vecTR = vecOrigin + vecParticleRight + vecParticleUp;
			Vector vecBL = vecOrigin - vecParticleRight - vecParticleUp;
			Vector vecBR = vecOrigin + vecParticleRight - vecParticleUp;

			c.TexCoord(0, 1);
			c.Vertex(vecTL);
			c.TexCoord(0, 0);
			c.Vertex(vecBL);
			c.TexCoord(1, 0);
			c.Vertex(vecBR);
			c.TexCoord(1, 1);
			c.Vertex(vecTR);

			// Do it again rotating the other way.
			flYaw = -m_flGlowYaw*M_PI/180;
			flSin = sin(flYaw);
			flCos = cos(flYaw);

			vecParticleUp = (flCos*vecUp + flSin*vecRight)*flRadius;
			vecParticleRight = (flCos*vecRight - flSin*vecUp)*flRadius;

			vecTL = vecOrigin - vecParticleRight + vecParticleUp;
			vecTR = vecOrigin + vecParticleRight + vecParticleUp;
			vecBL = vecOrigin - vecParticleRight - vecParticleUp;
			vecBR = vecOrigin + vecParticleRight - vecParticleUp;
			
			c.TexCoord(0, 1);
			c.Vertex(vecTL);
			c.TexCoord(0, 0);
			c.Vertex(vecBL);
			c.TexCoord(1, 0);
			c.Vertex(vecBR);
			c.TexCoord(1, 1);
			c.Vertex(vecTR);
			
		c.EndRender();
	}
}

Vector CDigitank::GetRenderOrigin() const
{
	float flLerp = 0;
	float flHoverHeight = 0;
	
	if (!IsFortified() && !IsFortifying())
	{
		float flOscillate = Oscillate(GameServer()->GetGameTime()+m_flBobOffset, 4);
		flLerp = SLerp(flOscillate, 0.2f);
		flHoverHeight = 1 + flLerp*BobHeight();
	}

	Vector vecChargeShake = Vector(0, 0, 0);
	Vector vecMaxChargeShake = Vector(0, 0, 0);
	if (m_flBeginCharge > 0 || m_flEndCharge > 0)
		vecMaxChargeShake = Vector(RandomFloat(-0.2f, 0.2f), RandomFloat(-0.2f, 0.2f), RandomFloat(-0.2f, 0.2f));

	if (m_flBeginCharge > 0)
		vecChargeShake = vecMaxChargeShake * RemapValClamped(GameServer()->GetGameTime(), m_flBeginCharge - GetTransitionTime(), m_flBeginCharge, 0, 1);

	if (m_flEndCharge > 0)
		vecChargeShake = vecMaxChargeShake;

	return GetOrigin() + Vector(0, flHoverHeight, 0) + vecChargeShake;
}

EAngle CDigitank::GetRenderAngles() const
{
	if (GetDigitanksTeam()->IsPrimarySelection(this))
	{
		if (DigitanksGame()->GetControlMode() == MODE_TURN && GetPreviewTurnPower() <= GetRemainingMovementEnergy())
			return EAngle(0, GetPreviewTurn(), 0);
	}

	if (DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		if (TurnsWith(DigitanksGame()->GetPrimarySelectionTank()))
		{
			Vector vecLookAt;
			bool bMouseOK = DigitanksWindow()->GetMouseGridPosition(vecLookAt);
			bool bNoTurn = bMouseOK && (vecLookAt - DigitanksGame()->GetPrimarySelectionTank()->GetOrigin()).LengthSqr() < 3*3;

			if (!bNoTurn && bMouseOK)
			{
				Vector vecDirection = (vecLookAt - GetOrigin()).Normalized();
				float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

				float flTankTurn = AngleDifference(flYaw, GetAngles().y);
				if (fabs(flTankTurn)/TurnPerPower() > GetRemainingMovementEnergy())
					flTankTurn = (flTankTurn / fabs(flTankTurn)) * GetRemainingMovementEnergy() * TurnPerPower() * 0.95f;

				return EAngle(0, GetAngles().y + flTankTurn, 0);
			}
		}
	}

	if (IsRocking())
	{
		EAngle angReturn;
		angReturn.y = GetAngles().y;

		Vector vecForward, vecRight;
		AngleVectors(GetAngles(), &vecForward, &vecRight, NULL);
		float flDotForward = -m_vecRockDirection.Dot(vecForward.Normalized());
		float flDotRight = -m_vecRockDirection.Dot(vecRight.Normalized());

		float flLerp = Lerp(1-Oscillate(GameServer()->GetGameTime() - m_flStartedRock, 1), 0.7f);

		angReturn.p = flDotForward*flLerp*m_flRockIntensity*45;
		angReturn.r = flDotRight*flLerp*m_flRockIntensity*45;

		return angReturn;
	}

	return GetAngles();
}

void CDigitank::ModifyContext(CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::ModifyContext(pContext, bTransparent);

	if (!GetTeam())
		return;

	pContext->SetColorSwap(GetTeam()->GetColor());
}

void CDigitank::OnRender(class CRenderingContext* pContext, bool bTransparent)
{
	BaseClass::OnRender(pContext, bTransparent);

	RenderTurret(bTransparent);

	if (bTransparent)
	{
		if (GetShieldStrength() > 0 && !IsFortified() && !IsFortifying())
			RenderShield(GetShieldStrength());
	}
}

void CDigitank::RenderTurret(bool bTransparent, float flAlpha)
{
	if (m_iTurretModel == ~0)
		return;

	if (GetVisibility() == 0 || flAlpha == 0)
		return;

	float flVisibility = flAlpha*GetVisibility();

	if (bTransparent && flVisibility == 1)
		return;

	if (!bTransparent && flVisibility < 1)
		return;

	CRenderingContext r(GameServer()->GetRenderer());

	if (bTransparent && flVisibility < 1)
	{
		r.SetAlpha(flVisibility);
		r.SetBlend(BLEND_ALPHA);
	}

	r.Translate(Vector(-0.0f, 0.810368f, 0));

	if ((GetDigitanksTeam()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM) || m_bFiredWeapon)
	{
		Vector vecAimTarget;
		if (GetDigitanksTeam()->IsSelected(this) && DigitanksGame()->GetControlMode() == MODE_AIM)
			vecAimTarget = GetPreviewAim();
		else
			vecAimTarget = m_vecLastAim.Get();
		Vector vecTarget = (vecAimTarget - GetRenderOrigin()).Normalized();
		m_flGoalTurretYaw = atan2(vecTarget.z, vecTarget.x) * 180/M_PI - GetRenderAngles().y;
	}

	float flSpeed = RemapValClamped(fabs(AngleDifference(m_flGoalTurretYaw, m_flCurrentTurretYaw)), 30, 90, 20, 40);
	m_flCurrentTurretYaw = AngleApproach(m_flGoalTurretYaw, m_flCurrentTurretYaw, flSpeed);

	r.Rotate(-m_flCurrentTurretYaw, Vector(0, 1, 0));

	if (IsDisabled())
		r.Rotate(-35, Vector(0, 0, 1));

	if (!GameServer()->GetRenderer()->ShouldUseShaders())
		r.SetColorSwap(GetTeam()->GetColor());

	r.RenderModel(m_iTurretModel);
}

void CDigitank::RenderShield(float flAlpha)
{
	if (m_iShieldModel == ~0)
		return;

	if (GetVisibility() == 0 || flAlpha == 0)
		return;

	float flFlicker = 1;
	
	if (GetShieldValue() < GetShieldMaxStrength()*3/4)
		flFlicker = Flicker("zzzzmmzzztzzzzzznzzz", GameServer()->GetGameTime() + ((float)GetSpawnSeed()/100), 1.0f);

	CRenderingContext r(GameServer()->GetRenderer());

	if (m_flShieldPulse == 0)
		return;

	float flPulseAlpha = RemapValClamped(GameServer()->GetGameTime(), m_flShieldPulse, m_flShieldPulse + 1.0f, 0.8f, 0);

	float flFinalAlpha = flPulseAlpha*flAlpha*flFlicker*GetVisibility();

	if (flFinalAlpha <= 0)
		return;

	if (GameServer()->GetRenderer()->ShouldUseShaders())
	{
		r.UseProgram(CShaderLibrary::GetScrollingTextureProgram());
		r.SetUniform("iTexture", 0);
		r.SetUniform("flAlpha", flFinalAlpha);
		r.SetUniform("flTime", -GameServer()->GetGameTime());
		r.SetUniform("flSpeed", 1.0f);
	}
	else
		r.SetAlpha(flFinalAlpha);

	r.SetBlend(BLEND_ADDITIVE);
	r.Scale(RenderShieldScale(), RenderShieldScale(), RenderShieldScale());
	r.SetDepthTest(false);

	// If you just call r.RenderModel() it overrides the shader
	CModel* pModel = CModelLibrary::Get()->GetModel(m_iShieldModel);
	if (pModel)
		glCallList((GLuint)pModel->m_iCallList);
}

float CDigitank::AvailableArea() const
{
	return GetBoundingRadius();
}

bool CDigitank::IsAvailableAreaActive(int iArea) const
{
	if (!GetDigitanksTeam())
		return false;

	if (!DigitanksGame()->GetCurrentLocalDigitanksTeam())
		return false;

	if (DigitanksGame()->GetCurrentLocalDigitanksTeam() == GetDigitanksTeam())
		return false;

	if (DigitanksGame()->GetControlMode() != MODE_AIM)
		return false;

	CDigitank* pTank = DigitanksGame()->GetCurrentLocalDigitanksTeam()->GetPrimarySelectionTank();

	if (!pTank)
		return false;

	if (!pTank->IsInsideMaxRange(GetOrigin()))
		return false;

	if (IsScout() && pTank->GetCurrentWeapon() != WEAPON_INFANTRYLASER)
		return false;

	if (GetVisibility(pTank->GetDigitanksTeam()) < 0.1f)
		return false;

	if (pTank->FiringCone() < 360 && fabs(AngleDifference(pTank->GetAngles().y, VectorAngles((GetOrigin()-pTank->GetOrigin()).Normalized()).y)) > pTank->FiringCone())
		return false;

	return true;
}

void CDigitank::DrawSchema(int x, int y, int w, int h)
{
	CRenderingContext c(GameServer()->GetRenderer());
	c.SetBlend(BLEND_ALPHA);

	int iSize = 36;
	DigitanksWindow()->GetHUD()->PaintWeaponSheet(GetCurrentWeapon(), x - 20, y + h - iSize + 10, iSize, iSize);

	int iIconFontSize = 11;
	eastl::string16 sFont = L"text";

	float flYPosition = (float)y + h;
	float flXPosition = (float)x + w + 20;

	if (HasGoalMovePosition())
	{
		float flDistance = (GetRealOrigin() - GetGoalMovePosition()).Length();
		int iTurns = (int)(flDistance/GetMaxMovementDistance());

		eastl::string16 sTurns = sprintf(L"Auto-Move: %d", iTurns);
		float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, (float)y);
	}

	float flIconFontHeight = glgui::CLabel::GetFontHeight(sFont, iIconFontSize) + 2;

	if (IsFortified() || IsFortifying())
	{
		eastl::string16 sTurns = sprintf(L"+%d", GetFortifyLevel());
		float flWidth = glgui::CLabel::GetTextWidth(sTurns, sTurns.length(), sFont, iIconFontSize);

		glgui::CBaseControl::PaintSheet(CHUD::GetButtonSheet(), (int)(flXPosition - flWidth - flIconFontHeight), (int)(flYPosition - flIconFontHeight) + 5, (int)flIconFontHeight, (int)flIconFontHeight, 0, 128, 64, 64, 512, 256);
		glgui::CLabel::PaintText(sTurns, sTurns.length(), sFont, iIconFontSize, flXPosition - flWidth, flYPosition);

		flYPosition -= flIconFontHeight;
	}
}

void CDigitank::UpdateInfo(eastl::string16& s)
{
	s = L"";
	eastl::string16 p;

	s += GetName();
	s += L"\n \n";

	if (GetTeam())
	{
		s += L"Team: " + GetTeam()->GetName() + L"\n";
		if (GetDigitanksTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
			s += L" Friendly\n \n";
		else
			s += L" Hostile\n \n";
	}
	else
	{
		s += L"Team: Neutral\n \n";
	}

	if (IsDisabled())
		s += L"[Disabled]\n \n";

	if (IsFortified())
		s += L"[Fortified]\n \n";

	else if (IsFortifying())
		s += L"[Fortifying...]\n \n";

	if (HasBonusPoints())
	{
		if (GetBonusPoints() > 1)
			s += p.sprintf(L"%d upgrades\n \n", GetBonusPoints());
		else
			s += L"1 upgrade\n \n";
	}

	if (GetBonusAttackPower())
	{
		s += p.sprintf(L"+%d attack energy\n", (int)GetBonusAttackPower());

		if (IsFortified() && (int)GetFortifyAttackPowerBonus() > 0)
			s += p.sprintf(L" (+%d from fortify)\n", (int)GetFortifyAttackPowerBonus());

		if ((int)GetSupportAttackPowerBonus() > 0)
			s += p.sprintf(L" (+%d from support)\n", (int)GetSupportAttackPowerBonus());

		s += L" \n";
	}

	if (GetBonusDefensePower())
	{
		s += p.sprintf(L"+%d shield energy\n \n", (int)GetBonusDefensePower());

		if (IsFortified() && (int)GetFortifyDefensePowerBonus() > 0)
			s += p.sprintf(L" (+%d from fortify)\n \n", (int)GetFortifyDefensePowerBonus());

		if ((int)GetSupportDefensePowerBonus() > 0)
			s += p.sprintf(L" (+%d from support)\n \n", (int)GetSupportDefensePowerBonus());

		s += L" \n";
	}

	if (GetBonusMovementEnergy() > 0)
		s += p.sprintf(L"+%d movement energy\n \n", (int)GetBonusMovementEnergy());
}

void CDigitank::GiveBonusPoints(size_t i, bool bPlayEffects)
{
	if (!CNetwork::IsHost())
		return;

	m_iBonusPoints += i;

	if (bPlayEffects)
	{
		TankPromoted(NULL);
		CNetwork::CallFunction(NETWORK_TOCLIENTS, "TankPromoted", GetHandle());
	}

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	Speak(TANKSPEECH_PROMOTED);
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
}

void CDigitank::PromoteAttack()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(NETWORK_TOSERVER, "PromoteAttack", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusAttackPower++;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteDefense()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(NETWORK_TOSERVER, "PromoteDefense", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusDefensePower++;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::PromoteMovement()
{
	if (!CNetwork::IsHost())
	{
		CNetwork::CallFunction(NETWORK_TOSERVER, "PromoteMovement", GetHandle());
		return;
	}

	if (m_iBonusPoints <= 0)
		return;

	m_iBonusPoints--;
	m_flBonusMovementPower++;

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "SetBonusPoints", GetHandle(), m_iBonusPoints, m_flBonusAttackPower, m_flBonusDefensePower, m_flBonusMovementPower);

	if (GetTeam()->IsPlayerControlled())
	{
		Speak(TANKSPEECH_PROMOTED);
		m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);
	}
}

void CDigitank::DownloadComplete(size_t x, size_t y)
{
	CUpdateItem* pItem = &DigitanksGame()->GetUpdateGrid()->m_aUpdates[x][y];

	if (pItem->m_eStructure != GetUnitType())
		return;

	if (pItem->m_eUpdateClass != UPDATECLASS_UNITSKILL)
		return;

	if (pItem->m_eUpdateType == UPDATETYPE_SKILL_CLOAK)
		GiveCloak();
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_AOE)
	{
		if (pItem->m_eStructure == UNIT_TANK)
			m_aeWeapons.push_back(PROJECTILE_AOE);
		else if (pItem->m_eStructure == UNIT_ARTILLERY)
			m_aeWeapons.push_back(PROJECTILE_ARTILLERY_AOE);
	}
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_ICBM)
	{
		if (pItem->m_eStructure == UNIT_TANK)
			m_aeWeapons.push_back(PROJECTILE_ICBM);
		else if (pItem->m_eStructure == UNIT_ARTILLERY)
			m_aeWeapons.push_back(PROJECTILE_ARTILLERY_ICBM);
	}
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_CHARGERAM)
		m_aeWeapons.push_back(WEAPON_CHARGERAM);
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_CLUSTER)
		m_aeWeapons.push_back(PROJECTILE_CLUSTERBOMB);
	else if (pItem->m_eUpdateType == UPDATETYPE_WEAPON_DEVASTATOR)
		m_aeWeapons.push_back(PROJECTILE_DEVASTATOR);

	DigitanksWindow()->GetHUD()->Layout();
}

void CDigitank::SetBonusPoints(class CNetworkParameters* p)
{
	m_iBonusPoints = p->i2;
	m_flBonusAttackPower = p->fl3;
	m_flBonusDefensePower = p->fl4;
	m_flBonusMovementPower = p->fl5;
}

void CDigitank::TankPromoted(class CNetworkParameters* p)
{
	if (GetVisibility() > 0)
	{
		EmitSound(L"sound/tank-promoted.wav");
		CParticleSystemLibrary::AddInstance(L"promotion", GetRealOrigin());
	}
}

void CDigitank::PromoteAttack(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	PromoteAttack();
}

void CDigitank::PromoteDefense(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	PromoteDefense();
}

void CDigitank::PromoteMovement(class CNetworkParameters* p)
{
	if (!CNetwork::IsHost())
		return;

	PromoteMovement();
}

void CDigitank::Speak(size_t iSpeech)
{
	if (!CNetwork::IsHost())
		return;

	if (rand()%4 != 0)
		return;

	if (GameServer()->GetGameTime() < m_flLastSpeech + 5.0f)
		return;

	if (GetVisibility() == 0)
		return;

	if (IsCloaked())
		return;

	// No talking when we're hiding in the trees!
	if (DigitanksGame()->GetTerrain()->IsPointInTrees(GetOrigin()))
		return;

	size_t iLine = g_aiSpeechLines[iSpeech][rand()%g_aiSpeechLines[iSpeech].size()];

	m_flLastSpeech = GameServer()->GetGameTime();
	m_flNextIdle = GameServer()->GetGameTime() + RandomFloat(10, 20);

	CNetwork::CallFunction(NETWORK_TOCLIENTS, "TankSpeak", GetHandle(), iLine);

	CNetworkParameters p;
	p.i2 = (int)iLine;
	Speak(&p);
}

void CDigitank::Speak(class CNetworkParameters* p)
{
	DigitanksGame()->TankSpeak(this, s_apszTankLines[p->i2]);
}

float CDigitank::FindHoverHeight(Vector vecPosition) const
{
	if (!DigitanksGame())
		return vecPosition.y;

	CTerrain* pTerrain = DigitanksGame()->GetTerrain();

	Vector vecHit;

	float flHighestTerrain = pTerrain->GetHeight(vecPosition.x, vecPosition.z);
	bool bHit;

	bHit = Game()->TraceLine(vecPosition + Vector(0, 100, 0), vecPosition + Vector(0, -100, 0), vecHit, NULL, CG_TERRAIN|CG_PROP);
	if (bHit)
		flHighestTerrain = vecHit.y;

	float flTerrain;

	// Only do one trace to see if we're on a structure. If we are then we'll just have to hope it's not too steep.
	// Five traces really slows things down.
	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.z+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x+2, vecPosition.z-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.z+2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	flTerrain = pTerrain->GetHeight(vecPosition.x-2, vecPosition.z-2);
	if (flTerrain > flHighestTerrain)
		flHighestTerrain = flTerrain;

	return flHighestTerrain;
}

bool CDigitank::Collide(const Vector& v1, const Vector& v2, Vector& vecPoint)
{
	if (BaseClass::Collide(v1, v2, vecPoint))
		return true;

	if (GetBoundingRadius() == 0)
		return false;

	return LineSegmentIntersectsSphere(v1, v2, GetOrigin(), GetBoundingRadius(), vecPoint);
}

float CDigitank::HealthRechargeRate() const
{
	return 2.0f + GetSupportHealthRechargeBonus();
}

float CDigitank::ShieldRechargeRate() const
{
	return 2.0f + GetSupportShieldRechargeBonus();
}

float CDigitank::FirstProjectileTime() const
{
	return RandomFloat(0, 1);
}

void CDigitank::Disable(size_t iTurns)
{
	if (m_iTurnsDisabled < iTurns)
		m_iTurnsDisabled = iTurns;

	DigitanksGame()->OnDisabled(this, NULL, NULL);
}

void CDigitank::SetFortifyPoint(CStructure* pStructure, Vector vecFortify)
{
	m_bFortifyPoint = true;
	m_vecFortifyPoint = vecFortify;

	m_hFortifyDefending = pStructure;
}

void CDigitank::RemoveFortifyPoint()
{
	m_bFortifyPoint = false;

	if (m_hFortifyDefending != NULL)
		m_hFortifyDefending->RemoveDefender(this);
}
