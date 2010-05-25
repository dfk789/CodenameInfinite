#include "renderer.h"

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <maths.h>

#include <modelconverter/convmesh.h>
#include <models/models.h>
#include <shaders/shaders.h>

CRenderingContext::CRenderingContext()
{
	m_bMatrixTransformations = false;
	m_bBoundTexture = false;
	m_iProgram = 0;
	m_bAttribs = false;

	m_eBlend = BLEND_NONE;
	m_flAlpha = 1;
}

CRenderingContext::~CRenderingContext()
{
	if (m_bMatrixTransformations)
		glPopMatrix();

	if (m_bBoundTexture)
		glBindTexture(GL_TEXTURE_2D, 0);

	if (m_iProgram)
		glUseProgram(0);

	if (m_bAttribs)
		glPopAttrib();
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
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	if (!m_bAttribs)
		PushAttribs();

	glDepthMask(bDepthMask);
}

void CRenderingContext::RenderModel(size_t iModel, bool bNewCallList)
{
	CModel* pModel = CModelLibrary::Get()->GetModel(iModel);

	if (pModel->m_bStatic && !bNewCallList)
	{
		GLuint iProgram = (GLuint)CShaderLibrary::GetModelProgram();
		glUseProgram(iProgram);

		GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
		glUniform1i(bDiffuse, false);

		GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
		glUniform1f(flAlpha, m_flAlpha);

		glCallList((GLuint)pModel->m_iCallList);

		glUseProgram(0);
	}
	else
	{
		for (size_t i = 0; i < pModel->m_pScene->GetNumScenes(); i++)
			RenderSceneNode(pModel, pModel->m_pScene, pModel->m_pScene->GetScene(i), bNewCallList);
	}
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

	// It uses this color if the texture is missing.
	GLfloat flMaterialColor[] = {0.7f, 0.7f, 0.7f, m_flAlpha};

	glMaterialfv(GL_FRONT, GL_DIFFUSE, flMaterialColor);
	glColor4fv(flMaterialColor);

	CConversionMesh* pMesh = pMeshInstance->GetMesh();

	Vector vecDiffuse(0.7f, 0.7f, 0.7f);

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
			vecDiffuse = pMaterial->m_vecDiffuse;
			GLuint iTexture = (GLuint)pModel->m_aiTextures[pConversionMaterialMap->m_iMaterial];
			glBindTexture(GL_TEXTURE_2D, iTexture);

			bTexture = !!iTexture;
		}
		else
			glBindTexture(GL_TEXTURE_2D, 0);

		if (!bNewCallList)
		{
			GLuint iProgram = (GLuint)CShaderLibrary::GetModelProgram();
			glUseProgram(iProgram);

			GLuint bDiffuse = glGetUniformLocation(iProgram, "bDiffuse");
			glUniform1i(bDiffuse, bTexture);

			GLuint iDiffuse = glGetUniformLocation(iProgram, "iDiffuse");
			glUniform1i(iDiffuse, 0);

			GLuint flAlpha = glGetUniformLocation(iProgram, "flAlpha");
			glUniform1f(flAlpha, m_flAlpha);
		}

		glBegin(GL_POLYGON);

		for (k = 0; k < pFace->GetNumVertices(); k++)
		{
			CConversionVertex* pVertex = pFace->GetVertex(k);

			// Give the tank a little bit of manual shading since there's no lights.
			float flDarken = RemapVal(pMesh->GetNormal(pVertex->vn).Dot(Vector(0, 1, 0)), -1, 1, 0.3f, 1.0f);
			glColor4f(vecDiffuse.x * flDarken, vecDiffuse.y * flDarken, vecDiffuse.z * flDarken, m_flAlpha);
			glTexCoord2fv(pMesh->GetUV(pVertex->vu));
			glVertex3fv(pMesh->GetVertex(pVertex->v));
		}

		if (!bNewCallList)
			glUseProgram(0);

		glEnd();
	}
}

void CRenderingContext::UseProgram(size_t iProgram)
{
	m_iProgram = iProgram;
	glUseProgram((GLuint)iProgram);
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1i(iUniform, iValue);
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1f(iUniform, flValue);
}

void CRenderingContext::BindTexture(size_t iTexture)
{
	glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
	m_bBoundTexture = true;
}

void CRenderingContext::BeginRenderQuads()
{
	glBegin(GL_QUADS);
}

void CRenderingContext::TexCoord(float s, float t)
{
	glTexCoord2f(s, t);
}

void CRenderingContext::Vertex(const Vector& v)
{
	glVertex3fv(v);
}

void CRenderingContext::EndRender()
{
	glEnd();
}

void CRenderingContext::PushAttribs()
{
	m_bAttribs = true;
	// Push all the attribs we'll ever need. I don't want to have to worry about popping them in order.
	glPushAttrib(GL_ENABLE_BIT|GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

CFrameBuffer::CFrameBuffer()
{
	m_iMap = m_iDepth = m_iFB = 0;
}

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	m_iWidth = iWidth;
	m_iHeight = iHeight;

	m_oSceneBuffer = CreateFrameBuffer(m_iWidth, m_iHeight, true, true);

	for (size_t i = 0; i < BLOOM_FILTERS; i++)
	{
		m_oBloom1Buffers[i] = CreateFrameBuffer(iWidth, iHeight, false, true);
		m_oBloom2Buffers[i] = CreateFrameBuffer(iWidth, iHeight, false, false);
		iWidth /= 2;
		iHeight /= 2;
	}
}

CFrameBuffer CRenderer::CreateFrameBuffer(size_t iWidth, size_t iHeight, bool bDepth, bool bLinear)
{
	CFrameBuffer oBuffer;

	glGenTextures(1, &oBuffer.m_iMap);
	glBindTexture(GL_TEXTURE_2D, (GLuint)oBuffer.m_iMap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bLinear?GL_LINEAR:GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bLinear?GL_LINEAR:GL_NEAREST);
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

void CRenderer::SetupFrame()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER, (GLuint)m_oSceneBuffer.m_iFB);

	glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight);

	glClear(GL_DEPTH_BUFFER_BIT);
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

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);

	glShadeModel(GL_SMOOTH);

	glBegin(GL_QUADS);
		glColor3ub(20, 20, 20);
		glVertex2f(-1.0f, 1.0f);
		glColor3ub(10, 10, 10);
		glVertex2f(-1.0f, -1.0f);
		glColor3ub(20, 20, 20);
		glVertex2f(1.0f, -1.0f);
		glColor3ub(10, 10, 10);
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
	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluPerspective(
			44.0,
			(float)m_iWidth/(float)m_iHeight,
			10,
			1000.0
		);

	glMatrixMode(GL_MODELVIEW);

	glPushMatrix();
	glLoadIdentity();

	gluLookAt(m_vecCameraPosition.x, m_vecCameraPosition.y, m_vecCameraPosition.z,
		m_vecCameraTarget.x, m_vecCameraTarget.y, m_vecCameraTarget.z,
		0.0, 1.0, 0.0);

	glGetDoublev( GL_MODELVIEW_MATRIX, m_aiModelView );
	glGetDoublev( GL_PROJECTION_MATRIX, m_aiProjection );
	glGetIntegerv( GL_VIEWPORT, m_aiViewport );

	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
}

void DrawTexture(GLuint iTexture, float flScale = 1.0f);

void CRenderer::FinishRendering()
{
	glPopMatrix();
	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);

	glPushAttrib(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_ENABLE_BIT|GL_TEXTURE_BIT);

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

	// Use a bright-pass filter to catch only the bright areas of the image
	GLuint iBrightPass = (GLuint)CShaderLibrary::GetBrightPassProgram();
	glUseProgram(iBrightPass);

	GLint iSource = glGetUniformLocation(iBrightPass, "iSource");
    glUniform1i(iSource, 0);

	GLint flScale = glGetUniformLocation(iBrightPass, "flScale");
	glUniform1f(flScale, (float)1/BLOOM_FILTERS);

	GLint flBrightness = glGetUniformLocation(iBrightPass, "flBrightness");

	for (size_t i = 0; i < BLOOM_FILTERS; i++)
	{
		glUniform1f(flBrightness, 0.7f - 0.1f*i);
		RenderMapToBuffer(m_oSceneBuffer.m_iMap, &m_oBloom1Buffers[i]);
	}

	glUseProgram(0);

	RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
	RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

	RenderBloomPass(m_oBloom1Buffers, m_oBloom2Buffers, true);
	RenderBloomPass(m_oBloom2Buffers, m_oBloom1Buffers, false);

	glReadBuffer(GL_BACK);
	glDrawBuffer(GL_BACK);

	RenderMapFullscreen(m_oSceneBuffer.m_iMap);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	for (size_t i = 0; i < BLOOM_FILTERS; i++)
		RenderMapFullscreen(m_oBloom1Buffers[i].m_iMap);
	glDisable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();   

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();
}

void CRenderer::RenderMapFullscreen(size_t iMap)
{
	glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

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
	glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, (GLuint)iMap);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, (GLuint)pBuffer->m_iFB);
	glViewport(0, 0, (GLsizei)pBuffer->m_iWidth, (GLsizei)pBuffer->m_iHeight);

	glBegin(GL_QUADS);
		glTexCoord2i(0, 1); glVertex2i(0, 0);
		glTexCoord2i(0, 0); glVertex2i(0, (GLint)m_iHeight);
		glTexCoord2i(1, 0); glVertex2i((GLint)m_iWidth, (GLint)m_iHeight);
		glTexCoord2i(1, 1); glVertex2i((GLint)m_iWidth, 0);
	glEnd();
}

#define KERNEL_SIZE   3
//float aflKernel[KERNEL_SIZE] = { 5, 6, 5 };
float aflKernel[KERNEL_SIZE] = { 0.3125f, 0.375f, 0.3125f };

void CRenderer::RenderBloomPass(CFrameBuffer* apSources, CFrameBuffer* apTargets, bool bHorizontal)
{
	GLuint iBlur = (GLuint)CShaderLibrary::GetBlurProgram();
	glUseProgram(iBlur);

	GLint iSource = glGetUniformLocation(iBlur, "iSource");
    glUniform1i(iSource, 0);

	// Can't I get rid of this and hard code it into the shader?
	GLint aflCoefficients = glGetUniformLocation(iBlur, "aflCoefficients");
    glUniform1fv(aflCoefficients, KERNEL_SIZE, aflKernel);

    GLint flOffsetX = glGetUniformLocation(iBlur, "flOffsetX");
    glUniform1f(flOffsetX, 0);

	GLint flOffset = glGetUniformLocation(iBlur, "flOffsetY");
    glUniform1f(flOffset, 0);
    if (bHorizontal)
        flOffset = glGetUniformLocation(iBlur, "flOffsetX");

    // Perform the blurring.
    for (size_t i = 0; i < BLOOM_FILTERS; i++)
    {
		glUniform1f(flOffset, 1.2f / apSources[i].m_iWidth);
		RenderMapToBuffer(apSources[i].m_iMap, &apTargets[i]);
    }

	glUseProgram(0);
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
		vecRight = vecForward.Cross(Vector(0, 1, 0));

	if (pvecRight)
		(*pvecRight) = vecRight;

	if (pvecUp)
		(*pvecUp) = vecRight.Cross(vecForward);
}

void CRenderer::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
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

size_t CRenderer::CreateCallList(size_t iModel)
{
	size_t iCallList = glGenLists(1);

	glNewList((GLuint)iCallList, GL_COMPILE);
	CRenderingContext c;
	c.RenderModel(iModel, true);
	glEndList();

	return iCallList;
}

size_t CRenderer::LoadTextureIntoGL(std::wstring sFilename)
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

	GLuint iGLId;
	glGenTextures(1, &iGLId);
	glBindTexture(GL_TEXTURE_2D, iGLId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D,
		ilGetInteger(IL_IMAGE_BPP),
		ilGetInteger(IL_IMAGE_WIDTH),
		ilGetInteger(IL_IMAGE_HEIGHT),
		ilGetInteger(IL_IMAGE_FORMAT),
		GL_UNSIGNED_BYTE,
		ilGetData());

	ilDeleteImages(1, &iDevILId);

	return iGLId;
}
