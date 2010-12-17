#include "terrain.h"

#include <simplex.h>
#include <maths.h>
#include <time.h>
#include <strutils.h>

#include <GL/glew.h>

#include <raytracer/raytracer.h>

#include "dt_renderer.h"
#include "digitanksgame.h"
#include "digitankslevel.h"
#include <digitanks/weapons/projectile.h>
#include "ui/digitankswindow.h"
#include "shaders/shaders.h"

using namespace raytrace;

NETVAR_TABLE_BEGIN(CTerrain);
NETVAR_TABLE_END();

SAVEDATA_TABLE_BEGIN(CTerrain);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, bool, m_bHeightsInitialized);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flHighest);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flLowest);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iThinkChunkX);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, int, m_iThinkChunkY);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, float, m_flNextThink);
	//SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, size_t, m_iCallList);
	//raytrace::CRaytracer*	m_pTracer;	// Regenerated procedurally
	SAVEDATA_DEFINE(CSaveData::DATA_COPYTYPE, Vector, m_vecTerrainColor);
	SAVEDATA_DEFINE(CSaveData::DATA_COPYARRAY, Vector, m_avecQuadMods);
	//SAVEDATA_DEFINE(CSaveData::DATA_COPYARRAY, CTerrainChunk, m_aTerrainChunks);	// Onserialize
SAVEDATA_TABLE_END();

size_t CTerrain::s_iTreeTexture = 0;

CTerrain::CTerrain()
{
	SetCollisionGroup(CG_TERRAIN);

	for (size_t x = 0; x < TERRAIN_CHUNKS; x++)
	{
		for (size_t y = 0; y < TERRAIN_CHUNKS; y++)
		{
			m_aTerrainChunks[x][y].x = x;
			m_aTerrainChunks[x][y].y = y;
		}
	}
}

CTerrain::~CTerrain()
{
}

void CTerrain::Precache()
{
	BaseClass::Spawn();
	s_iTreeTexture = CRenderer::LoadTextureIntoGL(L"textures/tree.png", 1);
}

void CTerrain::Spawn()
{
	BaseClass::Spawn();

	switch (mtrand()%4)
	{
	case 0:
		m_vecTerrainColor = Vector(0.40f, 0.41f, 0.55f);
		m_avecQuadMods[0] = Vector(0.0f, 0.01f, 0.0f);
		m_avecQuadMods[1] = Vector(0.0f, 0.0f, 0.0f);
		m_avecQuadMods[2] = Vector(0.01f, 0.01f, 0.0f);
		m_avecQuadMods[3] = Vector(0.01f, 0.0f, 0.0f);
		break;

	case 1:
		m_vecTerrainColor = Vector(0.55f, 0.47f, 0.28f);
		m_avecQuadMods[0] = Vector(0.0f, 0.0f, 0.0f);
		m_avecQuadMods[1] = Vector(0.0f, -0.02f, 0.0f);
		m_avecQuadMods[2] = Vector(0.03f, 0.0f, 0.0f);
		m_avecQuadMods[3] = Vector(0.03f, 0.02f, 0.0f);
		break;

	case 2:
		m_vecTerrainColor = Vector(0.28f, 0.55f, 0.47f);
		m_avecQuadMods[0] = Vector(0.0f, 0.0f, 0.0f);
		m_avecQuadMods[1] = Vector(0.0f, 0.0f, -0.02f);
		m_avecQuadMods[2] = Vector(0.0f, 0.03f, 0.0f);
		m_avecQuadMods[3] = Vector(0.0f, 0.03f, 0.02f);
		break;

	case 3:
		m_vecTerrainColor = Vector(0.55f, 0.25f, 0.27f);
		m_avecQuadMods[0] = Vector(0.0f, 0.0f, 0.0f);
		m_avecQuadMods[1] = Vector(0.0f, 0.0f, 0.02f);
		m_avecQuadMods[2] = Vector(0.03f, 0.0f, 0.0f);
		m_avecQuadMods[3] = Vector(0.03f, 0.0f, -0.02f);
		break;
	}

	m_bHeightsInitialized = false;

	m_iThinkChunkX = 0;
	m_iThinkChunkY = 0;
	m_flNextThink = 0;
}

void CTerrain::Think()
{
	BaseClass::Think();

	if (GameServer()->GetGameTime() - m_flNextThink > 0.0f)
	{
		++m_iThinkChunkY %= TERRAIN_CHUNKS;
		if (m_iThinkChunkY == 0)
			++m_iThinkChunkX %= TERRAIN_CHUNKS;

		CTerrainChunk* pChunk = GetChunk(m_iThinkChunkX, m_iThinkChunkY);

		pChunk->Think();

		m_flNextThink = GameServer()->GetGameTime() + 0.1f;
	}
}

void CTerrain::GenerateTerrain(float flHeight)
{
	gamesettings_t* pGameSettings = DigitanksGame()->GetGameSettings();

	CDigitanksLevel* pLevel = NULL;
	size_t iTerrainData = 0;
	Color* pclrTerrainData = NULL;
	if (pGameSettings->iLevel < GameServer()->GetNumLevels())
	{
		pLevel = dynamic_cast<CDigitanksLevel*>(CDigitanksGame::GetLevel(DigitanksGame()->GetGameType(), pGameSettings->iLevel));
		if (pLevel)
		{
			iTerrainData = CRenderer::LoadTextureData(convertstring<char, char16_t>(pLevel->GetTerrainData()));

			if (CRenderer::GetTextureHeight(iTerrainData) != 200)
			{
				CRenderer::UnloadTextureData(iTerrainData);
				iTerrainData = 0;
			}

			if (CRenderer::GetTextureWidth(iTerrainData) != 200)
			{
				CRenderer::UnloadTextureData(iTerrainData);
				iTerrainData = 0;
			}

			if (iTerrainData)
				pclrTerrainData = CRenderer::GetTextureData(iTerrainData);
		}
	}

	CSimplexNoise n1(m_iSpawnSeed);
	CSimplexNoise n2(m_iSpawnSeed+1);
	CSimplexNoise n3(m_iSpawnSeed+2);
	CSimplexNoise n4(m_iSpawnSeed+3);
	CSimplexNoise n5(m_iSpawnSeed+4);

	float flSpaceFactor1 = 0.01f;
	float flHeightFactor1 = flHeight;
	float flSpaceFactor2 = flSpaceFactor1*3;
	float flHeightFactor2 = flHeightFactor1/3;
	float flSpaceFactor3 = flSpaceFactor2*3;
	float flHeightFactor3 = flHeightFactor2/3;
	float flSpaceFactor4 = flSpaceFactor3*3;
	float flHeightFactor4 = flHeightFactor3/3;
	float flSpaceFactor5 = flSpaceFactor4*3;
	float flHeightFactor5 = flHeightFactor4/3;

	CSimplexNoise h1(m_iSpawnSeed+5);
	CSimplexNoise h2(m_iSpawnSeed+6);

	CSimplexNoise t1(m_iSpawnSeed+7);
	CSimplexNoise t2(m_iSpawnSeed+8);

	float aflHoles[TERRAIN_SIZE][TERRAIN_SIZE];
	float flHoleHighest, flHoleLowest;

	float aflTrees[TERRAIN_SIZE][TERRAIN_SIZE];
	float flTreeHighest, flTreeLowest;

	for (size_t x = 0; x < TERRAIN_SIZE; x++)
	{
		for (size_t y = 0; y < TERRAIN_SIZE; y++)
		{
			int i, j;
			CTerrainChunk* pChunk = GetChunk(ArrayToChunkSpace(x, i), ArrayToChunkSpace(y, j));

			float flHeight;
			if (pclrTerrainData)
			{
				flHeight = (float)(pclrTerrainData[x*TERRAIN_SIZE+y].g())/255*pLevel->GetMaxHeight();
			}
			else
			{
				flHeight  = n1.Noise(x*flSpaceFactor1, y*flSpaceFactor1) * flHeightFactor1;
				flHeight += n2.Noise(x*flSpaceFactor2, y*flSpaceFactor2) * flHeightFactor2;
				flHeight += n3.Noise(x*flSpaceFactor3, y*flSpaceFactor3) * flHeightFactor3;
				flHeight += n4.Noise(x*flSpaceFactor4, y*flSpaceFactor4) * flHeightFactor4;
				flHeight += n5.Noise(x*flSpaceFactor5, y*flSpaceFactor5) * flHeightFactor5;
			}
			SetRealHeight(x, y, flHeight);

			if (!m_bHeightsInitialized)
				m_flHighest = m_flLowest = flHeight;

			if (flHeight < m_flLowest)
				m_flLowest = flHeight;

			if (flHeight > m_flHighest)
				m_flHighest = flHeight;

			pChunk->m_aflLava[i][j] = 0.0f;

			if (pclrTerrainData)
			{
				float flLava = (float)(pclrTerrainData[x*TERRAIN_SIZE+y].r())/255;
				SetBit(x, y, TB_LAVA, flLava > 0.5f);
				pChunk->m_aflLava[i][j] = 1.0f;

				float flHole = (float)(pclrTerrainData[x*TERRAIN_SIZE+y].a())/255;
				SetBit(x, y, TB_HOLE, flHole < 0.5f);
			}
			else if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
			{
				aflHoles[x][y]  = h1.Noise(x*0.01f, y*0.01f) * 10;
				aflHoles[x][y] += h2.Noise(x*0.02f, y*0.02f) * 5;

				if (!m_bHeightsInitialized)
					flHoleHighest = flHoleLowest = aflHoles[x][y];

				if (aflHoles[x][y] < flHoleLowest)
					flHoleLowest = aflHoles[x][y];

				if (aflHoles[x][y] > flHoleHighest)
					flHoleHighest = aflHoles[x][y];
			}

			aflTrees[x][y]  = t1.Noise(x*0.02f, y*0.02f) * 5;
			aflTrees[x][y] += t2.Noise(x*0.04f, y*0.04f) * 2;

			if (!m_bHeightsInitialized)
				flTreeHighest = flTreeLowest = aflTrees[x][y];

			if (aflTrees[x][y] < flTreeLowest)
				flTreeLowest = aflTrees[x][y];

			if (aflTrees[x][y] > flTreeHighest)
				flTreeHighest = aflTrees[x][y];

			m_bHeightsInitialized = true;
		}
	}

	if (iTerrainData)
		CRenderer::UnloadTextureData(iTerrainData);

	if (!pclrTerrainData)
	{
		float flLavaHeight = RemapVal(LavaHeight(), 0.0f, 1.0f, m_flLowest, m_flHighest);

		for (size_t x = 0; x < TERRAIN_SIZE; x++)
		{
			for (size_t y = 0; y < TERRAIN_SIZE; y++)
			{
				int i, j;
				CTerrainChunk* pChunk = GetChunk(ArrayToChunkSpace(x, i), ArrayToChunkSpace(y, j));

				float flHeight = RemapVal(GetRealHeight(x, y), m_flLowest, m_flHighest, 0.0f, 1.0f);
				SetBit(x, y, TB_LAVA, flHeight < LavaHeight());

				if (flHeight < LavaHeight())
				{
					SetRealHeight(x, y, flLavaHeight);
					pChunk->m_aflLava[i][j] = 1.0f;
				}

				if (DigitanksGame()->GetGameType() == GAMETYPE_ARTILLERY)
				{
					flHeight = RemapVal(aflHoles[x][y], flHoleLowest, flHoleHighest, 0.0f, 1.0f);
					SetBit(x, y, TB_HOLE, flHeight < HoleHeight());
				}

				if (GetBit(x, y, TB_HOLE) || GetBit(x, y, TB_LAVA))
					SetBit(x, y, TB_TREE, false);
				else
				{
					flHeight = RemapVal(aflTrees[x][y], flTreeLowest, flTreeHighest, 0.0f, 1.0f);
					SetBit(x, y, TB_TREE, flHeight > TreeHeight());
				}
			}
		}
	}
}

void CTerrain::GenerateCollision()
{
	assert(m_bHeightsInitialized);

	// Don't need the collision mesh in the menu
	if (DigitanksGame()->GetGameType() != GAMETYPE_MENU)
	{
		for (size_t x = 0; x < TERRAIN_CHUNKS; x++)
		{
			for (size_t y = 0; y < TERRAIN_CHUNKS; y++)
			{
				CTerrainChunk* pChunk = &m_aTerrainChunks[x][y];

				if (pChunk->m_pTracer)
					delete pChunk->m_pTracer;

				pChunk->m_pTracer = new raytrace::CRaytracer();

				for (size_t i = 0; i < TERRAIN_CHUNK_SIZE; i++)
				{
					for (size_t j = 0; j < TERRAIN_CHUNK_SIZE; j++)
					{
						float flX = ChunkToWorldSpace(x, i);
						float flY = ChunkToWorldSpace(y, j);
						float flX1 = ChunkToWorldSpace(x, i+1);
						float flY1 = ChunkToWorldSpace(y, j+1);

						int ax = ChunkToArraySpace(x, i);
						int ay = ChunkToArraySpace(y, j);
						int ax1 = ChunkToArraySpace(x, i+1);
						int ay1 = ChunkToArraySpace(y, j+1);

						if (GetBit(ax, ay, TB_HOLE))
							continue;

						Vector v1 = Vector(flX, GetRealHeight(ax, ay), flY);
						Vector v2 = Vector(flX, GetRealHeight(ax, ay1), flY1);
						Vector v3 = Vector(flX1, GetRealHeight(ax1, ay1), flY1);
						Vector v4 = Vector(flX1, GetRealHeight(ax1, ay), flY);

						pChunk->m_pTracer->AddTriangle(v1, v2, v3);
						pChunk->m_pTracer->AddTriangle(v1, v3, v4);
					}
				}

				pChunk->m_bNeedsRegenerate = true;
				pChunk->m_pTracer->BuildTree();
			}
		}
	}

	GenerateCallLists();
}

void CTerrain::GenerateTerrainCallLists()
{
	// Break it up into sectors of smaller size so that when it comes time to regenerate,
	// it can be done only for the sector that needs it and it won't take too long
	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			GenerateTerrainCallList(i, j);
		}
	}
}

void CTerrain::GenerateTerrainCallList(int i, int j)
{
	CTerrainChunk* pChunk = &m_aTerrainChunks[i][j];
	if (!pChunk->m_bNeedsRegenerate)
		return;

	// What a hack!
	GameServer()->GetRenderer()->UseProgram(CShaderLibrary::GetTerrainProgram());
	GLuint iTree = glGetAttribLocation((GLuint)CShaderLibrary::GetTerrainProgram(), "flTree");
	GameServer()->GetRenderer()->UseProgram(0);

	glNewList((GLuint)pChunk->m_iCallList, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_QUADS);

	for (int x = TERRAIN_CHUNK_SIZE*i; x < TERRAIN_CHUNK_SIZE*(i+1); x++)
	{
		if (x >= TERRAIN_SIZE-1)
			continue;

		float flUVY0 = RemapVal((float)x, (float)TERRAIN_CHUNK_SIZE*i, (float)TERRAIN_CHUNK_SIZE*(i+1), 0, 1);
		float flUVY1 = RemapVal((float)x+1, (float)TERRAIN_CHUNK_SIZE*i, (float)TERRAIN_CHUNK_SIZE*(i+1), 0, 1);

		float flX = ArrayToWorldSpace((int)x);
		float flX1 = ArrayToWorldSpace((int)x+1);

		float flVisibilityX0 = RemapValClamped((float)x, (float)TERRAIN_CHUNK_SIZE*i, (float)TERRAIN_CHUNK_SIZE*(i+1), pChunk->m_aflTerrainVisibility[0][0], pChunk->m_aflTerrainVisibility[1][0]);
		float flVisibilityX1 = RemapValClamped((float)x, (float)TERRAIN_CHUNK_SIZE*i, (float)TERRAIN_CHUNK_SIZE*(i+1), pChunk->m_aflTerrainVisibility[0][1], pChunk->m_aflTerrainVisibility[1][1]);

		for (int y = TERRAIN_CHUNK_SIZE*j; y < TERRAIN_CHUNK_SIZE*(j+1); y++)
		{
			if (y >= TERRAIN_SIZE-1)
				continue;

			if (GetBit(x, y, TB_HOLE))
				continue;

			float flColor = RemapVal(GetRealHeight(x, y), m_flLowest, m_flHighest, 0.0f, 0.98f);

			float flY = ArrayToWorldSpace((int)y);
			float flY1 = ArrayToWorldSpace((int)y+1);

			float flUVX0 = RemapVal((float)y, (float)TERRAIN_CHUNK_SIZE*j, (float)TERRAIN_CHUNK_SIZE*(j+1), 0, 1);
			float flUVX1 = RemapVal((float)y+1, (float)TERRAIN_CHUNK_SIZE*j, (float)TERRAIN_CHUNK_SIZE*(j+1), 0, 1);

			Vector vecColor;

			if (GetBit(x, y, TB_LAVA))
				vecColor = Vector(1,1,1);
			else
				vecColor = Vector(flColor, flColor, flColor);

			float flVisibility = RemapValClamped((float)y, (float)TERRAIN_CHUNK_SIZE*j, (float)TERRAIN_CHUNK_SIZE*(j+1), flVisibilityX0, flVisibilityX1);

			if (GameServer()->GetRenderer()->ShouldUseShaders())
			{
				glVertexAttrib1f(iTree, GetBit(x, y, TB_TREE)?1.0f:0.0f);
			}

			glColor3fv((vecColor + m_avecQuadMods[0]) * flVisibility);
			glTexCoord2f(flUVX0, flUVY0);
			glVertex3f(flX, GetRealHeight(x, y), flY);

			glColor3fv((vecColor + m_avecQuadMods[1]) * flVisibility);
			glTexCoord2f(flUVX1, flUVY0);
			glVertex3f(flX, GetRealHeight(x, y+1), flY1);

			glColor3fv((vecColor + m_avecQuadMods[2]) * flVisibility);
			glTexCoord2f(flUVX1, flUVY1);
			glVertex3f(flX1, GetRealHeight(x+1, y+1), flY1);

			glColor3fv((vecColor + m_avecQuadMods[3]) * flVisibility);
			glTexCoord2f(flUVX0, flUVY1);
			glVertex3f(flX1, GetRealHeight(x+1, y), flY);
		}
	}

	glEnd();
	glPopAttrib();
	glEndList();

	Vector vecTopX = Vector(0.3f, 0.3f, 0.3f);
	Vector vecTopY = Vector(0.28f, 0.28f, 0.28f);
	Vector vecBottom = Vector(0, 0, 0);
	float flBottom = -1000.0f;

	glNewList((GLuint)pChunk->m_iWallList, GL_COMPILE);
	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_QUADS);

	for (int x = TERRAIN_CHUNK_SIZE*i; x < TERRAIN_CHUNK_SIZE*(i+1); x++)
	{
		float flX = ArrayToWorldSpace((int)x);
		float flX1 = ArrayToWorldSpace((int)x+1);

		for (int y = TERRAIN_CHUNK_SIZE*j; y < TERRAIN_CHUNK_SIZE*(j+1); y++)
		{
			float flY = ArrayToWorldSpace((int)y);
			float flY1 = ArrayToWorldSpace((int)y+1);

			if (x == 0 && !GetBit(x, y, TB_HOLE) && y != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y+1), flY1);

				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY1);
			}

			if (y == 0 && !GetBit(x, y, TB_HOLE) && x != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopY);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecTopY);
				glVertex3f(flX1, GetRealHeight(x+1, y), flY);

				glColor3fv(vecBottom);
				glVertex3f(flX1, flBottom, flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);
			}

			if (x == TERRAIN_SIZE-1 && !GetBit(x-1, y, TB_HOLE) && y != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y+1), flY1);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY1);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);
			}

			if (y == TERRAIN_SIZE-1 && !GetBit(x, y-1, TB_HOLE) && x != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopY);
				glVertex3f(flX1, GetRealHeight(x+1, y), flY);

				glColor3fv(vecTopY);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);

				glColor3fv(vecBottom);
				glVertex3f(flX1, flBottom, flY);
			}

			if (!GetBit(x, y, TB_HOLE) && GetBit(x-1, y, TB_HOLE) && y != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y+1), flY1);

				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY1);
			}

			if (!GetBit(x, y, TB_HOLE) && GetBit(x, y-1, TB_HOLE) && x != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopY);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecTopY);
				glVertex3f(flX1, GetRealHeight(x+1, y), flY);

				glColor3fv(vecBottom);
				glVertex3f(flX1, flBottom, flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);
			}

			if (GetBit(x, y, TB_HOLE) && !GetBit(x-1, y, TB_HOLE) && x > 0 && x != TERRAIN_SIZE-1 && y != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecTopX);
				glVertex3f(flX, GetRealHeight(x, y+1), flY1);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY1);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);
			}

			if (GetBit(x, y, TB_HOLE) && !GetBit(x, y-1, TB_HOLE) && y > 0 && x != TERRAIN_SIZE-1 && y != TERRAIN_SIZE-1)
			{
				glColor3fv(vecTopY);
				glVertex3f(flX1, GetRealHeight(x+1, y), flY);

				glColor3fv(vecTopY);
				glVertex3f(flX, GetRealHeight(x, y), flY);

				glColor3fv(vecBottom);
				glVertex3f(flX, flBottom, flY);

				glColor3fv(vecBottom);
				glVertex3f(flX1, flBottom, flY);
			}
		}
	}

	glEnd();
	glPopAttrib();
	glEndList();

	glNewList((GLuint)pChunk->m_iTransparentCallList, GL_COMPILE);
	glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(false);
	glBindTexture(GL_TEXTURE_2D, s_iTreeTexture);
	glBegin(GL_QUADS);

	float flXSize = 2;
	float flZSize = 8;

	for (int x = TERRAIN_CHUNK_SIZE*i; x < TERRAIN_CHUNK_SIZE*(i+1); x++)
	{
		if (x >= TERRAIN_SIZE-1)
			continue;

		float flX = (ArrayToWorldSpace((int)x) + ArrayToWorldSpace((int)x+1))/2;

		float flVisibilityX0 = RemapValClamped((float)x, (float)TERRAIN_CHUNK_SIZE*i, (float)TERRAIN_CHUNK_SIZE*(i+1), pChunk->m_aflTerrainVisibility[0][0], pChunk->m_aflTerrainVisibility[1][0]);
		float flVisibilityX1 = RemapValClamped((float)x, (float)TERRAIN_CHUNK_SIZE*i, (float)TERRAIN_CHUNK_SIZE*(i+1), pChunk->m_aflTerrainVisibility[0][1], pChunk->m_aflTerrainVisibility[1][1]);

		for (int y = TERRAIN_CHUNK_SIZE*j; y < TERRAIN_CHUNK_SIZE*(j+1); y++)
		{
			float flY = (ArrayToWorldSpace((int)y) + ArrayToWorldSpace((int)y+1))/2;

			float flVisibility = RemapValClamped((float)y, (float)TERRAIN_CHUNK_SIZE*j, (float)TERRAIN_CHUNK_SIZE*(j+1), flVisibilityX0, flVisibilityX1);

			if (flVisibility < 0.01f)
				continue;

			glColor3fv(Vector(flVisibility, flVisibility, flVisibility));

			float flHeight = GetHeight(flX, flY);
			if (GetBit(x, y, TB_TREE))
			{
				glTexCoord2f(0, 0);
				glVertex3f(flX - flXSize, flHeight-1, flY);

				glTexCoord2f(0, 1);
				glVertex3f(flX - flXSize, flHeight-1 + flZSize, flY);

				glTexCoord2f(1, 1);
				glVertex3f(flX + flXSize, flHeight-1 + flZSize, flY);

				glTexCoord2f(1, 0);
				glVertex3f(flX + flXSize, flHeight-1, flY);


				glTexCoord2f(0, 0);
				glVertex3f(flX, flHeight-1, flY - flXSize);

				glTexCoord2f(0, 1);
				glVertex3f(flX, flHeight-1 + flZSize, flY - flXSize);

				glTexCoord2f(1, 1);
				glVertex3f(flX, flHeight-1 + flZSize, flY + flXSize);

				glTexCoord2f(1, 0);
				glVertex3f(flX, flHeight-1, flY + flXSize);
			}
		}
	}

	glEnd();
	glPopAttrib();
	glEndList();

	for (int a = 0; a < TERRAIN_CHUNK_TEXTURE_SIZE; a++)
	{
		for (int b = 0; b < TERRAIN_CHUNK_TEXTURE_SIZE; b++)
		{
			int x = TERRAIN_CHUNK_SIZE*i + a * TERRAIN_CHUNK_SIZE / TERRAIN_CHUNK_TEXTURE_SIZE;
			int y = TERRAIN_CHUNK_SIZE*j + b * TERRAIN_CHUNK_SIZE / TERRAIN_CHUNK_TEXTURE_SIZE;

			if (GetBit(x, y, TB_LAVA))
			{
				Color clrLava = Color(255 - RandomInt(0, 20), RandomInt(0, 20), RandomInt(0, 20));
				Color clrLava2 = Color(214 + RandomInt(-10, 10), 55 + RandomInt(-10, 10), RandomInt(0, 20));

				float flRealHeight = GetRealHeight(x, y);
				float flLavaHeight = RemapVal(LavaHeight(), 0, 1, m_flLowest, m_flHighest);
				float flLerp = RemapValClamped(flRealHeight, m_flLowest, flLavaHeight, 1.0f, 0.0f);

				float flRamp = Lerp(RandomFloat(0, 1), flLerp);
				pChunk->m_aclrTexture[a][b] = Color((int)(clrLava.r()*flRamp + clrLava2.r()*(1-flRamp)), (int)(clrLava.g()*flRamp + clrLava2.g()*(1-flRamp)), (int)(clrLava.b()*flRamp + clrLava2.b()*(1-flRamp)));
			}
			else
				pChunk->m_aclrTexture[a][b] = m_vecTerrainColor;
		}
	}

	if (pChunk->m_iChunkTexture)
		glDeleteTextures(1, &pChunk->m_iChunkTexture);
	glGenTextures(1, &pChunk->m_iChunkTexture);
	glBindTexture(GL_TEXTURE_2D, (GLuint)pChunk->m_iChunkTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TERRAIN_CHUNK_TEXTURE_SIZE, TERRAIN_CHUNK_TEXTURE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)pChunk->m_aclrTexture[0][0]);
	glBindTexture(GL_TEXTURE_2D, 0);

	pChunk->m_bNeedsRegenerate = false;
}

void CTerrain::GenerateCallLists()
{
	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			CTerrainChunk* pChunk = &m_aTerrainChunks[i][j];

			if (pChunk->m_iCallList)
				glDeleteLists((GLuint)pChunk->m_iCallList, 1);
			pChunk->m_iCallList = glGenLists(1);

			if (pChunk->m_iTransparentCallList)
				glDeleteLists((GLuint)pChunk->m_iTransparentCallList, 1);
			pChunk->m_iTransparentCallList = glGenLists(1);

			if (pChunk->m_iWallList)
				glDeleteLists((GLuint)pChunk->m_iWallList, 1);
			pChunk->m_iWallList = glGenLists(1);
		}
	}

	GenerateTerrainCallLists();
}

void CTerrain::ClearArea(Vector vecCenter, float flRadius)
{
	int iRadius = WorldToArraySpace(flRadius)-WorldToArraySpace(0)+1;

	int iX = WorldToArraySpace(vecCenter.x);
	int iZ = WorldToArraySpace(vecCenter.z);

	Vector vecOriginFlat = vecCenter;
	vecOriginFlat.y = 0;

	for (int x = iX-iRadius; x <= iX+iRadius; x++)
	{
		if (x < 0)
			continue;

		if (x >= TERRAIN_SIZE)
			continue;

		float flX = ArrayToWorldSpace(x);

		for (int z = iZ-iRadius; z <= iZ+iRadius; z++)
		{
			if (z < 0)
				continue;

			if (z >= TERRAIN_SIZE)
				continue;

			int i, j;
			CTerrainChunk* pChunk = GetChunk(ArrayToChunkSpace(x, i), ArrayToChunkSpace(z, j));
			if (!pChunk)
				continue;

			float flZ = ArrayToWorldSpace(z);

			if ((Vector(flX, 0, flZ) - vecOriginFlat).LengthSqr() < flRadius*flRadius)
			{
				SetBit(x, z, TB_LAVA, false);
				SetBit(x, z, TB_TREE, false);
				SetBit(x, z, TB_HOLE, false);

				pChunk->m_bNeedsRegenerate = true;
			}
		}
	}
}

void CTerrain::CalculateVisibility()
{
	if (!DigitanksGame()->ShouldRenderFogOfWar())
	{
		for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
		{
			for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
			{
				CTerrainChunk* pChunk = GetChunk((int)i, j);

				if (pChunk->m_aflTerrainVisibility[0][0] < 1.0f)
				{
					pChunk->m_aflTerrainVisibility[0][0] = 1.0f;
					pChunk->m_bNeedsRegenerate = true;
				}

				if (pChunk->m_aflTerrainVisibility[1][0] < 1.0f)
				{
					pChunk->m_aflTerrainVisibility[1][0] = 1.0f;
					pChunk->m_bNeedsRegenerate = true;
				}

				if (pChunk->m_aflTerrainVisibility[0][1] < 1.0f)
				{
					pChunk->m_aflTerrainVisibility[0][1] = 1.0f;
					pChunk->m_bNeedsRegenerate = true;
				}

				if (pChunk->m_aflTerrainVisibility[1][1] < 1.0f)
				{
					pChunk->m_aflTerrainVisibility[1][1] = 1.0f;
					pChunk->m_bNeedsRegenerate = true;
				}
			}
		}
		return;
	}

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	if (!pTeam)
		return;

	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		float flX0 = ChunkToWorldSpace(i, 0);
		float flX1 = ChunkToWorldSpace(i+1, 0);

		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			float flY0 = ChunkToWorldSpace(j, 0);
			float flY1 = ChunkToWorldSpace(j+1, 0);

			CTerrainChunk* pChunk = GetChunk((int)i, j);

			float flVisibility;

			flVisibility = pTeam->GetVisibilityAtPoint(SetPointHeight(Vector(flX0, 0, flY0)));
			if (fabs(pChunk->m_aflTerrainVisibility[0][0] - flVisibility) > 0.1f)
			{
				pChunk->m_aflTerrainVisibility[0][0] = flVisibility;
				pChunk->m_bNeedsRegenerate = true;
			}

			flVisibility = pTeam->GetVisibilityAtPoint(SetPointHeight(Vector(flX1, 0, flY0)));
			if (fabs(pChunk->m_aflTerrainVisibility[1][0] - flVisibility) > 0.1f)
			{
				pChunk->m_aflTerrainVisibility[1][0] = flVisibility;
				pChunk->m_bNeedsRegenerate = true;
			}

			flVisibility = pTeam->GetVisibilityAtPoint(SetPointHeight(Vector(flX0, 0, flY1)));
			if (fabs(pChunk->m_aflTerrainVisibility[0][1] - flVisibility) > 0.1f)
			{
				pChunk->m_aflTerrainVisibility[0][1] = flVisibility;
				pChunk->m_bNeedsRegenerate = true;
			}

			flVisibility = pTeam->GetVisibilityAtPoint(SetPointHeight(Vector(flX1, 0, flY1)));
			if (fabs(pChunk->m_aflTerrainVisibility[1][1] - flVisibility) > 0.1f)
			{
				pChunk->m_aflTerrainVisibility[1][1] = flVisibility;
				pChunk->m_bNeedsRegenerate = true;
			}
		}
	}

	GenerateTerrainCallLists();
}

void CTerrain::OnRender(CRenderingContext* pContext, bool bTransparent)
{
	if (bTransparent)
	{
		RenderTransparentTerrain();
		return;
	}

	BaseClass::OnRender(pContext, bTransparent);

	if (GameServer()->GetRenderer()->ShouldUseShaders())
		RenderWithShaders();
	else
		RenderWithoutShaders();
}

void CTerrain::RenderTransparentTerrain()
{
	glPushAttrib(GL_ENABLE_BIT);

	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			glCallList((GLuint)m_aTerrainChunks[i][j].m_iTransparentCallList);
		}
	}

	glPopAttrib();
}

void CTerrain::RenderWithShaders()
{
	glPushAttrib(GL_ENABLE_BIT);

	GLuint iTerrainProgram = (GLuint)CShaderLibrary::GetTerrainProgram();
	GameServer()->GetRenderer()->UseProgram(iTerrainProgram);

	CDigitank* pCurrentTank = DigitanksGame()->GetPrimarySelectionTank();

	bool bIsCurrentTeam = false;
	if (pCurrentTank && pCurrentTank->GetTeam() == DigitanksGame()->GetCurrentLocalDigitanksTeam())
		bIsCurrentTeam = true;

	if (bIsCurrentTeam && pCurrentTank && !pCurrentTank->IsFortified() && DigitanksGame()->GetControlMode() == MODE_MOVE)
	{
		GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
		glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

		GLuint flMoveDistance = glGetUniformLocation(iTerrainProgram, "flMoveDistance");
		glUniform1f(flMoveDistance, pCurrentTank->GetRemainingMovementDistance());

		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, true);
	}
	else if (bIsCurrentTeam && pCurrentTank && !pCurrentTank->IsFortified() && DigitanksGame()->GetControlMode() == MODE_AIM && DigitanksGame()->GetAimType() == AIM_MOVEMENT)
	{
		GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
		glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

		GLuint flMoveDistance = glGetUniformLocation(iTerrainProgram, "flMoveDistance");
		glUniform1f(flMoveDistance, pCurrentTank->ChargeRadius());

		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, true);
	}
	else
	{
		GLuint bMovement = glGetUniformLocation(iTerrainProgram, "bMovement");
		glUniform1i(bMovement, false);
	}

	if (bIsCurrentTeam && pCurrentTank && !pCurrentTank->IsFortified() && DigitanksGame()->GetControlMode() == MODE_TURN)
	{
		Vector vecPoint;
		bool bMouseOnGrid = DigitanksWindow()->GetMouseGridPosition(vecPoint);

		if (bMouseOnGrid)
		{
			GLuint vecTankOrigin = glGetUniformLocation(iTerrainProgram, "vecTankOrigin");
			glUniform3fv(vecTankOrigin, 1, pCurrentTank->GetOrigin());

			GLuint vecTurnPosition = glGetUniformLocation(iTerrainProgram, "vecTurnPosition");
			glUniform3fv(vecTurnPosition, 1, vecPoint);

			GLuint bTurnValid = glGetUniformLocation(iTerrainProgram, "bTurnValid");

			GLuint flTankYaw = glGetUniformLocation(iTerrainProgram, "flTankYaw");
			glUniform1f(flTankYaw, pCurrentTank->GetAngles().y);

			float flMaxTurnWithLeftoverPower = pCurrentTank->GetRemainingTurningDistance();

			GLuint flTankMaxYaw = glGetUniformLocation(iTerrainProgram, "flTankMaxYaw");
			glUniform1f(flTankMaxYaw, flMaxTurnWithLeftoverPower);

			Vector vecDirection = (vecPoint - pCurrentTank->GetOrigin()).Normalized();
			float flYaw = atan2(vecDirection.z, vecDirection.x) * 180/M_PI;

			float flTankTurn = AngleDifference(flYaw, pCurrentTank->GetAngles().y);

			if (!pCurrentTank->IsPreviewMoveValid())
				glUniform1i(bTurnValid, true);
			else if (fabs(flTankTurn)/pCurrentTank->TurnPerPower() > pCurrentTank->GetRemainingMovementEnergy())
				glUniform1i(bTurnValid, false);
			else
				glUniform1i(bTurnValid, true);
		}

		GLuint bTurning = glGetUniformLocation(iTerrainProgram, "bTurning");
		glUniform1i(bTurning, bMouseOnGrid);
	}
	else
	{
		GLuint bTurning = glGetUniformLocation(iTerrainProgram, "bTurning");
		glUniform1i(bTurning, false);
	}

	if (pCurrentTank && DigitanksGame()->GetAimType() == AIM_NORMAL)
	{
		GLuint bShowRanges = glGetUniformLocation(iTerrainProgram, "bShowRanges");
		glUniform1i(bShowRanges, true);

		GLuint bFocusRanges = glGetUniformLocation(iTerrainProgram, "bFocusRanges");
		glUniform1i(bFocusRanges, bIsCurrentTeam && DigitanksGame()->GetControlMode() == MODE_AIM);

		Vector vecRangeOrigin = pCurrentTank->GetOrigin();
		if (bIsCurrentTeam && DigitanksGame()->GetControlMode() == MODE_MOVE && pCurrentTank->GetPreviewMovePower() <= pCurrentTank->GetRemainingMovementEnergy())
			vecRangeOrigin = pCurrentTank->GetPreviewMove();

		GLuint vecTankPreviewOrigin = glGetUniformLocation(iTerrainProgram, "vecTankPreviewOrigin");
		glUniform3fv(vecTankPreviewOrigin, 1, vecRangeOrigin);

		GLuint flTankMaxRange = glGetUniformLocation(iTerrainProgram, "flTankMaxRange");
		glUniform1f(flTankMaxRange, pCurrentTank->GetMaxRange());

		GLuint flTankEffRange = glGetUniformLocation(iTerrainProgram, "flTankEffRange");
		glUniform1f(flTankEffRange, pCurrentTank->GetEffRange());

		GLuint flTankMinRange = glGetUniformLocation(iTerrainProgram, "flTankMinRange");
		glUniform1f(flTankMinRange, pCurrentTank->GetMinRange());

		GLuint flTankYaw = glGetUniformLocation(iTerrainProgram, "flTankYaw");
		if (bIsCurrentTeam && DigitanksGame()->GetControlMode() == MODE_TURN)
			glUniform1f(flTankYaw, pCurrentTank->GetPreviewTurn());
		else
			glUniform1f(flTankYaw, pCurrentTank->GetAngles().y);

		GLuint flTankFiringCone = glGetUniformLocation(iTerrainProgram, "flTankFiringCone");
		glUniform1f(flTankFiringCone, pCurrentTank->FiringCone());
	}
	else
	{
		GLuint bShowRanges = glGetUniformLocation(iTerrainProgram, "bShowRanges");
		glUniform1i(bShowRanges, false);
	}

	if (DigitanksGame()->GetAimType() == AIM_NORANGE)
	{
		GLuint iAimTargets = glGetUniformLocation(iTerrainProgram, "iAimTargets");
		glUniform1i(iAimTargets, 1);

		GLuint avecAimTargets = glGetUniformLocation(iTerrainProgram, "avecAimTargets");
		GLuint aflAimTargetRadius = glGetUniformLocation(iTerrainProgram, "aflAimTargetRadius");
		GLuint iFocusTarget = glGetUniformLocation(iTerrainProgram, "iFocusTarget");

		float flRadius = 50;
		glUniform3fv(avecAimTargets, 1, (float*)pCurrentTank->GetPreviewAim());
		glUniform1fv(aflAimTargetRadius, 1, &flRadius);

		glUniform1i(iFocusTarget, 0);
	}
	else if (DigitanksGame()->GetAimType() == AIM_NORMAL)
	{
		eastl::vector<Vector> avecTankAims;
		eastl::vector<float> aflTankAimRadius;
		size_t iTankAimFocus;

		DigitanksGame()->GetTankAims(avecTankAims, aflTankAimRadius, iTankAimFocus);
		DigitanksGame()->ClearTankAims();

		GLuint iAimTargets = glGetUniformLocation(iTerrainProgram, "iAimTargets");
		glUniform1i(iAimTargets, (GLint)avecTankAims.size());

		if (avecTankAims.size())
		{
			GLuint avecAimTargets = glGetUniformLocation(iTerrainProgram, "avecAimTargets");
			GLuint aflAimTargetRadius = glGetUniformLocation(iTerrainProgram, "aflAimTargetRadius");
			GLuint iFocusTarget = glGetUniformLocation(iTerrainProgram, "iFocusTarget");

			glUniform3fv(avecAimTargets, (GLint)avecTankAims.size(), avecTankAims[0]);
			glUniform1fv(aflAimTargetRadius, (GLint)aflTankAimRadius.size(), &aflTankAimRadius[0]);

			if (DigitanksGame()->GetControlMode() == MODE_AIM)
				glUniform1i(iFocusTarget, (GLint)iTankAimFocus);
			else
				glUniform1i(iFocusTarget, -1);
		}
	}
	else
	{
		GLuint iAimTargets = glGetUniformLocation(iTerrainProgram, "iAimTargets");
		GLuint avecAimTargets = glGetUniformLocation(iTerrainProgram, "avecAimTargets");
		GLuint aflAimTargetRadius = glGetUniformLocation(iTerrainProgram, "aflAimTargetRadius");
		GLuint iFocusTarget = glGetUniformLocation(iTerrainProgram, "iFocusTarget");

		glUniform1i(iAimTargets, 0);
		glUniform3fv(avecAimTargets, 1, Vector(0,0,0));
		glUniform1f(aflAimTargetRadius, 1.0f);
		glUniform1i(iFocusTarget, 0);
	}

	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			if (GLEW_ARB_multitexture || GLEW_VERSION_1_3)
				glActiveTexture(GL_TEXTURE0);

			glBindTexture(GL_TEXTURE_2D, m_aTerrainChunks[i][j].m_iChunkTexture);
			GLuint iDiffuse = glGetUniformLocation(iTerrainProgram, "iDiffuse");
			glUniform1i(iDiffuse, 0);
			glCallList((GLuint)m_aTerrainChunks[i][j].m_iCallList);
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	GameServer()->GetRenderer()->ClearProgram();

	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			glCallList((GLuint)m_aTerrainChunks[i][j].m_iWallList);
		}
	}

	glPopAttrib();
}

void CTerrain::RenderWithoutShaders()
{
	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			glBindTexture(GL_TEXTURE_2D, (GLuint)m_aTerrainChunks[i][j].m_iChunkTexture);
			glCallList((GLuint)m_aTerrainChunks[i][j].m_iCallList);
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			glCallList((GLuint)m_aTerrainChunks[i][j].m_iWallList);
		}
	}
}

void CTerrain::GetChunk(float x, float y, int& i, int& j)
{
	int iIndex;
	i = WorldToChunkSpace(x, iIndex);
	j = WorldToChunkSpace(y, iIndex);
}

CTerrainChunk* CTerrain::GetChunk(int x, int y)
{
	if (x >= TERRAIN_CHUNKS)
		return NULL;

	if (y >= TERRAIN_CHUNKS)
		return NULL;

	if (x < 0 || y < 0)
		return NULL;

	return &m_aTerrainChunks[x][y];
}

CTerrainChunk* CTerrain::GetChunk(float x, float y)
{
	int i, j;
	GetChunk(x, y, i, j);
	return GetChunk(i, j);
}

float CTerrain::GetRealHeight(int x, int y)
{
	if (x < 0)
		x = 0;
	if (x >= TERRAIN_SIZE)
		x = TERRAIN_SIZE-1;
	if (y < 0)
		y = 0;
	if (y >= TERRAIN_SIZE)
		y = TERRAIN_SIZE-1;

	int i, j;
	CTerrainChunk* pChunk = &m_aTerrainChunks[ArrayToChunkSpace(x, i)][ArrayToChunkSpace(y, j)];
	return pChunk->m_aflHeights[i][j];
}

void CTerrain::SetRealHeight(int x, int y, float flHeight)
{
	int iXIndex, iYIndex;
	int iChunkX = ArrayToChunkSpace(x, iXIndex);
	int iChunkY = ArrayToChunkSpace(y, iYIndex);

	CTerrainChunk* pChunk = GetChunk(iChunkX, iChunkY);

	pChunk->m_aflHeights[iXIndex][iYIndex] = flHeight;
}

float CTerrain::GetHeight(float flX, float flY)
{
	float flX2 = RemapVal(flX, -GetMapSize(), GetMapSize(), 0, TERRAIN_SIZE);
	float flY2 = RemapVal(flY, -GetMapSize(), GetMapSize(), 0, TERRAIN_SIZE);

	int iX = (int)flX2;
	int iY = (int)flY2;

	float a = GetRealHeight(iX, iY);
	float b = GetRealHeight(iX, iY+1);
	float c = GetRealHeight(iX+1, iY);
	float d = GetRealHeight(iX+1, iY+1);

	float flXLerp = fmod(flX2, 1);
	float flYLerp = fmod(flY2, 1);

	float l1 = RemapVal(flYLerp, 0, 1, a, b);
	float l2 = RemapVal(flYLerp, 0, 1, c, d);

	return RemapVal(flXLerp, 0, 1, l1, l2);
}

Vector CTerrain::SetPointHeight(Vector& vecPoint)
{
	vecPoint.y = GetHeight(vecPoint.x, vecPoint.z);
	return vecPoint;
}

float CTerrain::GetMapSize()
{
	return 200;
}

float CTerrain::ArrayToWorldSpace(int i)
{
	return RemapVal((float)i, 0, TERRAIN_SIZE, -GetMapSize(), GetMapSize());
}

int CTerrain::WorldToArraySpace(float f)
{
	return (int)RemapVal(f, -GetMapSize(), GetMapSize(), 0, TERRAIN_SIZE);
}

int CTerrain::ArrayToChunkSpace(int i, int& iIndex)
{
	iIndex = i%TERRAIN_CHUNK_SIZE;
	int iChunk = i/TERRAIN_CHUNK_SIZE;

	if (iIndex < 0)
		iIndex = 0;
	if (iIndex >= TERRAIN_CHUNK_SIZE)
		iIndex = TERRAIN_CHUNK_SIZE-1;

	if (iChunk < 0)
	{
		iIndex = 0;
		return 0;
	}

	if (iChunk >= TERRAIN_CHUNKS)
	{
		iIndex = TERRAIN_CHUNK_SIZE-1;
		return TERRAIN_CHUNKS-1;
	}

	return iChunk;
}

int CTerrain::ChunkToArraySpace(int iChunk, int i)
{
	return iChunk * TERRAIN_CHUNK_SIZE + i;
}

float CTerrain::ChunkToWorldSpace(int iChunk, int i)
{
	return ArrayToWorldSpace(ChunkToArraySpace(iChunk, i));
}

int CTerrain::WorldToChunkSpace(float f, int& iIndex)
{
	return ArrayToChunkSpace(WorldToArraySpace(f), iIndex);
}

bool CTerrain::IsPointOnMap(Vector vecPoint)
{
	if (vecPoint.x < -GetMapSize())
		return false;

	if (vecPoint.x > GetMapSize())
		return false;

	if (vecPoint.z < -GetMapSize())
		return false;

	if (vecPoint.z > GetMapSize())
		return false;

	return true;
}

bool CTerrain::IsPointOverLava(Vector vecPoint)
{
	return GetBit(WorldToArraySpace(vecPoint.x), WorldToArraySpace(vecPoint.z), TB_LAVA);
}

bool CTerrain::IsPointOverHole(Vector vecPoint)
{
	return GetBit(WorldToArraySpace(vecPoint.x), WorldToArraySpace(vecPoint.z), TB_HOLE);
}

void CTerrain::SetBit(int x, int y, terrainbit_t b, bool v)
{
	int x2, y2;
	int iChunkX = ArrayToChunkSpace(x, x2);
	int iChunkY = ArrayToChunkSpace(y, y2);

	CTerrainChunk* pChunk = GetChunk(iChunkX, iChunkY);
	if (!pChunk)
		return;

	if (v)
		pChunk->m_aiSpecialData[x2][y2] |= (1<<b);
	else
		pChunk->m_aiSpecialData[x2][y2] &= ~(1<<b);
}

bool CTerrain::GetBit(int x, int y, terrainbit_t b)
{
	int x2, y2;
	int iChunkX = ArrayToChunkSpace(x, x2);
	int iChunkY = ArrayToChunkSpace(y, y2);

	CTerrainChunk* pChunk = GetChunk(iChunkX, iChunkY);
	if (!pChunk)
		return false;

	return !!(pChunk->m_aiSpecialData[x2][y2] & (1<<b));
}

Vector CTerrain::GetNormalAtPoint(Vector vecPoint)
{
	Vector vecA = Vector(vecPoint.x, GetHeight(vecPoint.x, vecPoint.z), vecPoint.z);
	Vector vecB = Vector(vecPoint.x+1, GetHeight(vecPoint.x+1, vecPoint.z), vecPoint.z);
	Vector vecC = Vector(vecPoint.x, GetHeight(vecPoint.x, vecPoint.z+1), vecPoint.z+1);

	return (vecA-vecC).Normalized().Cross((vecA-vecB).Normalized()).Normalized();
}

bool CTerrain::Collide(const Vector& v1, const Vector& v2, Vector& vecPoint)
{
	vecPoint = v2;
	bool bReturn = false;
	for (int i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (int j = 0; j < TERRAIN_CHUNKS; j++)
		{
			CTerrainChunk* pChunk = GetChunk(i, j);
			if (pChunk->m_pTracer)
			{
				raytrace::CTraceResult tr;
				bool bHit = pChunk->m_pTracer->Raytrace(v1, v2, &tr);
				if (bHit)
				{
					if ((v1-tr.m_vecHit).LengthSqr() < (v1-vecPoint).LengthSqr())
						vecPoint = tr.m_vecHit;
					bReturn = true;
				}
			}
		}
	}

	return bReturn;
}

void CTerrain::TakeDamage(CBaseEntity* pAttacker, CBaseEntity* pInflictor, damagetype_t eDamageType, float flDamage, bool bDirectHit)
{
	if (eDamageType != DAMAGE_EXPLOSION)
		return;

	CBaseWeapon* pWeapon = dynamic_cast<CBaseWeapon*>(pInflictor);
	if (pWeapon && !pWeapon->CreatesCraters())
		return;

	float flRadius = 4.0f;
	if (pWeapon)
		flRadius = pWeapon->ExplosionRadius();

	int iRadius = WorldToArraySpace(flRadius)-WorldToArraySpace(0)+1;

	Vector vecOrigin = pInflictor->GetOrigin();

	if (!CNetwork::IsHost())
		return;

	int iX = WorldToArraySpace(vecOrigin.x);
	int iZ = WorldToArraySpace(vecOrigin.z);

	Vector vecOriginFlat = vecOrigin;
	vecOriginFlat.y = 0;

	for (int x = iX-iRadius; x <= iX+iRadius; x++)
	{
		if (x < 0)
			continue;

		if (x >= TERRAIN_SIZE)
			continue;

		for (int z = iZ-iRadius; z <= iZ+iRadius; z++)
		{
			if (z < 0)
				continue;

			if (z >= TERRAIN_SIZE)
				continue;

			float flX = ArrayToWorldSpace(x);
			float flZ = ArrayToWorldSpace(z);

			float flX1 = ArrayToWorldSpace((int)x+1);
			float flZ1 = ArrayToWorldSpace((int)z+1);

			if ((Vector(flX, 0, flZ) - vecOriginFlat).LengthSqr() < flRadius*flRadius)
			{
				float flXDistance = (flX - vecOriginFlat.x);
				float flZDistance = (flZ - vecOriginFlat.z);

				float flSqrt = sqrt(flRadius*flRadius - flXDistance*flXDistance - flZDistance*flZDistance);
				float flNewY = -flSqrt + vecOrigin.y;

				// As if the dirt from above drops down into the hole.
				float flAbove = GetRealHeight(x, z) - (flSqrt + vecOrigin.y);
				if (flAbove > 0)
					flNewY += flAbove;

				if (flNewY > GetRealHeight(x, z))
					continue;

				SetRealHeight(x, z, flNewY);

				int iIndex;
				int iChunkX = ArrayToChunkSpace(x, iIndex);
				int iChunkY = ArrayToChunkSpace(z, iIndex);

				if (RandomInt(0, 2) == 0)
					SetBit(x, z, TB_TREE, false);

				CTerrainChunk* pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;

				// Also regenerate nearby chunks which may have been affected.
				iChunkX = ArrayToChunkSpace(x-1, iIndex);
				iChunkY = ArrayToChunkSpace(z, iIndex);
				pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;

				iChunkX = ArrayToChunkSpace(x, iIndex);
				iChunkY = ArrayToChunkSpace(z-1, iIndex);
				pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;

				iChunkX = ArrayToChunkSpace(x-1, iIndex);
				iChunkY = ArrayToChunkSpace(z-1, iIndex);
				pChunk = GetChunk(iChunkX, iChunkY);
				if (pChunk && !pChunk->m_bNeedsRegenerate)
					pChunk->m_bNeedsRegenerate = true;
			}
		}
	}

	bool bTerrainDeformed = false;
	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			CTerrainChunk* pChunk = GetChunk((int)i, (int)j);
			if (pChunk->m_bNeedsRegenerate)
			{
				bTerrainDeformed = true;
				break;
			}
		}

		if (bTerrainDeformed)
			break;
	}

	if (!bTerrainDeformed)
		return;

	UpdateTerrainData();
}

Color CTerrain::GetPrimaryTerrainColor()
{
	return m_vecTerrainColor;
}

void CTerrain::UpdateTerrainData()
{
	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			CTerrainChunk* pChunk = GetChunk((int)i, (int)j);
			if (!pChunk->m_bNeedsRegenerate)
				continue;

			CNetworkParameters p;
			p.ui1 = i;
			p.ui2 = j;

			p.CreateExtraData(sizeof(float)*TERRAIN_CHUNK_SIZE*TERRAIN_CHUNK_SIZE);

			size_t iPosition = 0;
			float* flHeightData = (float*)p.m_pExtraData;

			// Serialize the height data
			for (int x = TERRAIN_CHUNK_SIZE*i; x < (int)(TERRAIN_CHUNK_SIZE*(i+1)); x++)
			{
				for (int z = TERRAIN_CHUNK_SIZE*j; z < (int)(TERRAIN_CHUNK_SIZE*(j+1)); z++)
					flHeightData[iPosition++] = GetRealHeight(x, z);
			}

			if (CNetwork::ShouldReplicateClientFunction())
				CNetwork::CallFunctionParameters(NETWORK_TOCLIENTS, "TerrainData", &p);

			TerrainData(&p);
		}
	}
}

void CTerrain::TerrainData(class CNetworkParameters* p)
{
	size_t i = p->ui1;
	size_t j = p->ui2;

	size_t iPosition = 0;
	float* flHeightData = (float*)p->m_pExtraData;

	CTerrainChunk* pChunk = GetChunk((int)i, j);

	// Unserialize the height data
	for (int x = TERRAIN_CHUNK_SIZE*i; x < (int)(TERRAIN_CHUNK_SIZE*(i+1)); x++)
	{
		for (int z = TERRAIN_CHUNK_SIZE*j; z < (int)(TERRAIN_CHUNK_SIZE*(j+1)); z++)
		{
			if (fabs(GetRealHeight(x, z) - flHeightData[iPosition]) > 0.01f)
			{
				pChunk->m_bNeedsRegenerate = true;
				SetRealHeight(x, z, flHeightData[iPosition]);

				float flHeight = flHeightData[iPosition];

				if (!m_bHeightsInitialized)
				{
					m_flHighest = m_flLowest = flHeight;
					m_bHeightsInitialized = true;
				}
				else
				{
					if (flHeight < m_flLowest)
						m_flLowest = flHeight;

					if (flHeight > m_flHighest)
						m_flHighest = flHeight;
				}
			}

			iPosition++;
		}
	}

	if (!pChunk->m_bNeedsRegenerate)
		return;

	if (pChunk->m_pTracer)
		delete pChunk->m_pTracer;

	pChunk->m_pTracer = new raytrace::CRaytracer();

	int iXMin = (int)(TERRAIN_CHUNK_SIZE*i);
	int iYMin = (int)(TERRAIN_CHUNK_SIZE*j);
	int iXMax = (int)(TERRAIN_CHUNK_SIZE*(i+1));
	int iYMax = (int)(TERRAIN_CHUNK_SIZE*(j+1));

	for (int x = iXMin; x < iXMax; x++)
	{
		for (int z = iYMin; z < iYMax; z++)
		{
			if (GetBit(x, z, TB_HOLE))
				continue;

			float flX = ArrayToWorldSpace(x);
			float flZ = ArrayToWorldSpace(z);

			float flX1 = ArrayToWorldSpace((int)x+1);
			float flZ1 = ArrayToWorldSpace((int)z+1);

			Vector v1 = Vector(flX, GetRealHeight(x, z), flZ);
			Vector v2 = Vector(flX, GetRealHeight(x, z+1), flZ1);
			Vector v3 = Vector(flX1, GetRealHeight(x+1, z+1), flZ1);
			Vector v4 = Vector(flX1, GetRealHeight(x+1, z), flZ);

			pChunk->m_pTracer->AddTriangle(v1, v2, v3);
			pChunk->m_pTracer->AddTriangle(v1, v3, v4);
		}
	}

	if (!GameServer()->IsLoading())
	{
		pChunk->m_pTracer->BuildTree();
		GenerateTerrainCallList(i, j);
	}
}

void CTerrain::ResyncClientTerrainData(int iClient)
{
	for (size_t i = 0; i < TERRAIN_CHUNKS; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNKS; j++)
		{
			CNetworkParameters p;
			p.ui1 = i;
			p.ui2 = j;

			p.CreateExtraData(sizeof(float)*TERRAIN_CHUNK_SIZE*TERRAIN_CHUNK_SIZE);

			size_t iPosition = 0;
			float* flHeightData = (float*)p.m_pExtraData;

			// Serialize the height data
			for (int x = TERRAIN_CHUNK_SIZE*i; x < (int)(TERRAIN_CHUNK_SIZE*(i+1)); x++)
			{
				for (int z = TERRAIN_CHUNK_SIZE*j; z < (int)(TERRAIN_CHUNK_SIZE*(j+1)); z++)
					flHeightData[iPosition++] = GetRealHeight(x, z);
			}

			CNetwork::CallFunctionParameters(iClient, "TerrainData", &p);
		}
	}
}

void CTerrain::OnSerialize(std::ostream& o)
{
	for (size_t x = 0; x < TERRAIN_CHUNKS; x++)
	{
		for (size_t y = 0; y < TERRAIN_CHUNKS; y++)
		{
			CTerrainChunk* pChunk = &m_aTerrainChunks[x][y];
			o.write((char*)&pChunk->m_aflHeights[0][0], sizeof(pChunk->m_aflHeights));
		}
	}

	BaseClass::OnSerialize(o);
}

bool CTerrain::OnUnserialize(std::istream& i)
{
	for (size_t x = 0; x < TERRAIN_CHUNKS; x++)
	{
		for (size_t y = 0; y < TERRAIN_CHUNKS; y++)
		{
			CTerrainChunk* pChunk = &m_aTerrainChunks[x][y];
			i.read((char*)&pChunk->m_aflHeights[0][0], sizeof(pChunk->m_aflHeights));
			pChunk->m_bNeedsRegenerate = true;
		}
	}

	return BaseClass::OnUnserialize(i);
}

void CTerrain::ClientEnterGame()
{
	BaseClass::ClientEnterGame();

	GenerateCollision();

	CalculateVisibility();
}

CTerrainChunk::CTerrainChunk()
{
	m_pTracer = NULL;
	m_iCallList = 0;
	m_iTransparentCallList = 0;
	m_iWallList = 0;
	m_bNeedsRegenerate = true;
	m_iChunkTexture = 0;

	memset(m_aiSpecialData, 0, sizeof(m_aiSpecialData));
	memset(m_aflTerrainVisibility, 0, sizeof(m_aflTerrainVisibility));
}

CTerrainChunk::~CTerrainChunk()
{
	if (m_iCallList)
		glDeleteLists((GLuint)m_iCallList, 1);
	if (m_iTransparentCallList)
		glDeleteLists((GLuint)m_iTransparentCallList, 1);
	if (m_iWallList)
		glDeleteLists((GLuint)m_iWallList, 1);
	if (m_iChunkTexture)
		glDeleteTextures(1, &m_iChunkTexture);
}

void CTerrainChunk::Think()
{
	for (size_t i = 0; i < TERRAIN_CHUNK_SIZE; i++)
	{
		for (size_t j = 0; j < TERRAIN_CHUNK_SIZE; j++)
		{
			if (m_aflLava[i][j] > 0)
			{
				// Flow into all lower neighboring squares.
				int a = CTerrain::ChunkToArraySpace(x, i);
				int b = CTerrain::ChunkToArraySpace(y, j);

				float flSourceHeight = m_aflHeights[i][j];

				for (int m = a-1; m <= a+1; m++)
				{
					if (m < 0 || m >= TERRAIN_SIZE)
						continue;

					for (int n = b-1; n <= b+1; n++)
					{
						if (n < 0 || n >= TERRAIN_SIZE)
							continue;

						float flDestinationHeight = DigitanksGame()->GetTerrain()->GetRealHeight(m, n);

						if (flDestinationHeight > flSourceHeight)
							continue;

						// This is getting out of hand.
						int k, l;
						CTerrainChunk* pChunk = DigitanksGame()->GetTerrain()->GetChunk(CTerrain::ArrayToChunkSpace(m, k), CTerrain::ArrayToChunkSpace(n, l));

						float flLava = m_aflLava[i][j] - 0.125f;

						if (pChunk->m_aflLava[k][l] <= 0 && flLava > 0.0f)
							pChunk->m_bNeedsRegenerate = true;

						if (pChunk->m_aflLava[k][l] < flLava)
						{
							pChunk->m_aflLava[k][l] = flLava;
							DigitanksGame()->GetTerrain()->SetBit(m, n, CTerrain::TB_LAVA, true);
						}
					}
				}
			}
		}
	}

	DigitanksGame()->GetTerrain()->GenerateTerrainCallLists();
}
