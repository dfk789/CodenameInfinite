#include "updatespanel.h"

#include <strutils.h>

#include <renderer/renderer.h>
#include <models/texturelibrary.h>

#include <game/digitanks/digitanksgame.h>
#include <ui/digitankswindow.h>
#include <ui/hud.h>

using namespace glgui;

CUpdatesPanel::CUpdatesPanel()
	: CPanel(0, 0, 600, 600)
{
	m_pCloseButton = new CButton(0, 0, 100, 20, L"Close");
	m_pCloseButton->SetClickedListener(this, Close);
	m_pCloseButton->SetFont(L"header");
	AddControl(m_pCloseButton);

	m_pInfo = new CLabel(0, 0, 100, 300, L"");
	m_pInfo->SetFont(L"text");
	AddControl(m_pInfo);

	m_pTutorial = new CLabel(0, 0, 100, 300, L"");
	m_pTutorial->SetFont(L"text");
	AddControl(m_pTutorial);
}

void CUpdatesPanel::Layout()
{
	CPanel::Layout();

	SetPos(CRootPanel::Get()->GetWidth()/2-GetWidth()/2, CRootPanel::Get()->GetHeight()/2-GetHeight()/2);

	m_pCloseButton->SetPos(GetWidth()-m_pCloseButton->GetWidth()-20, 20);

	m_pInfo->SetSize(200, 200);
	m_pInfo->SetPos(GetWidth()+2, GetHeight()/2 - m_pInfo->GetHeight()/2);

	m_pTutorial->SetSize(GetWidth(), 20);
	m_pTutorial->SetPos(0, GetHeight() - 30);

	for (size_t i = 0; i < m_apUpdates.size(); i++)
	{
		m_apUpdates[i]->Destructor();
		m_apUpdates[i]->Delete();
	}

	m_apUpdates.clear();

	if (!DigitanksGame() || !DigitanksGame()->GetUpdateGrid())
		return;

	CDigitanksTeam* pCurrentGame = DigitanksGame()->GetCurrentLocalDigitanksTeam();
	if (!pCurrentGame)
		return;

	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();

	int iLowestX = pUpdates->m_iLowestX;
	int iLowestY = pUpdates->m_iLowestY;

	int iXSize = pUpdates->m_iHighestX - iLowestX + 1;
	int iYSize = pUpdates->m_iHighestY - iLowestY + 1;

	int iLarger = ((iXSize > iYSize) ? iXSize : iYSize);

	if (iLarger < 11)
	{
		iLarger = 11;
		iLowestX = (pUpdates->m_iHighestX + pUpdates->m_iLowestX)/2 - iLarger/2;
		iLowestY = (pUpdates->m_iHighestY + pUpdates->m_iLowestY)/2 - iLarger/2;
	}

	m_iButtonSize = GetWidth()/(iLarger+2);
	int iXWidth = m_iButtonSize*iXSize;
	int iYWidth = m_iButtonSize*iYSize;
	int iXBuffer = (GetWidth() - iXWidth)/2;
	int iYBuffer = (GetWidth() - iYWidth)/2;

	for (int i = pUpdates->m_iLowestX; i <= pUpdates->m_iHighestX; i++)
	{
		for (int j = pUpdates->m_iLowestY; j <= pUpdates->m_iHighestY; j++)
		{
			if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_EMPTY)
				continue;

			m_apUpdates.push_back(new CUpdateButton(this));
			CUpdateButton* pUpdate = m_apUpdates[m_apUpdates.size()-1];
			pUpdate->SetSize(m_iButtonSize-2, m_iButtonSize-1);
			pUpdate->SetPos((i-iLowestX)*m_iButtonSize + iXBuffer, (j-iLowestY)*m_iButtonSize + iYBuffer);
			pUpdate->SetFont(L"text", 10);
			pUpdate->SetLocation(i, j);
			AddControl(pUpdate);

			bool bCanDownload = pCurrentGame->CanDownloadUpdate(i, j);
			bool bAlreadyDownloaded = pCurrentGame->HasDownloadedUpdate(i, j);

			if (bAlreadyDownloaded)
			{
				pUpdate->SetEnabled(false);
				pUpdate->SetFGColor(Color(0, 0, 0));
			}
			else
			{
				pUpdate->SetFGColor(Color(150, 150, 150));
				if (bCanDownload)
					pUpdate->SetClickedListener(pUpdate, &CUpdateButton::ChooseDownload);
				else
					pUpdate->SetEnabled(false);
			}

			size_t iSheet;
			int sx, sy, sw, sh, tw, th;
			GetTextureForUpdateItem(&pUpdates->m_aUpdates[i][j], iSheet, sx, sy, sw, sh, tw, th);
			pUpdate->SetSheetTexture(iSheet, sx, sy, sw, sh, tw, th);
			pUpdate->SetText(pUpdates->m_aUpdates[i][j].GetName().c_str());

			if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTURE)
			{
				Color clrButton = Color(50, 50, 255);
				if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_TANKLOADER)
					clrButton = Color(255, 255, 50);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_ARTILLERYLOADER)
					clrButton = Color(255, 255, 50);

				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(clrButton.r()/2, clrButton.g()/2, clrButton.b()/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/10, clrButton.g()/10, clrButton.b()/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/3, clrButton.g()/3, clrButton.b()/3));
					pUpdate->SetAlpha(200*2/3);
				}
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
			{
				Color clrButton = Color(200, 200, 200);
				if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_CPU)
					clrButton = Color(200, 200, 200);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_INFANTRYLOADER)
					clrButton = Color(200, 200, 150);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_TANKLOADER)
					clrButton = Color(200, 200, 150);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_ARTILLERYLOADER)
					clrButton = Color(200, 200, 150);
				else if (pUpdates->m_aUpdates[i][j].m_eStructure == STRUCTURE_BUFFER)
					clrButton = Color(150, 150, 200);

				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(clrButton.r()/2, clrButton.g()/2, clrButton.b()/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/10, clrButton.g()/10, clrButton.b()/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(0, 0, 0));
					pUpdate->SetAlpha(200*2/3);
				}
			}
			else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_UNITSKILL)
			{
				Color clrButton = Color(200, 0, 0);

				if (bAlreadyDownloaded)
					pUpdate->SetButtonColor(Color(clrButton.r()/2, clrButton.g()/2, clrButton.b()/2));
				else if (!bCanDownload)
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/10, clrButton.g()/10, clrButton.b()/10));
					pUpdate->SetAlpha(200/6);
				}
				else
				{
					pUpdate->SetButtonColor(Color(clrButton.r()/3, clrButton.g()/3, clrButton.b()/3));
					pUpdate->SetAlpha(200*2/3);
				}
			}

			if (pCurrentGame->IsDownloading(i, j))
			{
				if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTURE)
				{
					pUpdate->SetButtonColor(Color(250, 150, 150));
					pUpdate->SetFGColor(Color(50, 50, 50));
				}
				else if (pUpdates->m_aUpdates[i][j].m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
				{
					pUpdate->SetButtonColor(Color(250, 250, 250));
					pUpdate->SetFGColor(Color(50, 50, 50));
				}
			}
		}
	}

	UpdateInfo(NULL);
}

void CUpdatesPanel::Paint(int x, int y, int w, int h)
{
	CRootPanel::PaintRect(x, y, w, h, Color(0, 0, 0, GetAlpha()));

	int ix, iy, iw, ih;
	m_pInfo->GetAbsDimensions(ix, iy, iw, ih);
	CRootPanel::PaintRect(ix, iy, iw, ih, Color(0, 0, 0, GetAlpha()));

	CPanel::Paint(x, y, w, h);
}

void CUpdatesPanel::CloseCallback()
{
	SetVisible(false);
}

void CUpdatesPanel::UpdateInfo(CUpdateItem* pInfo)
{
	if (!pInfo)
	{
		m_pTutorial->SetText("");
		return;
	}

	CDigitanksTeam* pTeam = DigitanksGame()->GetCurrentLocalDigitanksTeam();

	int x, y;
	DigitanksGame()->GetUpdateGrid()->FindUpdate(pInfo, x, y);

	eastl::string16 s;
	eastl::string16 p;
	s += pInfo->GetName() + L"\n \n";
	s += pInfo->GetInfo() + L"\n \n";

	if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
		s += p.sprintf(L"Increase: %.1f %s\n", pInfo->m_flValue, pInfo->GetUnits());

	s += p.sprintf(L"Download size: %d\n", (int)pInfo->m_flSize);

	if (pTeam && pTeam->GetBandwidth() > 0 && !pTeam->HasDownloadedUpdate(x, y))
	{
		float flDownloaded = pTeam->GetMegabytes();
		int iTurns = (int)((pInfo->m_flSize-flDownloaded)/pTeam->GetBandwidth())+1;

		if (iTurns < 1)
			iTurns = 1;

		s += p.sprintf(L"Turns to download: %d\n", iTurns);
	}

	m_pInfo->SetText(s);

	m_pInfo->SetSize(m_pInfo->GetWidth(), 9999);
	m_pInfo->SetSize(m_pInfo->GetWidth(), (int)m_pInfo->GetTextHeight()+20);

	if (pTeam->HasDownloadedUpdate(x, y))
		m_pTutorial->SetText(L"You already have this update.");
	else if (pTeam->CanDownloadUpdate(x, y))
	{
		float flDownloaded = pTeam->GetMegabytes();
		int iTurns = (int)((pInfo->m_flSize-flDownloaded)/pTeam->GetBandwidth())+1;

		if (iTurns < 1)
			iTurns = 1;

		if (iTurns == 1)
			m_pTutorial->SetText(L"This update will take 1 turn to download. It will be available on your next turn.");
		else
			m_pTutorial->SetText(sprintf(L"This update will take %d turns to download.", iTurns));
	}
	else
		m_pTutorial->SetText(L"This update is not yet available for download.");
}

void CUpdatesPanel::GetTextureForUpdateItem(class CUpdateItem* pInfo, size_t& iSheet, int& sx, int& sy, int& sw, int& sh, int& tw, int& th)
{
	int iDownloadWidth = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth();
	int iDownloadHeight = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight();

	if (!pInfo)
	{
		iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet();
		Rect rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("DownloadButton");
		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		tw = iDownloadWidth;
		th = iDownloadHeight;
		return;
	}

	int iMenuWidth = 512;
	int iMenuHeight = 256;

	if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTURE)
	{
		Rect rArea;
		switch (pInfo->m_eStructure)
		{
		case STRUCTURE_CPU:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("CPU");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet();
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth();
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight();
			break;

		case STRUCTURE_BUFFER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("MacroBuffer");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet();
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth();
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight();
			break;

		case STRUCTURE_PSU:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("PSU");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet();
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth();
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight();
			break;

		case STRUCTURE_INFANTRYLOADER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("ResistorLoader");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet();
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth();
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight();
			break;

		case STRUCTURE_TANKLOADER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("DigitankLoader");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet();
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth();
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight();
			break;

		case STRUCTURE_ARTILLERYLOADER:
			rArea = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetArea("ArtilleryLoader");
			iSheet = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheet();
			tw = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetWidth();
			th = DigitanksWindow()->GetHUD()->GetDownloadSheet().GetSheetHeight();
			break;

		default:
			break;
		}

		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		return;
	}
	else if (pInfo->m_eUpdateClass == UPDATECLASS_STRUCTUREUPDATE)
	{
		eastl::string sArea;
		const CTextureSheet* pSheet;
		switch (pInfo->m_eUpdateType)
		{
		case UPDATETYPE_PRODUCTION:
			sArea = "CPUPower";
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			return;

		case UPDATETYPE_BANDWIDTH:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_CPU)
				sArea = "CPUBandwidth";
			else
				sArea = "BufferBandwidth";
			break;

		case UPDATETYPE_FLEETSUPPLY:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_CPU)
				sArea = "CPUFleet";
			else
				sArea = "BufferFleet";
			break;

		case UPDATETYPE_SUPPORTENERGY:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			sArea = "BufferEnergy";
			break;

		case UPDATETYPE_SUPPORTRECHARGE:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			sArea = "BufferRecharge";
			break;

		case UPDATETYPE_TANKATTACK:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorAttack";
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				sArea = "DigitankAttack";
			else
				sArea = "ArtilleryAttack";
			break;

		case UPDATETYPE_TANKDEFENSE:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorAttack";
			else
				sArea = "DigitankAttack";
			break;

		case UPDATETYPE_TANKMOVEMENT:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorMovement";
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				sArea = "DigitankMovement";
			else
				sArea = "ArtilleryMovement";
			break;

		case UPDATETYPE_TANKHEALTH:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			if (pInfo->m_eStructure == STRUCTURE_INFANTRYLOADER)
				sArea = "ResistorMovement";
			else if (pInfo->m_eStructure == STRUCTURE_TANKLOADER)
				sArea = "DigitankMovement";
			else
				sArea = "ArtilleryMovement";
			break;

		case UPDATETYPE_TANKRANGE:
			pSheet = &DigitanksWindow()->GetHUD()->GetDownloadSheet();
			sArea = "ArtilleryRange";
			break;
		}

		Rect rArea = pSheet->GetArea(sArea);
		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		tw = pSheet->GetSheetWidth();
		th = pSheet->GetSheetHeight();
		return;
	}
	else if (pInfo->m_eUpdateClass == UPDATECLASS_UNITSKILL)
	{
		eastl::string sArea;
		const CTextureSheet* pSheet;

		switch (pInfo->m_eUpdateType)
		{
		case UPDATETYPE_SKILL_CLOAK:
			sArea = "Cloak";
			pSheet = &DigitanksWindow()->GetHUD()->GetButtonSheet();
			break;

		case UPDATETYPE_WEAPON_CHARGERAM:
			sArea = "ChargeRam";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_AOE:
			sArea = "AOE";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_CLUSTER:
			sArea = "ClusterBomb";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_ICBM:
			sArea = "ICBM";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;

		case UPDATETYPE_WEAPON_DEVASTATOR:
			sArea = "Devastator";
			pSheet = &DigitanksWindow()->GetHUD()->GetWeaponSheet();
			break;
		}

		Rect rArea = pSheet->GetArea(sArea);
		sx = rArea.x;
		sy = rArea.y;
		sw = rArea.w;
		sh = rArea.h;
		tw = pSheet->GetSheetWidth();
		th = pSheet->GetSheetHeight();
		return;
	}

	return;
}

CUpdateButton::CUpdateButton(CUpdatesPanel* pPanel)
	: CPictureButton(L"")
{
	m_pUpdatesPanel = pPanel;
	m_iX = m_iY = 0;
}

void CUpdateButton::SetLocation(int x, int y)
{
	m_iX = x;
	m_iY = y;
}

void CUpdateButton::CursorIn()
{
	CPictureButton::CursorIn();

	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();
	if (!pUpdates)
		return;

	m_pUpdatesPanel->UpdateInfo(&pUpdates->m_aUpdates[m_iX][m_iY]);
}

void CUpdateButton::CursorOut()
{
	CPictureButton::CursorOut();

	m_pUpdatesPanel->UpdateInfo(NULL);
}

void CUpdateButton::ChooseDownloadCallback()
{
	CUpdateGrid* pUpdates = DigitanksGame()->GetUpdateGrid();
	if (!pUpdates)
		return;

	DigitanksGame()->GetCurrentLocalDigitanksTeam()->DownloadUpdate(m_iX, m_iY);

	m_pUpdatesPanel->SetVisible(false);

	int x, y;
	GetAbsPos(x, y);
	DigitanksWindow()->GetHUD()->SlideUpdateIcon(x, y);

	// Do this very last thing because since it calls Layout() this button will be deleted.
	DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(ACTIONTYPE_DOWNLOADCOMPLETE);
	DigitanksGame()->GetCurrentLocalDigitanksTeam()->HandledActionItem(ACTIONTYPE_DOWNLOADUPDATES);

	CRootPanel::Get()->Layout();
}

