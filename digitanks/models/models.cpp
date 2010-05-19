#include "models.h"

#include <modelconverter/modelconverter.h>

CModelLibrary* CModelLibrary::s_pModelLibrary = NULL;
static CModelLibrary g_ModelLibrary = CModelLibrary();

CModelLibrary::CModelLibrary()
{
	s_pModelLibrary = this;
}

CModelLibrary::~CModelLibrary()
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		delete m_apModels[i];
	}

	s_pModelLibrary = NULL;
}

size_t CModelLibrary::AddModel(const wchar_t* pszFilename)
{
	size_t iModel = FindModel(pszFilename);
	if (iModel != ~0)
		return iModel;

	m_apModels.push_back(new CModel(pszFilename));
	return m_apModels.size()-1;
}

CModel* CModelLibrary::GetModel(size_t i)
{
	if (i >= m_apModels.size())
		return NULL;

	return m_apModels[i];
}

size_t CModelLibrary::FindModel(const wchar_t* pszFilename)
{
	for (size_t i = 0; i < m_apModels.size(); i++)
	{
		if (wcscmp(m_apModels[i]->m_sFilename.c_str(), pszFilename) == 0)
			return i;
	}

	return ~0;
}

CModel::CModel(const wchar_t* pszFilename)
{
	m_sFilename = pszFilename;
	m_pScene = new CConversionScene();
	CModelConverter c(m_pScene);
	c.ReadModel(pszFilename);
}

CModel::~CModel()
{
	delete m_pScene;
}
