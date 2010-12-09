#ifndef DT_PROJECTILE_H
#define DT_PROJECTILE_H

#include "baseweapon.h"
#include <digitanks/units/digitank.h>

class CProjectile : public CBaseWeapon
{
	REGISTER_ENTITY_CLASS(CProjectile, CBaseWeapon);

public:
								CProjectile();

public:
	virtual void				Precache();

	virtual void				Think();

	virtual bool				MakesSounds() { return true; };

	virtual void				ModifyContext(class CRenderingContext* pContext, bool bTransparent);
	virtual bool				ShouldRender() const { return true; };
	virtual void				OnRender(class CRenderingContext* pContext, bool bTransparent);

	virtual void				OnDeleted();

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual bool				IsTouching(CBaseEntity* pOther, Vector& vecPoint) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				OnExplode(CBaseEntity* pInstigator);
	virtual bool				ShouldPlayExplosionSound();

	virtual void				OnSetOwner(CDigitank* pOwner);
	virtual bool				ShouldBeVisible();

	virtual void				SetLandingSpot(Vector vecLandingSpot) { m_vecLandingSpot = vecLandingSpot; };

	virtual size_t				CreateParticleSystem();

	virtual void				ClientEnterGame();

	virtual float				ShieldDamageScale() { return 1; };
	virtual float				HealthDamageScale() { return 1; };
	virtual float				ShellRadius() { return 0.5f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				BombDropNoise() { return true; };
	virtual bool				SendsNotifications() { return true; };
	virtual size_t				Fragments() { return 0; };
	virtual size_t				Bounces() { return 0; };

protected:
	bool						m_bFallSoundPlayed;

	Vector						m_vecLandingSpot;

	size_t						m_iParticleSystem;

	bool						m_bFragmented;
	size_t						m_iBounces;

	bool						m_bMissileDefensesNotified;
};

class CSmallShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CSmallShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_SMALL; }
	virtual float				ShellRadius() { return 0.5f; };
	virtual float				ExplosionRadius() { return 6.0f; };
	virtual float				PushDistance() { return 3.0f; };
	virtual float				RockIntensity() { return 0.4f; };
};

class CMediumShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CMediumShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_MEDIUM; }
	virtual float				ShellRadius() { return 1.0f; };
	virtual float				ExplosionRadius() { return 12.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
};

class CLargeShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CLargeShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_LARGE; }
	virtual float				ShellRadius() { return 1.5f; };
	virtual float				ExplosionRadius() { return 18.0f; };
	virtual float				PushDistance() { return 9.0f; };
	virtual float				RockIntensity() { return 1.0f; };
};

class CAOEShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CAOEShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_AOE; }
	virtual float				ShellRadius() { return 1.2f; };
	virtual float				ExplosionRadius() { return 30.0f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				HasDamageFalloff() { return false; };
};

class CEMP : public CProjectile
{
	REGISTER_ENTITY_CLASS(CEMP, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_EMP; }
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShellRadius() { return 0.7f; };
	virtual float				ExplosionRadius() { return 6.0f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
};

class CICBM : public CProjectile
{
	REGISTER_ENTITY_CLASS(CICBM, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_ICBM; }
	virtual float				ShellRadius() { return 1.2f; };
	virtual float				ExplosionRadius() { return 12.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
	virtual size_t				Fragments() { return 6; };
};

class CGrenade : public CProjectile
{
	REGISTER_ENTITY_CLASS(CGrenade, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_GRENADE; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return 16.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
	virtual size_t				Bounces() { return 2; };
};

class CDaisyChain : public CProjectile
{
	REGISTER_ENTITY_CLASS(CDaisyChain, CProjectile);

public:
	virtual void				Spawn();

	virtual void				OnExplode(CBaseEntity* pInstigator);

	virtual weapon_t			GetWeaponType() { return PROJECTILE_DAISYCHAIN; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return m_flExplosionRadius; };
	virtual float				PushDistance() { return 3.0f; };
	virtual float				RockIntensity() { return 0.7f; };

protected:
	float						m_flExplosionRadius;
};

class CClusterBomb : public CProjectile
{
	REGISTER_ENTITY_CLASS(CClusterBomb, CProjectile);

public:
	virtual void				Spawn();

	virtual void				OnExplode(CBaseEntity* pInstigator);

	virtual weapon_t			GetWeaponType() { return PROJECTILE_CLUSTERBOMB; }
	virtual float				ShellRadius() { return 1.3f; };
	virtual float				ExplosionRadius() { return m_flExplosionRadius; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };

protected:
	float						m_flExplosionRadius;
};

class CEarthshaker : public CProjectile
{
	REGISTER_ENTITY_CLASS(CEarthshaker, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_EARTHSHAKER; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return 22.0f; };
	virtual float				PushDistance() { return 6.0f; };
	virtual float				RockIntensity() { return 0.7f; };
};

class CSploogeShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CSploogeShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_SPLOOGE; }
	virtual float				ShellRadius() { return 0.2f; };
	virtual float				ExplosionRadius() { return 0.0f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
	virtual bool				CreatesCraters() { return false; };
};

class CTractorBomb : public CProjectile
{
	REGISTER_ENTITY_CLASS(CTractorBomb, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_TRACTORBOMB; }
	virtual float				ShellRadius() { return 0.8f; };
	virtual float				ExplosionRadius() { return 0.0f; };
	virtual float				PushRadius() { return 40.0f; };
	virtual float				PushDistance() { return 20.0f; };
	virtual float				RockIntensity() { return 1.0f; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				HasDamageFalloff() { return false; };
};

class CArtilleryShell : public CProjectile
{
	REGISTER_ENTITY_CLASS(CArtilleryShell, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_ARTILLERY; }
	virtual bool				CreatesCraters() { return false; };
	virtual float				ShieldDamageScale() { return 2; };
	virtual float				HealthDamageScale() { return 0.5f; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };
};

class CInfantryFlak : public CProjectile
{
	REGISTER_ENTITY_CLASS(CInfantryFlak, CProjectile);

public:
	virtual weapon_t			GetWeaponType() { return PROJECTILE_FLAK; }
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.2f; };
	virtual bool				ShouldExplode() { return false; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return false; };
	virtual bool				SendsNotifications() { return false; };
	virtual size_t				CreateParticleSystem();
};

class CTorpedo : public CProjectile
{
	REGISTER_ENTITY_CLASS(CTorpedo, CProjectile);

public:
								CTorpedo();

public:
	virtual void				Think();

	virtual bool				ShouldTouch(CBaseEntity* pOther) const;
	virtual void				Touching(CBaseEntity* pOther);

	virtual void				Explode(CBaseEntity* pInstigator = NULL);

	virtual weapon_t			GetWeaponType() { return PROJECTILE_TORPEDO; }
	virtual bool				MakesSounds() { return true; };
	virtual float				ShellRadius() { return 0.35f; };
	virtual bool				ShouldExplode() { return true; };
	virtual bool				CreatesCraters() { return false; };
	virtual bool				BombDropNoise() { return false; };
	virtual bool				SendsNotifications() { return false; };
	virtual float				PushDistance() { return 0.0f; };
	virtual float				RockIntensity() { return 0.0f; };

protected:
	bool						m_bBurrowing;
};

#endif