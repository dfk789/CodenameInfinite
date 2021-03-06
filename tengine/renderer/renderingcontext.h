#ifndef TINKER_RENDERINGCONTEXT_H
#define TINKER_RENDERINGCONTEXT_H

#include <tstring.h>
#include <tengine_config.h>
#include <plane.h>
#include <matrix.h>
#include <color.h>

#include "render_common.h"

class CRenderingContext
{
public:
							CRenderingContext(class CRenderer* pRenderer);
							~CRenderingContext();

public:
	void					Transform(const Matrix4x4& m);
	void					Translate(const Vector& vecTranslate);
	void					Rotate(float flAngle, Vector vecAxis);
	void					Scale(float flX, float flY, float flZ);
	void					ResetTransformations();

	void					SetBlend(blendtype_t eBlend);
	void					SetAlpha(float flAlpha) { m_flAlpha = flAlpha; };
	void					SetDepthMask(bool bDepthMask);
	void					SetDepthTest(bool bDepthTest);
	void					SetBackCulling(bool bCull);
	void					SetColorSwap(Color clrSwap);
	void					SetLighting(bool bLighting);

	float					GetAlpha() { return m_flAlpha; };
	blendtype_t				GetBlend() { return m_eBlend; };

	void					RenderModel(size_t iModel, class CModel* pCompilingModel = NULL);
	void					RenderSceneNode(class CModel* pModel, class CConversionScene* pScene, class CConversionSceneNode* pNode, class CModel* pCompilingModel);
	void					RenderMeshInstance(class CModel* pModel, class CConversionScene* pScene, class CConversionMeshInstance* pMeshInstance, class CModel* pCompilingModel);

	void					RenderSphere();

	void					RenderBillboard(const tstring& sTexture, float flRadius);

	void					UseFrameBuffer(const class CFrameBuffer* pBuffer);
	void					UseProgram(const tstring& sProgram);
	void					SetUniform(const char* pszName, int iValue);
	void					SetUniform(const char* pszName, float flValue);
	void					SetUniform(const char* pszName, const Vector& vecValue);
	void					SetUniform(const char* pszName, const Color& vecValue);
	void					BindTexture(const tstring& sName, int iChannel = 0);
	void					BindTexture(size_t iTexture, int iChannel = 0);
	void					SetColor(Color c);
	void					BeginRenderTris();
	void					BeginRenderQuads();
	void					BeginRenderDebugLines();
	void					TexCoord(float s, float t, int iChannel = 0);
	void					TexCoord(const Vector2D& v, int iChannel = 0);
	void					TexCoord(const DoubleVector2D& v, int iChannel = 0);
	void					TexCoord(const Vector& v, int iChannel = 0);
	void					TexCoord(const DoubleVector& v, int iChannel = 0);
	void					Normal(const Vector& v);
	void					Vertex(const Vector& v);
	void					RenderCallList(size_t iCallList);
	void					EndRender();

protected:
	void					PushAttribs();

public:
	CRenderer*				m_pRenderer;

	bool					m_bMatrixTransformations;
	bool					m_bBoundTexture;
	bool					m_bFBO;
	size_t					m_iProgram;
	class CShader*			m_pShader;
	bool					m_bAttribs;

	bool					m_bColorSwap;
	Color					m_clrSwap;

	blendtype_t				m_eBlend;
	float					m_flAlpha;

	int						m_iDrawMode;
	bool					m_bTexCoord;
	bool					m_bNormal;
	eastl::vector<Vector2D>	m_avecTexCoord;
	Vector					m_vecNormal;
	eastl::vector<eastl::vector<Vector2D> >	m_aavecTexCoords;
	eastl::vector<Vector>	m_avecNormals;
	eastl::vector<Vector>	m_avecVertices;
};

#endif
