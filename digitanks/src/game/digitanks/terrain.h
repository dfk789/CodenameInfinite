#ifndef DT_TERRAIN_H
#define DT_TERRAIN_H

#include "baseentity.h"
#include "color.h"

#define TERRAIN_SIZE 200
#define TERRAIN_CHUNKS 10
#define TERRAIN_CHUNK_SIZE (TERRAIN_SIZE/TERRAIN_CHUNKS)

#define TERRAIN_CHUNK_TEXTURE_SIZE 256
#define TERRAIN_CHUNK_TEXELS_PER (TERRAIN_CHUNK_TEXTURE_SIZE/TERRAIN_CHUNK_SIZE)

class CTerrainChunk
{
public:
	friend class CTerrain;

public:
									CTerrainChunk();
	virtual							~CTerrainChunk();

public:
	void							Think();

protected:
	float							m_aflHeights[TERRAIN_CHUNK_SIZE][TERRAIN_CHUNK_SIZE];
	float							m_aflLava[TERRAIN_CHUNK_SIZE][TERRAIN_CHUNK_SIZE];
	class raytrace::CRaytracer*		m_pTracer;

	// A bit field
	unsigned char					m_aiSpecialData[TERRAIN_CHUNK_SIZE][TERRAIN_CHUNK_SIZE];

	Color							m_aclrTexture[TERRAIN_CHUNK_TEXTURE_SIZE][TERRAIN_CHUNK_TEXTURE_SIZE];
	size_t							m_iChunkTexture;

	size_t							m_iCallList;
	size_t							m_iTransparentCallList;
	size_t							m_iWallList;

	bool							m_bNeedsRegenerate;

	size_t							x, y;

	float							m_aflTerrainVisibility[2][2];
};

class CTerrain : public CBaseEntity
{
	REGISTER_ENTITY_CLASS(CTerrain, CBaseEntity);

public:
							CTerrain();
	virtual					~CTerrain();

public:
	virtual void			Precache();
	virtual void			Spawn();

	virtual void			Think();

	virtual float			GetBoundingRadius() const { return sqrt(GetMapSize()*GetMapSize() + GetMapSize()*GetMapSize()); };

	void					GenerateTerrain(float flHeight = 50);
	void					GenerateCollision();
	void					GenerateTerrainCallLists();
	void					GenerateTerrainCallList(int x, int y);
	void					GenerateCallLists();

	void					ClearArea(Vector vecCenter, float flRadius);

	void					CalculateVisibility();

	virtual bool			ShouldRender() const { return true; };
	virtual void			OnRender(class CRenderingContext* pContext, bool bTransparent);
	void					RenderTransparentTerrain();
	void					RenderWithShaders();
	void					RenderWithoutShaders();

	void					GetChunk(float x, float y, int& i, int& j);
	CTerrainChunk*			GetChunk(int x, int y);
	CTerrainChunk*			GetChunk(float x, float y);

	float					GetRealHeight(int x, int y);
	void					SetRealHeight(int x, int y, float h);
	float					GetHeight(float x, float y);
	Vector					SetPointHeight(Vector& vecPoint);
	static float			GetMapSize();
	float					ArrayToWorldSpace(int i);
	static int				WorldToArraySpace(float f);
	static int				ArrayToChunkSpace(int i, int& iIndex);
	static int				ChunkToArraySpace(int iChunk, int i);
	float					ChunkToWorldSpace(int iChunk, int i);
	int						WorldToChunkSpace(float f, int& iIndex);
	bool					IsPointOnMap(Vector vecPoint);
	bool					IsPointOverLava(Vector vecPoint);
	bool					IsPointOverHole(Vector vecPoint);

	typedef enum
	{
		TB_LAVA = 0,
		TB_HOLE = 1,
		TB_TREE = 2,
		// uses m_aiSpecialData which is unsigned char so max 8 of these.
	} terrainbit_t;
	void					SetBit(int x, int y, terrainbit_t b, bool v);
	bool					GetBit(int x, int y, terrainbit_t b);

	Vector					GetNormalAtPoint(Vector vecPoint);

	virtual bool			Collide(const Vector& v1, const Vector& v2, Vector& vecPoint);
	void					TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit);

	Color					GetPrimaryTerrainColor();

	void					UpdateTerrainData();
	void					TerrainData(class CNetworkParameters* p);
	void					ResyncClientTerrainData(int iClient);

	virtual void			OnSerialize(std::ostream& o);
	virtual bool			OnUnserialize(std::istream& i);

	virtual void			ClientEnterGame();

	float					LavaHeight() { return 0.2f; }
	float					HoleHeight() { return 0.25f; }
	float					TreeHeight() { return 0.65f; }

protected:
	bool					m_bHeightsInitialized;
	float					m_flHighest;
	float					m_flLowest;

	Vector					m_vecTerrainColor;
	Vector					m_avecQuadMods[4];

	CTerrainChunk			m_aTerrainChunks[TERRAIN_CHUNKS][TERRAIN_CHUNKS];

	float					m_flNextThink;
	int						m_iThinkChunkX;
	int						m_iThinkChunkY;

	static size_t			s_iTreeTexture;
};

#endif
