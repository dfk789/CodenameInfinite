#ifndef DT_INTRO_TANK_H
#define DT_INTRO_TANK_H

#include <game/baseentity.h>

class CIntroTank : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CIntroTank, CBaseEntity);

public:
					CIntroTank();

public:
	virtual bool	ShouldRender() const { return true; };
	virtual void	ModifyContext(class CRenderingContext* pContext, bool bTransparent);
	virtual void	OnRender(class CRenderingContext* pContext, bool bTransparent);

	void			FaceTurret(float flYaw) { m_flGoalTurretYaw = flYaw; };

protected:
	float			m_flCurrentTurretYaw;
	float			m_flGoalTurretYaw;
	size_t			m_iTurretModel;
};

#endif
