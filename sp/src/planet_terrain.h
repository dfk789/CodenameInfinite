#ifndef SP_PLANET_TERRAIN_H
#define SP_PLANET_TERRAIN_H

#include <EASTL/map.h>

#include <quadtree.h>

#include "sp_common.h"

class CBranchData
{
public:
	CBranchData()
	{
		flHeight = 0;
		bRender = false;
		flScreenSize = 1;
		flLastScreenUpdate = -1;
		flLastPushPull = -1;
		iShouldRenderLastFrame = ~0;
		iLocalCharacterDotLastFrame = ~0;
		iRenderVectorsLastFrame = ~0;
		flRadiusMeters = 0;
		bCompletelyInsideFrustum = false;
	}

public:
	float				flHeight;
	bool				bRender;
	float				flScreenSize;
	CScalableFloat		flGlobalRadius;
	float				flRadiusMeters;
	float				flLastScreenUpdate;
	float				flLastPushPull;
	size_t				iShouldRenderLastFrame;
	bool				bShouldRender;
	size_t				iLocalCharacterDotLastFrame;
	float				flLocalCharacterDot;
	bool				bCompletelyInsideFrustum;

	size_t				iRenderVectorsLastFrame;
	CScalableVector		vecGlobalQuadCenter;
	DoubleVector		vec1;
	DoubleVector		vec2;
	DoubleVector		vec3;
	DoubleVector		vec4;
	Vector				vec1n;
	Vector				vec2n;
	Vector				vec3n;
	Vector				vec4n;
	DoubleVector2D		vecDetailMin;
	DoubleVector2D		vecDetailMax;
};

typedef CQuadTree<CBranchData, double> CTerrainQuadTree;
typedef CQuadTreeBranch<CBranchData, double> CTerrainQuadTreeBranch;
typedef CQuadTreeDataSource<CBranchData, double> CTerrainQuadTreeDataSource;

class CPlanetTerrain : public CTerrainQuadTree, public CTerrainQuadTreeDataSource
{
	friend class CPlanet;

public:
	CPlanetTerrain(class CPlanet* pPlanet, Vector vecDirection)
		: CTerrainQuadTree()
	{
		m_pPlanet = pPlanet;
		m_vecDirection = vecDirection;
	};

public:
	void						Init();

	void						Think();
	void						ThinkBranch(CTerrainQuadTreeBranch* pBranch);
	bool						ShouldPush(CTerrainQuadTreeBranch* pBranch);
	bool						ShouldPull(CTerrainQuadTreeBranch* pBranch);
	void						ProcessBranchRendering(CTerrainQuadTreeBranch* pBranch);

	void						Render(class CRenderingContext* c) const;
	void						RenderBranch(const CTerrainQuadTreeBranch* pBranch, class CRenderingContext* c) const;

	void						UpdateScreenSize(CTerrainQuadTreeBranch* pBranch);
	bool						ShouldRenderBranch(CTerrainQuadTreeBranch* pBranch);
	void						InitRenderVectors(CTerrainQuadTreeBranch* pBranch);
	void						CalcRenderVectors(CTerrainQuadTreeBranch* pBranch);
	float						GetLocalCharacterDot(CTerrainQuadTreeBranch* pBranch);

	virtual TemplateVector2D<double>	WorldToQuadTree(const CTerrainQuadTree* pTree, const DoubleVector& vecWorld) const;
	virtual DoubleVector		QuadTreeToWorld(const CTerrainQuadTree* pTree, const TemplateVector2D<double>& vecTree) const;
	virtual TemplateVector2D<double>	WorldToQuadTree(CTerrainQuadTree* pTree, const DoubleVector& vecWorld);
	virtual DoubleVector		QuadTreeToWorld(CTerrainQuadTree* pTree, const TemplateVector2D<double>& vecTree);
	virtual bool				ShouldBuildBranch(CTerrainQuadTreeBranch* pBranch, bool& bDelete);

	Vector						GetDirection() const { return m_vecDirection; }

protected:
	class CPlanet*				m_pPlanet;
	Vector						m_vecDirection;
	int							m_iBuildsThisFrame;
	eastl::map<scale_t, eastl::vector<CTerrainQuadTreeBranch*> >	m_apRenderBranches;
	bool						m_bOneQuad;
};

#endif
