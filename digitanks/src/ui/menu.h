#ifndef DT_MENU_H
#define DT_MENU_H

#include <common.h>
#include "glgui/glgui.h"

class CDockPanel : public glgui::CPanel
{
	DECLARE_CLASS(CDockPanel, glgui::CPanel);

public:
									CDockPanel();

public:
	virtual void					Destructor();

	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					SetDockedPanel(glgui::CPanel* pDock);

	void							SetBGColor(Color clrBG) { m_clrBackground = clrBG; };

protected:
	glgui::CPanel*					m_pDockedPanel;

	Color							m_clrBackground;
};

class CTutorialsPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CTutorialsPanel, glgui::CPanel);

public:
									CTutorialsPanel();

public:
	virtual void					Layout();

	EVENT_CALLBACK(CTutorialsPanel,	Basics);
	EVENT_CALLBACK(CTutorialsPanel,	Bases);

	EVENT_CALLBACK(CTutorialsPanel,	BasicsHint);
	EVENT_CALLBACK(CTutorialsPanel,	BasesHint);

protected:
	glgui::CButton*					m_pBasics;
	glgui::CButton*					m_pBases;
};

class CGamesPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CGamesPanel, glgui::CPanel);

public:
									CGamesPanel();

public:
	virtual void					Layout();

	EVENT_CALLBACK(CGamesPanel,	Artillery);
	EVENT_CALLBACK(CGamesPanel,	Strategy);
	EVENT_CALLBACK(CGamesPanel,	Load);

	EVENT_CALLBACK(CGamesPanel,	ArtilleryHint);
	EVENT_CALLBACK(CGamesPanel,	StrategyHint);

protected:
	glgui::CButton*					m_pArtillery;
	glgui::CButton*					m_pStrategy;

	glgui::CButton*					m_pLoad;

	CDockPanel*						m_pDockPanel;
};

class CMultiplayerPanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CMultiplayerPanel, glgui::CPanel);

public:
									CMultiplayerPanel();

public:
	virtual void					Layout();

	EVENT_CALLBACK(CMultiplayerPanel,	Connect);
	EVENT_CALLBACK(CMultiplayerPanel,	Artillery);
	EVENT_CALLBACK(CMultiplayerPanel,	Strategy);
	EVENT_CALLBACK(CMultiplayerPanel,	Load);

	EVENT_CALLBACK(CMultiplayerPanel,	ClientHint);
	EVENT_CALLBACK(CMultiplayerPanel,	HostHint);
	EVENT_CALLBACK(CMultiplayerPanel,	LoadHint);

protected:
	glgui::CButton*					m_pConnect;

	glgui::CButton*					m_pArtillery;
	glgui::CButton*					m_pStrategy;

	glgui::CButton*					m_pLoad;

	CDockPanel*						m_pDockPanel;
};

class CArtilleryGamePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CArtilleryGamePanel, glgui::CPanel);

public:
									CArtilleryGamePanel(bool bMultiplayer = false);

public:
	virtual void					Layout();

	EVENT_CALLBACK(CArtilleryGamePanel,	BeginGame);

protected:
	glgui::CScrollSelector<int>*	m_pDifficulty;
	glgui::CLabel*					m_pDifficultyLabel;

	glgui::CScrollSelector<int>*	m_pPlayers;
	glgui::CLabel*					m_pPlayersLabel;

	glgui::CScrollSelector<int>*	m_pTanks;
	glgui::CLabel*					m_pTanksLabel;

	glgui::CButton*					m_pBeginGame;
};

class CStrategyGamePanel : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CStrategyGamePanel, glgui::CPanel);

public:
									CStrategyGamePanel(bool bMultiplayer = false);

public:
	virtual void					Layout();

	EVENT_CALLBACK(CStrategyGamePanel,	BeginGame);

protected:
	glgui::CScrollSelector<int>*	m_pDifficulty;
	glgui::CLabel*					m_pDifficultyLabel;

	glgui::CScrollSelector<int>*	m_pPlayers;
	glgui::CLabel*					m_pPlayersLabel;

	glgui::CButton*					m_pBeginGame;
};

class CMainMenu : public glgui::CPanel, public glgui::IEventListener
{
	DECLARE_CLASS(CMainMenu, glgui::CPanel);

public:
									CMainMenu();

public:
	virtual void					Layout();
	virtual void					Paint(int x, int y, int w, int h);

	virtual void					SetVisible(bool bVisible);

	EVENT_CALLBACK(CMainMenu,		OpenTutorialsPanel);
	EVENT_CALLBACK(CMainMenu,		OpenGamesPanel);
	EVENT_CALLBACK(CMainMenu,		OpenMultiplayerPanel);
	EVENT_CALLBACK(CMainMenu,		Quit);

	CDockPanel*						GetDockPanel();

	virtual void					SetHint(const std::wstring &sHint);

protected:
	glgui::CButton*					m_pTutorial;
	glgui::CButton*					m_pPlay;
	glgui::CButton*					m_pMultiplayer;
	glgui::CButton*					m_pOptions;
	glgui::CButton*					m_pQuit;

	glgui::CLabel*					m_pHint;

	CDockPanel*						m_pDockPanel;

	size_t							m_iLunarWorkshop;
	size_t							m_iDigitanks;
};

#endif
