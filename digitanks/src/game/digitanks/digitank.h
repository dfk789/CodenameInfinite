#ifndef DT_DIGITANK_H
#define DT_DIGITANK_H

#include "selectable.h"
#include "loader.h"
#include <common.h>

#define TANK_SHIELDS 4

// Tank speech
typedef enum
{
	TANKSPEECH_SELECTED,
	TANKSPEECH_MOVED,
	TANKSPEECH_ATTACK,
	TANKSPEECH_DAMAGED,
	TANKSPEECH_KILL,
	TANKSPEECH_MISSED,
	TANKSPEECH_IDLE,
	TANKSPEECH_PROMOTED,
};

typedef enum
{
	TANKLINE_CUTE,
	TANKLINE_LOVE,
	TANKLINE_HAPPY,
	TANKLINE_CHEER,
	TANKLINE_EVIL,
	TANKLINE_SQUINT,
	TANKLINE_COOL,
	TANKLINE_DEAD,
	TANKLINE_DEAD2,
	TANKLINE_DEAD3,
	TANKLINE_FROWN,
	TANKLINE_SAD,
	TANKLINE_ASLEEP,
	TANKLINE_CONFUSED,
	TANKLINE_DOTDOTDOT,
	TANKLINE_SURPRISED,
	TANKLINE_THRILLED,
};

class CDigitank : public CSelectable
{
	REGISTER_ENTITY_CLASS(CDigitank, CSelectable);

public:
								CDigitank();
								~CDigitank();

public:
	virtual void				Precache();

	virtual float				GetBoundingRadius() const { return 4; };

	float						GetTotalPower() const { return m_flTotalPower; };
	float						GetStartingPower() const { return m_flStartingPower; };
	float						GetBaseAttackPower(bool bPreview = false);
	float						GetBaseDefensePower(bool bPreview = false);
	float						GetBaseMovementPower(bool bPreview = false);

	float						GetAttackPower(bool bPreview = false);
	float						GetDefensePower(bool bPreview = false);
	float						GetMovementPower(bool bPreview = false);
	float						GetTotalAttackPower();
	float						GetTotalDefensePower();
	float						GetTotalMovementPower() const;
	float						GetMaxMovementDistance() const;

	float						GetAttackScale(bool bPreview = false) { return GetAttackPower(bPreview) / 10; };
	float						GetDefenseScale(bool bPreview = false) { return GetDefensePower(bPreview) / 10; };
	float						GetMovementScale(bool bPreview = false) { return GetMovementPower(bPreview) / 10; };

	virtual float				GetBonusAttackScale(bool bPreview = false);
	virtual float				GetBonusDefenseScale(bool bPreview = false);
	virtual float				GetBonusAttackPower(bool bPreview = false);
	virtual float				GetBonusDefensePower(bool bPreview = false);
	virtual float				GetBonusMovementPower() const { return m_flBonusMovementPower; };

	virtual void				AddRangeBonus(float flAmount) { m_flRangeBonus += flAmount; };

	virtual float				GetSupportAttackPowerBonus();
	virtual float				GetSupportDefensePowerBonus();

	virtual float				GetSupportHealthRechargeBonus() const;
	virtual float				GetSupportShieldRechargeBonus() const;

	virtual void				SetAttackPower(float flAttackPower);
	void						SetAttackPower(class CNetworkParameters* p);

	float						GetPowerAttackSplit() { return m_flAttackSplit; }

	float						GetPreviewMoveTurnPower();
	float						GetPreviewMovePower() const;
	float						GetPreviewTurnPower() const;
	float						GetPreviewBaseMovePower() const;
	float						GetPreviewBaseTurnPower() const;

	bool						IsPreviewMoveValid() const;

	virtual float				GetFrontShieldMaxStrength() { return m_flFrontMaxShieldStrength; };
	virtual float				GetLeftShieldMaxStrength() { return m_flLeftMaxShieldStrength; };
	virtual float				GetRightShieldMaxStrength() { return m_flRightMaxShieldStrength; };
	virtual float				GetRearShieldMaxStrength() { return m_flRearMaxShieldStrength; };
	virtual float				GetFrontShieldStrength();
	virtual float				GetLeftShieldStrength();
	virtual float				GetRightShieldStrength();
	virtual float				GetRearShieldStrength();

	virtual float*				GetShieldForAttackDirection(Vector vecAttack);

	virtual void				StartTurn();
	virtual void				EndTurn();

	virtual CDigitank*			FindClosestVisibleEnemyTank();

	Vector						GetPreviewMove() const { return m_vecPreviewMove; };
	virtual void				SetPreviewMove(Vector vecPreviewMove);
	void						ClearPreviewMove();

	float						GetPreviewTurn() const { return m_flPreviewTurn; };
	virtual void				SetPreviewTurn(float flPreviewTurn);
	void						ClearPreviewTurn();

	Vector						GetPreviewAim() { return m_vecPreviewAim; };
	void						SetPreviewAim(Vector vecPreviewAim);
	void						ClearPreviewAim();
	bool						IsPreviewAimValid();

	virtual void				SetDesiredMove();
	void						SetDesiredMove(class CNetworkParameters* p);
	void						CancelDesiredMove();
	void						CancelDesiredMove(class CNetworkParameters* p);
	bool						HasDesiredMove() const { return m_bDesiredMove; };
	Vector						GetDesiredMove() const;
	bool						HasSelectedMove() { return m_bSelectedMove; };
	bool						IsMoving();

	virtual void				SetDesiredTurn();
	void						SetDesiredTurn(class CNetworkParameters* p);
	void						CancelDesiredTurn();
	void						CancelDesiredTurn(class CNetworkParameters* p);
	bool						HasDesiredTurn() const { return m_bDesiredTurn; };
	float						GetDesiredTurn() const;

	void						SetDesiredAim();
	void						SetDesiredAim(class CNetworkParameters* p);
	void						CancelDesiredAim();
	void						CancelDesiredAim(class CNetworkParameters* p);
	bool						HasDesiredAim() { return m_bDesiredAim; };
	Vector						GetDesiredAim();

	void						SetGoalMovePosition(const Vector& vecPosition);
	void						MoveTowardsGoalMovePosition();
	void						CancelGoalMovePosition();
	bool						HasGoalMovePosition() { return m_bGoalMovePosition; };
	Vector						GetGoalMovePosition() { return m_vecGoalMovePosition; };

	bool						ChoseFirepower() { return m_bChoseFirepower; };

	virtual void				Fortify();
	virtual bool				CanFortify() { return false; };
	virtual bool				IsArtillery() const { return false; };
	virtual bool				IsInfantry() const { return false; };
	virtual bool				IsScout() const { return false; };
	virtual bool				IsFortified() const { return m_bFortified && m_iFortifyLevel; };
	virtual bool				IsFortifying() const { return m_bFortified && m_iFortifyLevel == 0; };
	virtual bool				CanMoveFortified() { return false; };
	virtual bool				CanTurnFortified() { return false; };
	virtual bool				CanAimMobilized() const { return true; };
	virtual bool				CanAim() const;
	virtual float				GetFortifyAttackPowerBonus() { return 0; };
	virtual float				GetFortifyDefensePowerBonus() { return 0; };
	virtual bool				CanGetPowerups() const { return true; };

	virtual bool				MovesWith(CDigitank* pOther) const;
	virtual bool				TurnsWith(CDigitank* pOther) const;
	virtual bool				AimsWith(CDigitank* pOther) const;

	virtual void				Think();

	// CSelectable
	virtual void				OnCurrentSelection();
	virtual bool				AllowControlMode(controlmode_t eMode) const;
	virtual void				OnControlModeChange(controlmode_t eOldMode, controlmode_t eNewMode);

	virtual const char*			GetPowerBar1Text() { return "Attack"; }
	virtual const char*			GetPowerBar2Text() { return "Defense"; }
	virtual const char*			GetPowerBar3Text() { return "Movement"; }

	virtual float				GetPowerBar1Value();
	virtual float				GetPowerBar2Value();
	virtual float				GetPowerBar3Value();

	virtual float				GetPowerBar1Size();
	virtual float				GetPowerBar2Size();
	virtual float				GetPowerBar3Size();

	virtual bool				NeedsOrders();

	virtual void				SetupMenu(menumode_t eMenuMode);

	virtual void				Move();
	virtual void				Turn();
	virtual void				Fire();
	void						FireProjectile();
	virtual void				FireProjectile(class CNetworkParameters* p);
	virtual class CProjectile*	CreateProjectile();
	virtual float				GetProjectileDamage();
	virtual bool				IsWaitingToFire() { return m_flFireProjectileTime != 0; };

	virtual void				ClientUpdate(int iClient);

	virtual void				TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, float flDamage, bool bDirectHit = true);
	virtual void				OnKilled(CBaseEntity* pKilledBy);

	virtual Vector				GetRenderOrigin() const;
	virtual EAngle				GetRenderAngles() const;
	virtual void				PreRender();
	virtual void				ModifyContext(class CRenderingContext* pContext);
	virtual void				OnRender();
	virtual void				RenderTurret(float flAlpha = 1.0f);
	virtual void				RenderShield(float flAlpha, float flAngle);
	virtual float				RenderShieldScale() { return 1.0f; };
	virtual void				PostRender();

	virtual void				UpdateInfo(std::wstring& sInfo);

	void						GiveBonusPoints(size_t i, bool bPlayEffects = true);
	bool						HasBonusPoints() { return m_iBonusPoints > 0; };
	size_t						GetBonusPoints() { return m_iBonusPoints; };
	void						PromoteAttack();
	void						PromoteDefense();
	void						PromoteMovement();
	void						SetBonusPoints(class CNetworkParameters* p);
	void						TankPromoted(class CNetworkParameters* p);
	void						PromoteAttack(class CNetworkParameters* p);
	void						PromoteDefense(class CNetworkParameters* p);
	void						PromoteMovement(class CNetworkParameters* p);

	void						Speak(size_t iSpeech);
	void						Speak(class CNetworkParameters* p);

	float						FindHoverHeight(Vector vecPosition) const;

	virtual bool				Collide(const Vector& v1, const Vector& v2, Vector& vecPoint);

	virtual float				HealthRechargeRate() const;
	virtual float				ShieldRechargeRate() const;
	virtual float				GetTankSpeed() const { return 2.0f; };
	virtual float				TurnPerPower() const { return 45; };
	virtual float				InitialMaxRange() const { return 70.0f; };
	virtual float				GetMaxRange() const { return InitialMaxRange() + m_flRangeBonus; };
	virtual float				InitialEffRange() const { return 50.0f; };
	virtual float				GetEffRange() const { return InitialEffRange() + m_flRangeBonus/2; };
	virtual float				GetMinRange() const { return 4.0f; };
	virtual float				GetTransitionTime() const { return 2.0f; };
	virtual float				ProjectileCurve() const { return -0.03f; };
	virtual float				FiringCone() const { return 360; };
	virtual float				VisibleRange() const { return 75; };
	virtual size_t				FleetPoints() const { return 2; };
	virtual float				BobHeight() const { return 0.5f; };
	virtual float				MaxRangeRadius() const { return 10; };

	virtual bool				HasFiredWeapon() const { return m_bFiredWeapon; }

	virtual buildunit_t			GetBuildUnit() const { return BUILDUNIT_TANK; }

	// AI stuff
	virtual void				SetFortifyPoint(Vector vecFortify);
	virtual bool				HasFortifyPoint() { return m_bFortifyPoint; };
	virtual Vector				GetFortifyPoint() { return m_vecFortifyPoint; }

protected:
	// Power remaining for this turn
	float						m_flStartingPower;
	float						m_flTotalPower;

	// Power used so far
	float						m_flAttackPower;
	float						m_flDefensePower;
	float						m_flMovementPower;

	float						m_flAttackSplit;

	float						m_flBonusAttackPower;
	float						m_flBonusDefensePower;
	float						m_flBonusMovementPower;
	size_t						m_iBonusPoints;

	float						m_flRangeBonus;

	union {
		struct {
			float				m_flFrontMaxShieldStrength;
			float				m_flLeftMaxShieldStrength;
			float				m_flRightMaxShieldStrength;
			float				m_flRearMaxShieldStrength;
		};
		float					m_flMaxShieldStrengths[TANK_SHIELDS];
	};

	union {
		struct {
			float				m_flFrontShieldStrength;
			float				m_flLeftShieldStrength;
			float				m_flRightShieldStrength;
			float				m_flRearShieldStrength;
		};
		float					m_flShieldStrengths[TANK_SHIELDS];
	};

	Vector						m_vecPreviewMove;
	bool						m_bDesiredMove;
	Vector						m_vecPreviousOrigin;
	Vector						m_vecDesiredMove;
	bool						m_bSelectedMove;
	float						m_flStartedMove;

	float						m_flPreviewTurn;
	bool						m_bDesiredTurn;
	float						m_flPreviousTurn;
	float						m_flDesiredTurn;
	float						m_flStartedTurn;

	bool						m_bPreviewAim;
	Vector						m_vecPreviewAim;
	bool						m_bDesiredAim;
	Vector						m_vecDesiredAim;

	bool						m_bDisplayAim;
	Vector						m_vecDisplayAim;
	float						m_flDisplayAimRadius;

	bool						m_bGoalMovePosition;
	Vector						m_vecGoalMovePosition;

	bool						m_bChoseFirepower;
	bool						m_bFiredWeapon;

	float						m_flFireProjectileTime;
	CEntityHandle<class CProjectile>	m_hProjectile;

	float						m_flLastSpeech;
	float						m_flNextIdle;

	size_t						m_iTurretModel;
	size_t						m_iShieldModel;

	size_t						m_iHoverParticles;

	bool						m_bFortified;
	size_t						m_iFortifyLevel;

	CEntityHandle<class CSupplier>		m_hSupplier;
	CEntityHandle<class CSupplyLine>	m_hSupplyLine;

	float						m_flFortifyTime;

	float						m_flBobOffset;

	// AI stuff
	bool						m_bFortifyPoint;
	Vector						m_vecFortifyPoint;

	static size_t				s_iAimBeam;
	static size_t				s_iCancelIcon;
	static size_t				s_iMoveIcon;
	static size_t				s_iTurnIcon;
	static size_t				s_iAimIcon;
	static size_t				s_iFireIcon;
	static size_t				s_iEnergyIcon;
	static size_t				s_iPromoteIcon;
	static size_t				s_iPromoteAttackIcon;
	static size_t				s_iPromoteDefenseIcon;
	static size_t				s_iPromoteMoveIcon;
	static size_t				s_iFortifyIcon;
	static size_t				s_iDeployIcon;
	static size_t				s_iMobilizeIcon;

	static size_t				s_iAutoMove;

	static const char*			s_apszTankLines[];
};

#endif