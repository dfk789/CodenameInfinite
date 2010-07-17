#ifndef DT_DIGITANKSGAME_H
#define DT_DIGITANKSGAME_H

#include <vector>

#include "game.h"
#include "digitank.h"
#include <common.h>
#include "terrain.h"
#include "digitanksteam.h"
#include "dt_common.h"

class IDigitanksGameListener
{
public:
	virtual void			GameStart()=0;
	virtual void			GameOver(bool bPlayerWon)=0;

	virtual void			NewCurrentTeam()=0;
	virtual void			NewCurrentSelection()=0;

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly)=0;
	virtual void			OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled)=0;

	virtual void			TankSpeak(class CDigitank* pTank, const std::string& sSpeech)=0;

	virtual void			ClearTurnInfo()=0;
	virtual void			AppendTurnInfo(const char* pszInfo)=0;

	virtual void			SetHUDActive(bool bActive)=0;
};

class CDigitanksGame : public CGame
{
	DECLARE_CLASS(CDigitanksGame, CGame);

public:
							CDigitanksGame();
							~CDigitanksGame();

public:
	void					SetListener(IDigitanksGameListener* pListener) { m_pListener = pListener; };

	virtual void			RegisterNetworkFunctions();

	virtual void			OnClientConnect(CNetworkParameters* p);
	virtual void			OnClientUpdate(CNetworkParameters* p);
	virtual void			OnClientDisconnect(CNetworkParameters* p);

	void					SetupGame();
	void					SetupEntities();
	NET_CALLBACK(CDigitanksGame, SetupEntities);

	void					StartGame();
	NET_CALLBACK(CDigitanksGame, EnterGame);

	virtual void			Think();

	void					SetDesiredMove(bool bAllTanks = false);
	void					SetDesiredTurn(bool bAllTanks = false, Vector vecLookAt = Vector());
	void					SetDesiredAim(bool bAllTanks = false);

	void					NextTank();
	void					EndTurn();
	NET_CALLBACK(CDigitanksGame, EndTurn);
	void					StartTurn();
	NET_CALLBACK(CDigitanksGame, StartTurn);

	void					Bot_ExecuteTurn();

	virtual bool			Explode(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flRadius, float flDamage, CBaseEntity* pIgnore, CTeam* pTeamIgnore);

	virtual void			OnTakeShieldDamage(class CDigitank* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bShieldOnly);

	virtual void			OnTakeDamage(class CBaseEntity* pVictim, class CBaseEntity* pAttacker, class CBaseEntity* pInflictor, float flDamage, bool bDirectHit, bool bKilled);
	virtual void			OnKilled(class CBaseEntity* pEntity);
	void					CheckWinConditions();

	virtual void			OnDeleted(class CBaseEntity* pEntity);

	virtual void			TankSpeak(class CDigitank* pTank, const std::string& sSpeech);

	CDigitanksTeam*			GetDigitanksTeam(size_t i);

	CDigitanksTeam*			GetCurrentTeam();
	class CSelectable*		GetCurrentSelection();
	class CDigitank*		GetCurrentTank();
	class CStructure*		GetCurrentStructure();
	size_t					GetCurrentSelectionId();
	bool					IsCurrentSelection(const class CSelectable* pEntity);
	void					SetCurrentSelection(CSelectable* pCurrent);

	controlmode_t			GetControlMode();
	void					SetControlMode(controlmode_t eMode, bool bAutoProceed = false);

	CTerrain*				GetTerrain() { if (m_hTerrain == NULL) return NULL; return m_hTerrain; };
	NET_CALLBACK(CDigitanksGame, SetTerrain);

	NET_CALLBACK(CDigitanksGame, SetCurrentTeam);

	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetDesiredMove);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelDesiredMove);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetDesiredTurn);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelDesiredTurn);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetDesiredAim);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, CancelDesiredAim);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetAttackPower);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, FireProjectile);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, SetBonusPoints);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, TankPromoted);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteAttack);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteDefense);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, PromoteMovement);
	NET_CALLBACK_ENTITY(CDigitanksGame, CDigitank, Speak);

	virtual void			CreateRenderer();
	virtual class CDigitanksRenderer*	GetDigitanksRenderer();

	float					GetGravity();

	void					AddProjectileToWaitFor() { m_iWaitingForProjectiles++; };

	void					AddTankAim(Vector vecAim, float flRadius, bool bFocus);
	void					GetTankAims(std::vector<Vector>& avecAims, std::vector<float>& aflAimRadius, size_t& iFocus);
	void					ClearTankAims();

	void					SetDifficulty(size_t iDifficulty) { m_iDifficulty = iDifficulty; };
	size_t					GetDifficulty() { return m_iDifficulty; };

	void					AppendTurnInfo(const char* pszTurnInfo);

	void					SetRenderFogOfWar(bool bRenderFogOfWar) { m_bRenderFogOfWar = bRenderFogOfWar; };
	bool					ShouldRenderFogOfWar() { return m_bRenderFogOfWar; };

	// CHEAT!
	void					CompleteProductions();

	static CDigitanksTeam*	GetLocalDigitanksTeam();

protected:
	size_t					m_iCurrentTeam;
	size_t					m_iCurrentSelection;

	controlmode_t			m_eControlMode;

	CEntityHandle<CTerrain>	m_hTerrain;

	IDigitanksGameListener*	m_pListener;

	bool					m_bWaitingForMoving;

	bool					m_bWaitingForProjectiles;
	size_t					m_iWaitingForProjectiles;

	size_t					m_iPowerups;

	std::vector<Vector>		m_avecTankAims;
	std::vector<float>		m_aflTankAimRadius;
	size_t					m_iTankAimFocus;

	size_t					m_iDifficulty;

	bool					m_bRenderFogOfWar;
};

inline class CDigitanksGame* DigitanksGame()
{
	if (!Game())
		return NULL;

	return dynamic_cast<CDigitanksGame*>(Game());
}

enum
{
	CG_ENTITY = 1,
	CG_TERRAIN,
	CG_PROJECTILE,
	CG_POWERUP,
};

#endif