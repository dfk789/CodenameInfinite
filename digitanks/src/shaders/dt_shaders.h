#ifndef DT_DT_SHADERS_H
#define DT_DT_SHADERS_H

#define TANKMOVEMENT \
	"	if (bMovement)" \
	"	{" \
	"		float flTankDistanceSqr = LengthSqr(vecPosition - vecTankOrigin);" \
	"		if (flTankDistanceSqr <= flMoveDistance*flMoveDistance)" \
	"		{" \
	"			float flMoveColorStrength = RemapVal(flTankDistanceSqr, 0.0, flMoveDistance*flMoveDistance, 0.0, 1.0);" \
	"			flMoveColorStrength = Lerp(flMoveColorStrength, 0.2);" \
	"			vecBaseColor = vecBaseColor * (1.0-flMoveColorStrength) + vec4(0.8, 0.8, 0.0, 1.0) * flMoveColorStrength;" \
	"		}" \
	"	}" \

#endif