#include "renderer.h"

#include <GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <assert.h>

#include <maths.h>
#include <simplex.h>

#include <modelconverter/convmesh.h>
#include <models/models.h>
#include <shaders/shaders.h>

CRenderingContext::CRenderingContext(CRenderer* pRenderer)
{
	m_pRenderer = pRenderer;

	m_bMatrixTransformations = false;
	m_bBoundTexture = false;
	m_bFBO = false;
	m_iProgram = 0;
	m_bAttribs = false;

	m_bColorSwap = false;

	m_eBlend = BLEND_NONE;
	m_flAlpha = 1;
}

CRenderingContext::~CRenderingContext()
{
	if (m_bMatrixTransformations)
		glPopMatrix();

	if (m_bBoundTexture)
		glBindTexture(GL_TEXTURE_2D, 0);

	if (m_bFBO)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_pRenderer->GetSceneBuffer()->m_iFB);
		glViewport(0, 0, (GLsizei)m_pRenderer->GetSceneBuffer()->m_iWidth, (GLsizei)m_pRenderer->GetSceneBuffer()->m_iHeight);
	}

	if (m_iProgram)
		glUseProgram(0);

	if (m_bAttribs)
		glPopAttrib();
}

void CRenderingContext::Transform(const Matrix4x4& m)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glMultMatrixf(m.Transposed());	// GL uses column major.
}

void CRenderingContext::Translate(Vector vecTranslate)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glTranslatef(vecTranslate.x, vecTranslate.y, vecTranslate.z);
}

void CRenderingContext::Rotate(float flAngle, Vector vecAxis)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glRotatef(flAngle, vecAxis.x, vecAxis.y, vecAxis.z);
}

void CRenderingContext::Scale(float flX, float flY, float flZ)
{
	if (!m_bMatrixTransformations)
	{
		m_bMatrixTransformations = true;
		glPushMatrix();
	}

	glScalef(flX, flY, flZ);
}

void CRenderingContext::SetBlend(blendtype_t eBlend)
{
	if (!m_bAttribs)
		PushAttribs();

	if (eBlend)
	{
		glEnable(GL_BLEND);

		if (eBlend == BLEND_ALPHA)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	m_eBlend = eBlend;
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	if (!m_bAttribs)
		PushAttribs();

	glDepthMask(bDepthMask);
}

void CRenderingContext::SetBackCulling(bool bCull)
{
	if (!m_bAttribs)
		PushAttribs();

	if (bCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void CRenderingContext::SetColorSwap(Color clrSwap)
{
	m_bColorSwap = true;
	m_clrSwap = clrSwap;
}

void CRenderingContext::RenderModel(size_t iModel, bool bNewCallList)
{
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);

	if (!pModel)
		return;

	if (pModel->m_bStatic && !bNewCallList)
	{
		glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);

		if (m_pRenderer->ShouldUseShaders())
		{
			GLuint iProgram = (GLuint)CShaderLibrary::GetModelProgram();
			glUseProgram(iProgram);

			GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
			glUniform1i(bDiffuse, true);

			GLuint iDiffuse = glGetUniformLocation(iProgram, "iDiffuse");
			glUniform1i(iDiffuse, 0);

			GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
			glUniform1f(flAlpha, m_flAlpha);

			GLuint bColorSwapInAlpha = glGetUniformLocation(iProgram, "bColorSwapInAlpha");
			glUniform1i(bColorSwapInAlpha, m_bColorSwap);

			if (m_bColorSwap)
			{
				GLuint vecColorSwap = glGetUniformLocation(iProgram, "vecColorSwap");
				Vector vecColor((float)m_clrSwap.r()/255, (float)m_clrSwap.g()/255, (float)m_clrSwap.b()/255);
				glUniform3fv(vecColorSwap, 1, vecColor);
			}

			glColor4f(255, 255, 255, 255);

			glCallList((GLuint)pModel->m_iCallList);

			glUseProgram(0);
		}
		else
		{
			if (m_bColorSwap)
				glColor4f(((float)m_clrSwap.r())/255, ((float)m_clrSwap.g())/255, ((float)m_clrSwap.b())/255, m_flAlpha);
			else
				glColor4f(255, 255, 255, m_flAlpha);

			glCallList((GLuint)pModel->m_iCallList);
		}

		glPopAttrib();
	}
	else
	{
		for (size_t i = 0; i < pModel->m_pScene->GetNumScenes(); i++)
			RenderSceneNode(pModel, pModel->m_pScene, pModel->m_pScene->GetScene(i), bNewCallList);
	}

	if (!bNewCallList && m_pRenderer->ShouldUseShaders())
		m_pRenderer->ClearProgram();
}

void CRenderingContext::RenderSceneNode(CModel* pModel, CConversionScene* pScene, CConversionSceneNode* pNode, bool bNewCallList)
{
	if (!pNode)
		return;

	if (!pNode->IsVisible())
		return;

	glPushMatrix();

	glMultMatrixf(pNode->m_mTransformations.Transposed());	// GL uses column major.

	for (size_t i = 0; i < pNode->GetNumChildren(); i++)
		RenderSceneNode(pModel, pScene, pNode->GetChild(i), bNewCallList);

	for (size_t m = 0; m < pNode->GetNumMeshInstances(); m++)
		RenderMeshInstance(pModel, pScene, pNode->GetMeshInstance(m), bNewCallList);

	glPopMatrix();
}

void CRenderingContext::RenderMeshInstance(CModel* pModel, CConversionScene* pScene, CConversionMeshInstance* pMeshInstance, bool bNewCallList)
{
	if (!pMeshInstance->IsVisible())
		return;

	glPushAttrib(GL_ENABLE_BIT|GL_CURRENT_BIT|GL_LIGHTING_BIT|GL_TEXTURE_BIT);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	for (size_t j = 0; j < pMesh->GetNumFaces(); j++)
	{
		size_t k;
		CConversionFace* pFace = pMesh->GetFace(j);

		if (pFace->m == ~0)
			continue;

		CConversionMaterial* pMaterial = NULL;
		CConversionMaterialMap* pConversionMaterialMap = pMeshInstance->GetMappedMaterial(pFace->m);

		if (pConversionMaterialMap)
		{
			if (!pConversionMaterialMap->IsVisible())
				continue;

			pMaterial = pScene->GetMaterial(pConversionMaterialMap->m_iMaterial);
			if (pMaterial && !pMaterial->IsVisible())
				continue;
		}

		bool bTexture = false;
		if (pMaterial)
		{
			GLuint iTexture = (GLuint)pModel->m_aiTextures[pConversionMaterialMap->m_iMaterial];
			glBindTexture(GL_TEXTURE_2D, iTexture);

			bTexture = !!iTexture;
		}
		else
			glBindTexture(GL_TEXTURE_2D, 0);

		if (!bNewCallList)
		{
			if (m_pRenderer->ShouldUseShaders())
			{
				GLuint iProgram = (GLuint)CShaderLibrary::GetModelProgram();
				glUseProgram(iProgram);

				GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
				glUniform1i(bDiffuse, bTexture);

				GLuint iDiffuse = glGetUniformLocation(iProgram, "iDiffuse");
				glUniform1i(iDiffuse, 0);

				GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
				glUniform1f(flAlpha, m_flAlpha);

				GLuint bColorSwapInAlpha = glGetUniformLocation(iProgram, "bColorSwapInAlpha");
				glUniform1i(bColorSwapInAlpha, m_bColorSwap);

				if (m_bColorSwap)
				{
					GLuint vecColorSwap = glGetUniformLocation(iProgram, "vecColorSwap");
					Vector vecColor((float)m_clrSwap.r()/255, (float)m_clrSwap.g()/255, (float)m_clrSwap.b()/255);
					glUniform3fv(vecColorSwap, 1, vecColor);
				}
			}
			else
			{
				if (m_bColorSwap)
					glColor4f(((float)m_clrSwap.r())/255, ((float)m_clrSwap.g())/255, ((float)m_clrSwap.b())/255, m_flAlpha);
				else
					glColor4f(pMaterial->m_vecDiffuse.x, pMaterial->m_vecDiffuse.y, pMaterial->m_vecDiffuse.z, m_flAlpha);
			}
		}
		glBegin(GL_POLYGON);

		for (k = 0; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(k);

			glTexCoord2fv(pMesh->GetUV(pVertex->vu));
			glVertex3fv(pMesh->GetVertex(pVertex->v));
		}

		glEnd();
	}

	glPopAttrib();
}

void CRenderingContext::RenderSphere()
{
	static GLUquadricObj* pQuadric = gluNewQuadric();

	gluSphere(pQuadric, 1, 20, 10);
}

void CRenderingContext::UseFrameBuffer(const CFrameBuffer* pBuffer)
{
	assert(m_pRenderer->ShouldUseFramebuffers());

	m_bFBO = true;
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)pBuffer->m_iFB);
	glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);
}

void CRenderingContext::UseProgram(size_t iProgram)
{
	assert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	m_iProgram = iProgram;
	glUseProgram((GLuint)iProgram);
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	assert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1i(iUniform, iValue);
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	assert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1f(iUniform, flValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector& vecValue)
{
	assert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Color& clrValue)
{
	assert(m_pRenderer->ShouldUseShaders());

	if (!m_pRenderer->ShouldUseShaders())
		return;

	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, Vector(clrValue));
}

void CRenderingContext::BindTexture(size_t iTexture)
{
	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	m_bBoundTexture = true;
}

void CRenderingContext::SetColor(Color c)
{
	if (!m_bAttribs)
		PushAttribs();

	glColor4ub(c.r(), c.g(), c.b(), (unsigned char)(c.a()*m_flAlpha));
}

void CRenderingContext::BeginRenderTris()
{
	glBegin(GL_TRIANGLES);
}

void CRenderingContext::BeginRenderQuads()
{
	glBegin(GL_QUADS);
}

void CRenderingContext::TexCoord(float s, float t)
{
	glTexCoord2f(s, t);
}

void CRenderingContext::TexCoord(const Vector& v)
{
	glTexCoord2fv(v);
}

void CRenderingContext::Vertex(const Vector& v)
{
	glVertex3fv(v);
}

void CRenderingContext::RenderCallList(size_t iCallList)
{
	glCallList((GLuint)iCallList);
}

void CRenderingContext::EndRender()
{
	glEnd();
}

void CRenderingContext::PushAttribs()
{
	m_bAttribs = true;
	// Push all the attribs we'll ever need. I don't want to have to worry about popping them in order.
	glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_CURRENT_BIT);
}

CFrameBuffer::CFrameBuffer()
{
	m_iMap = m_iDepth = m_iFB = 0;
}

CRopeRenderer::CRopeRenderer(CRenderer *pRenderer, size_t iTexture, Vector vecStart)
	: m_oContext(pRenderer)
{
	m_pRenderer = pRenderer;

	m_oContext.BindTexture(iTexture);
	m_vecLastLink = vecStart;
	m_bFirstLink = true;

	m_flWidth = 1;

	m_flTextureScale = 1;
	m_flTextureOffset = 0;

	m_bUseForward = false;

	m_oContext.SetBlend(BLEND_ADDITIVE);
	m_oContext.SetDepthMask(false);

	m_clrRope = Color(255, 255, 255, 255);
	m_oContext.BeginRenderQuads();
}

void CRopeRenderer::AddLink(Vector vecLink)
{
	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = m_pRenderer->GetCameraVector();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecRight = vecForward.Cross(vecUp)*(m_flWidth/2);

	float flAddV = (1/m_flTextureScale);

	m_oContext.SetColor(m_clrRope);

	if (!m_bFirstLink)
	{
		// Finish the previous link
		m_oContext.TexCoord(1, m_flTextureOffset+flAddV);
		m_oContext.Vertex(m_vecLastLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset+flAddV);
		m_oContext.Vertex(m_vecLastLink-vecRight);

		m_flTextureOffset += flAddV;
	}

	m_bFirstLink = false;

	// Start this link
	m_oContext.TexCoord(0, m_flTextureOffset);
	m_oContext.Vertex(m_vecLastLink-vecRight);
	m_oContext.TexCoord(1, m_flTextureOffset);
	m_oContext.Vertex(m_vecLastLink+vecRight);

	m_vecLastLink = vecLink;
}

void CRopeRenderer::Finish(Vector vecLink)
{
	Vector vecForward;
	if (m_bUseForward)
		vecForward = m_vecForward;
	else
		vecForward = m_pRenderer->GetCameraVector();

	Vector vecUp = (vecLink - m_vecLastLink).Normalized();
	Vector vecRight = vecForward.Cross(vecUp)*(m_flWidth/2);

	float flAddV = (1/m_flTextureScale);

	if (m_bFirstLink)
	{
		// Start the previous link
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}
	else
	{
		m_flTextureOffset += flAddV;

		// Finish the last link
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecRight);

		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink-vecRight);
		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(m_vecLastLink+vecRight);

		m_flTextureOffset += flAddV;

		m_oContext.TexCoord(1, m_flTextureOffset);
		m_oContext.Vertex(vecLink+vecRight);
		m_oContext.TexCoord(0, m_flTextureOffset);
		m_oContext.Vertex(vecLink-vecRight);
	}

	m_oContext.EndRender();
}

void CRopeRenderer::SetForward(Vector vecForward)
{
	m_bUseForward = true;
	m_vecForward = vecForward;
}

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	m_bHardwareSupportsFramebuffers = false;
	m_bHardwareSupportsFramebuffersTestCompleted = false;

	m_bUseFramebuffers = true;

	if (!HardwareSupportsFramebuffers())
		m_bUseFramebuffers = false;

	m_bUseShaders = true;

	m_bHardwareSupportsShaders = false;
	m_bHardwareSupportsShadersTestCompleted = false;

	if (!HardwareSupportsShaders())
		m_bUseShaders = false;
	else
		CShaderLibrary::CompileShaders();

	if (!CShaderLibrary::IsCompiled())
		m_bUseShaders = false;

	m_iWidth = iWidth;
	m_iHeight = iHeight;
}

void CRenderer::Initialize()
{
	if (ShouldUseFramebuffers())
	{
		m_oSceneBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, true, true);

		size_t iWidth = m_oSceneBuffer.m_iWidth;
		size_t iHeight = m_oSceneBuffer.m_iHeight;
		for (size_t i = 0; i < BLOOM_FILTERS; i++)
		{
			m_oBloom1Buffers[i] = CreateFrameBuffer(iWidth, iHeight, false, true);
			m_oBloom2Buffers[i] = CreateFrameBuffer(iWidth, iHeight, false, false);
			iWidth /= 2;
			iHeight /= 2;
		}

		m_oNoiseBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, false, false);

		CreateNoise();
	}
}

CFrameBuffer CRenderer::CreateFrameBuffer(size_t iWidth, size_t iHeight, bool bDepth, bool bLinear)
{
	assert(ShouldUseFramebuffers());

	if (!GLEW_ARB_texture_non_power_of_two)
	{
		// If non power of two textures are not supported, framebuffers the size of the screen will probably fuck up.
		// I don't know this for sure but I'm not taking any chances. If the extension isn't supported, roll those
		// framebuffer sizes up to the next power of two.
		iWidth--;
		iWidth |= iWidth >> 1;
		iWidth |= iWidth >> 2;
		iWidth |= iWidth >> 4;
		iWidth |= iWidth >> 8;
		iWidth |= iWidth >> 16;
		iWidth++;

		iHeight--;
		iHeight |= iHeight >> 1;
		iHeight |= iHeight >> 2;
		iHeight |= iHeight >> 4;
		iHeight |= iHeight >> 8;
		iHeight |= iHeight >> 16;
		iHeight++;
	}

	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bLinear?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bLinear?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)iWidth, (GLsizei)iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (bDepth)
	{
		glGenRenderbuffersEXT(1, &oBuffer.m_iDepth);
		glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
		glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)iWidth, (GLsizei)iHeight );
		glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );
	}

	glGenFramebuffersEXT(1, &oBuffer.m_iFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	if (bDepth)
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
		printf("Framebuffer not complete!\n");

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	oBuffer.m_iWidth = iWidth;
	oBuffer.m_iHeight = iHeight;

	return oBuffer;
}

void CRenderer::CreateNoise()
{
	CSimplexNoise n1(mtrand()+0);
	CSimplexNoise n2(mtrand()+1);
	CSimplexNoise n3(mtrand()+2);

	float flSpaceFactor1 = 0.1f;
	float flHeightFactor1 = 0.5f;
	float flSpaceFactor2 = flSpaceFactor1*3;
	float flHeightFactor2 = flHeightFactor1/3;
	float flSpaceFactor3 = flSpaceFactor2*3;
	float flHeightFactor3 = flHeightFactor2/3;

	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oNoiseBuffer.m_iFB);

	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glOrtho(0, m_iWidth, m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPointSize(1);

	glPushAttrib(GL_CURRENT_BIT);

	for (size_t x = 0; x < m_iWidth; x++)
	{
		for (size_t y = 0; y < m_iHeight; y++)
		{
			float flValue = 0.5f;
			flValue += n1.Noise(x*flSpaceFactor1, y*flSpaceFactor1) * flHeightFactor1;
			flValue += n2.Noise(x*flSpaceFactor2, y*flSpaceFactor2) * flHeightFactor2;
			flValue += n3.Noise(x*flSpaceFactor3, y*flSpaceFactor3) * flHeightFactor3;

			glBegin(GL_POINTS);
				glColor3f(flValue, flValue, flValue);
				glVertex2f((float)x, (float)y);
			glEnd();
		}
	}

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
}

void CRenderer::SetupFrame()
{
	if (ShouldUseFramebuffers())
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oSceneBuffer.m_iFB);
		glViewport(0, 0, (GLsizei)m_oSceneBuffer.m_iWidth, (GLsizei)m_oSceneBuffer.m_iHeight);
	}
	else
	{
		glReadBuffer(GL_BACK);
		glDrawBuffer(GL_BACK);
		glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	}

	glClear(GL_DEPTH_BUFFER_BIT);

	glColor4f(1, 1, 1, 1);
}

void CRenderer::DrawBackground()
{
	// First draw a nice faded gray background.
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glShadeModel(GL_SMOOTH);

	glBegin(GL_QUADS);
		glColor3ub(0, 0, 0);
		glVertex2f(-1.0f, 1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(-1.0f, -1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(1.0f, -1.0f);
		glColor3ub(0, 0, 0);
		glVertex2f(1.0f, 1.0f);
	glEnd();

	glPopAttrib();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void CRenderer::StartRendering()
{
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	gluPerspective(
			m_flCameraFOV,
			(float)m_iWidth/(float)m_iHeight,
			m_flCameraNear,
			m_flCameraFar
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );

	// Momentarily return the viewport to the window size. This is because if the scene buffer is not the same as the window size,
	// the viewport here will be the scene buffer size, but we need it to be the window size so we can do world/screen transformations.
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );
	glPopAttrib();

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

void CRenderer::FinishRendering()
{
	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	if (ShouldUseFramebuffers())
		glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT|GL_CURRENT_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
    glOrtho(0, m_iWidth, m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	RenderOffscreenBuffers();

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	if (ShouldUseFramebuffers())
		RenderMapFullscreen(m_oSceneBuffer.m_iMap);

	RenderFullscreenBuffers();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void CRenderer::RenderMapFullscreen(size_t iMap)
{
	if (GLEW_ARB_multitexture || GLEW_VERSION_1_3)
		glActiveTexture(GL_TEXTURE0);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

	if (ShouldUseFramebuffers())
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 1); glVertex2d(0, 0);
		glTexCoord2i(0, 0); glVertex2d(0, m_iHeight);
		glTexCoord2i(1, 0); glVertex2d(m_iWidth, m_iHeight);
		glTexCoord2i(1, 1); glVertex2d(m_iWidth, 0);
	glEnd();
}

void CRenderer::RenderMapToBuffer(size_t iMap, CFrameBuffer* pBuffer)
{
	assert(ShouldUseFramebuffers());

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, pBuffer->m_iWidth, pBuffer->m_iHeight, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)pBuffer->m_iFB);
	glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 1); glVertex2i(0, 0);
		glTexCoord2i(0, 0); glVertex2i(0, (GLint)pBuffer->m_iHeight);
		glTexCoord2i(1, 0); glVertex2i((GLint)pBuffer->m_iWidth, (GLint)pBuffer->m_iHeight);
		glTexCoord2i(1, 1); glVertex2i((GLint)pBuffer->m_iWidth, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

Vector CRenderer::GetCameraVector()
{
	return (m_vecCameraTarget - m_vecCameraPosition).Normalized();
}

void CRenderer::GetCameraVectors(Vector* pvecForward, Vector* pvecRight, Vector* pvecUp)
{
	Vector vecForward = GetCameraVector();
	Vector vecRight;

	if (pvecForward)
		(*pvecForward) = vecForward;

	if (pvecRight || pvecUp)
		vecRight = vecForward.Cross(Vector(0, 1, 0)).Normalized();

	if (pvecRight)
		(*pvecRight) = vecRight;

	if (pvecUp)
		(*pvecUp) = vecRight.Cross(vecForward).Normalized();
}

void CRenderer::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
}

void CRenderer::ClearProgram()
{
	assert(ShouldUseShaders());

	if (!ShouldUseShaders())
		return;

	glUseProgram(0);
}

void CRenderer::UseProgram(size_t i)
{
	assert(ShouldUseShaders());

	if (!ShouldUseShaders())
		return;

	glUseProgram(i);
}

Vector CRenderer::ScreenPosition(Vector vecWorld)
{
	GLdouble x, y, z;
	gluProject(
		vecWorld.x, vecWorld.y, vecWorld.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)m_iHeight - (float)y, (float)z);
}

Vector CRenderer::WorldPosition(Vector vecScreen)
{
	GLdouble x, y, z;
	gluUnProject(
		vecScreen.x, (float)m_iHeight - vecScreen.y, vecScreen.z,
		(GLdouble*)m_aiModelView, (GLdouble*)m_aiProjection, (GLint*)m_aiViewport,
		&x, &y, &z);
	return Vector((float)x, (float)y, (float)z);
}

bool CRenderer::HardwareSupportsFramebuffers()
{
	if (m_bHardwareSupportsFramebuffersTestCompleted)
		return m_bHardwareSupportsFramebuffers;

	m_bHardwareSupportsFramebuffersTestCompleted = true;

	if (!GLEW_EXT_framebuffer_object)
	{
		m_bHardwareSupportsFramebuffers = false;
		return false;
	}

	// Compile a test framebuffer. If it fails we don't support framebuffers.

	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffersEXT(1, &oBuffer.m_iDepth);
	glBindRenderbufferEXT( GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth );
	glRenderbufferStorageEXT( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512 );
	glBindRenderbufferEXT( GL_RENDERBUFFER, 0 );

	glGenFramebuffersEXT(1, &oBuffer.m_iFB);
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)oBuffer.m_iFB);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (GLuint)oBuffer.m_iDepth);
    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
	{
		glDeleteTextures(1, &oBuffer.m_iMap);
		glDeleteRenderbuffersEXT(1, &oBuffer.m_iDepth);
		glDeleteFramebuffersEXT(1, &oBuffer.m_iFB);
		m_bHardwareSupportsFramebuffers = false;
		return false;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glDeleteTextures(1, &oBuffer.m_iMap);
	glDeleteRenderbuffersEXT(1, &oBuffer.m_iDepth);
	glDeleteFramebuffersEXT(1, &oBuffer.m_iFB);

	m_bHardwareSupportsFramebuffers = true;
	return true;
}

bool CRenderer::HardwareSupportsShaders()
{
	if (m_bHardwareSupportsShadersTestCompleted)
		return m_bHardwareSupportsShaders;

	m_bHardwareSupportsShadersTestCompleted = true;

	if (!GLEW_ARB_fragment_program)
	{
		m_bHardwareSupportsShaders = false;
		return false;
	}

	if (!GLEW_VERSION_2_0)
	{
		m_bHardwareSupportsShaders = false;
		return false;
	}

	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"void main()"
		"{"
		"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
		"	gl_TexCoord[0] = gl_MultiTexCoord0;"
		"	gl_FrontColor = gl_Color;"
		"}";

	const char* pszFragmentShader =
		"void main(void)"
		"{"
		"	gl_FragColor = vec4(1.0,1.0,1.0,1.0);"
		"}";

	GLuint iVShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint iProgram = glCreateProgram();

	glShaderSource(iVShader, 1, &pszVertexShader, NULL);
	glCompileShader(iVShader);

	int iVertexCompiled;
	glGetShaderiv(iVShader, GL_COMPILE_STATUS, &iVertexCompiled);


	glShaderSource(iFShader, 1, &pszFragmentShader, NULL);
	glCompileShader(iFShader);

	int iFragmentCompiled;
	glGetShaderiv(iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);


	glAttachShader(iProgram, iVShader);
	glAttachShader(iProgram, iFShader);
	glLinkProgram(iProgram);

	int iProgramLinked;
	glGetProgramiv(iProgram, GL_LINK_STATUS, &iProgramLinked);


	if (iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE)
		m_bHardwareSupportsShaders = true;

	glDetachShader(iProgram, iVShader);
	glDetachShader(iProgram, iFShader);
	glDeleteShader(iVShader);
	glDeleteShader(iFShader);
	glDeleteProgram(iProgram);

	return m_bHardwareSupportsShaders;
}

size_t CRenderer::CreateCallList(size_t iModel)
{
	size_t iCallList = glGenLists(1);

	glNewList((GLuint)iCallList, GL_COMPILE);
	CRenderingContext c(NULL);
	c.RenderModel(iModel, true);
	glEndList();

	return iCallList;
}

size_t CRenderer::LoadTextureIntoGL(eastl::string16 sFilename, int iClamp)
{
	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(sFilename.c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(sFilename.c_str());

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Width & (ImageInfo.Width-1))
	{
		//assert(!"Image width is not power of 2.");
		ilDeleteImages(1, &iDevILId);
		return 0;
	}

	if (ImageInfo.Height & (ImageInfo.Height-1))
	{
		//assert(!"Image height is not power of 2.");
		ilDeleteImages(1, &iDevILId);
		return 0;
	}

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	if (iClamp && !GLEW_EXT_texture_edge_clamp)
		iClamp = 1;

	if (iClamp == 1)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else if (iClamp == 2)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	gluBuild2DMipmaps(GL_TEXTURE_2D,
		ilGetInteger(IL_IMAGE_BPP),
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		GL_UNSIGNED_BYTE,
		ilGetData());

	ilBindImage(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	ilDeleteImages(1, &iDevILId);

	return iGLId;
}

size_t CRenderer::LoadTextureData(eastl::string16 sFilename)
{
	if (!sFilename.length())
		return 0;

	ILuint iDevILId;
	ilGenImages(1, &iDevILId);
	ilBindImage(iDevILId);

	ILboolean bSuccess = ilLoadImage(sFilename.c_str());

	if (!bSuccess)
		bSuccess = ilLoadImage(sFilename.c_str());

	ILenum iError = ilGetError();

	if (!bSuccess)
		return 0;

	bSuccess = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	if (!bSuccess)
		return 0;

	ILinfo ImageInfo;
	iluGetImageInfo(&ImageInfo);

	if (ImageInfo.Origin == IL_ORIGIN_UPPER_LEFT)
		iluFlipImage();

	ilBindImage(0);

	return iDevILId;
}

Color* CRenderer::GetTextureData(size_t iTexture)
{
	ilBindImage(iTexture);
	Color* pclr = (Color*)ilGetData();
	ilBindImage(0);
	return pclr;
}

size_t CRenderer::GetTextureWidth(size_t iTexture)
{
	ilBindImage(iTexture);
	size_t iWidth = ilGetInteger(IL_IMAGE_WIDTH);
	ilBindImage(0);
	return iWidth;
}

size_t CRenderer::GetTextureHeight(size_t iTexture)
{
	ilBindImage(iTexture);
	size_t iHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	ilBindImage(0);
	return iHeight;
}

void CRenderer::UnloadTextureData(size_t iTexture)
{
	ilBindImage(0);
	ilDeleteImages(1, &iTexture);
}
