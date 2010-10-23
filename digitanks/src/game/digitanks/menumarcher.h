#ifndef DT_MENUMARCHER_H
#define DT_MENUMARCHER_H

#include "baseentity.h"

class CMenuMarcher : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CMenuMarcher, CBaseEntity);

public:
								~CMenuMarcher();

public:
	virtual void				Precache();
	virtual void				Spawn();

	virtual float				GetBoundingRadius() const { return 4; };

	virtual void				Think();

	virtual bool				ShouldRender() const { return true; };
	virtual Vector				GetRenderOrigin() const;
	virtual void				ModifyContext(class CRenderingContext* pContext);
	virtual void				OnRender();
	virtual void				RenderTurret();

	void						Speak();

	float						FindHoverHeight(Vector vecPosition) const;

protected:
	float						m_flNextSpeech;

	size_t						m_iTurretModel;

	size_t						m_iHoverParticles;

	float						m_flBobOffset;
};

#endif