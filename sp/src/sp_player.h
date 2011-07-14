#ifndef SP_PLAYER_H
#define SP_PLAYER_H

#include <tengine/game/entities/player.h>

class CSPPlayer : public CPlayer
{
	REGISTER_ENTITY_CLASS(CSPPlayer, CPlayer);

public:
	virtual void					MouseMotion(int x, int y);

	class CSPCharacter*				GetSPCharacter();
};

#endif