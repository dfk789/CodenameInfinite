#include "hud.h"

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "digitankswindow.h"
#include "game/digitanksgame.h"
#include "debugdraw.h"

using namespace glgui;

CPowerBar::CPowerBar(powerbar_type_t ePowerbarType)
	: CLabel(0, 0, 100, 100, "")
{
	m_ePowerbarType = ePowerbarType;
}

void CPowerBar::Think()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	char szLabel[100];
	if (m_ePowerbarType == POWERBAR_HEALTH)
	{
		sprintf(szLabel, "Health: %.1f/%.1f", pTank->GetHealth(), pTank->GetTotalHealth());
		SetText(szLabel);
	}
	else if (m_ePowerbarType == POWERBAR_ATTACK)
	{
		sprintf(szLabel, "Attack Power: %.1f/%.1f", pTank->GetAttackPower(true), pTank->GetTotalAttackPower());
		SetText(szLabel);
	}
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
	{
		sprintf(szLabel, "Defense Power: %.1f/%.1f", pTank->GetDefensePower(true), pTank->GetTotalDefensePower());
		SetText(szLabel);
	}
	else
	{
		sprintf(szLabel, "Movement Power: %.1f/%.1f", pTank->GetMovementPower(true), pTank->GetTotalMovementPower());
		SetText(szLabel);
	}
}

void CPowerBar::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	CRootPanel::PaintRect(x, y, w, h, Color(255, 255, 255, 128));

	if (m_ePowerbarType == POWERBAR_HEALTH)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetHealth() / pTank->GetTotalHealth())-2, h-2, Color(0, 150, 0));
	else if (m_ePowerbarType == POWERBAR_ATTACK)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetAttackPower(true) / pTank->GetTotalAttackPower())-2, h-2, Color(150, 0, 0));
	else if (m_ePowerbarType == POWERBAR_DEFENSE)
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetDefensePower(true) / pTank->GetTotalDefensePower())-2, h-2, Color(100, 100, 0));
	else
		CRootPanel::PaintRect(x+1, y+1, (int)(w * pTank->GetMovementPower(true) / pTank->GetTotalMovementPower())-2, h-2, Color(0, 0, 150));

	BaseClass::Paint(x, y, w, h);
}

CHUD::CHUD()
	: CPanel(0, 0, 100, 100)
{
	m_pGame = NULL;

	m_pHealthBar = new CPowerBar(POWERBAR_HEALTH);
	m_pAttackPower = new CPowerBar(POWERBAR_ATTACK);
	m_pDefensePower = new CPowerBar(POWERBAR_DEFENSE);
	m_pMovementPower = new CPowerBar(POWERBAR_MOVEMENT);

	AddControl(m_pHealthBar);
	AddControl(m_pAttackPower);
	AddControl(m_pDefensePower);
	AddControl(m_pMovementPower);

	m_pButton1 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton1);

	m_pButton2 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton2);

	m_pButton3 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton3);

	m_pButton4 = new CButton(0, 0, 50, 50, "");
	AddControl(m_pButton4);

	m_pAttackInfo = new CLabel(0, 0, 100, 150, "");
	m_pAttackInfo->SetWrap(false);
	m_pAttackInfo->SetAlign(glgui::CLabel::TA_TOPLEFT);
	AddControl(m_pAttackInfo);

	SetupMenu(MENUMODE_MAIN);
}

void CHUD::Layout()
{
	SetSize(GetParent()->GetWidth(), GetParent()->GetHeight());

	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	m_pAttackInfo->SetPos(iWidth/2 - 1024/2 + 190 + 3, iHeight - 150 - 10 - 80 + 3);
	m_pAttackInfo->SetSize(200, 80);

	m_pHealthBar->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 140);
	m_pHealthBar->SetSize(200, 20);

	m_pAttackPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 90);
	m_pAttackPower->SetSize(200, 20);

	m_pDefensePower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 60);
	m_pDefensePower->SetSize(200, 20);

	m_pMovementPower->SetPos(iWidth/2 - 1024/2 + 450, iHeight - 30);
	m_pMovementPower->SetSize(200, 20);

	m_pButton1->SetPos(iWidth/2 - 1024/2 + 700, iHeight - 100);
	m_pButton2->SetPos(iWidth/2 - 1024/2 + 760, iHeight - 100);
	m_pButton3->SetPos(iWidth/2 - 1024/2 + 820, iHeight - 100);
	m_pButton4->SetPos(iWidth/2 - 1024/2 + 880, iHeight - 100);
}

void CHUD::Think()
{
	BaseClass::Think();

	if (m_eMenuMode == MENUMODE_MAIN)
	{
		if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
			m_pButton1->SetButtonColor(Color(100, 0, 0));
		else
			m_pButton1->SetButtonColor(Color(0, 0, 100));

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
			m_pButton2->SetButtonColor(Color(100, 0, 0));
		else
			m_pButton2->SetButtonColor(Color(0, 0, 100));

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
			m_pButton3->SetButtonColor(Color(100, 0, 0));
		else
			m_pButton3->SetButtonColor(Color(60, 40, 0));

		m_pButton4->SetButtonColor(Color(200, 200, 0));
	}
	else if (m_eMenuMode == MENUMODE_PROMOTE)
	{
		if (DigitanksGame()->GetCurrentTank()->HasBonusPoints())
		{
			m_pButton1->SetButtonColor(Color(200, 0, 0));
			m_pButton2->SetButtonColor(Color(200, 200, 0));
			m_pButton3->SetButtonColor(Color(0, 0, 200));
		}
		else
		{
			m_pButton1->SetButtonColor(g_clrBox);
			m_pButton2->SetButtonColor(g_clrBox);
			m_pButton3->SetButtonColor(g_clrBox);
		}
		m_pButton4->SetButtonColor(Color(100, 0, 0));
	}
}

void CHUD::Paint(int x, int y, int w, int h)
{
	if (!DigitanksGame())
		return;

#ifdef _DEBUG
	int iWidth = CDigitanksWindow::Get()->GetWindowWidth();
	int iHeight = CDigitanksWindow::Get()->GetWindowHeight();

	// Nobody runs resolutions under 1024x768 anymore.
	// Show me my constraints!
	CRootPanel::PaintRect(iWidth/2 - 1024/2, iHeight - 150, 1024, 200, Color(0, 0, 0, 100));

	// This is where the minimap will be.
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 10, iHeight - 150 - 30, 170, 170, Color(255, 255, 255, 100));

	// Shield schematic
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 190, iHeight - 150 + 10, 130, 130, Color(255, 255, 255, 100));

	// Tank data
	CRootPanel::PaintRect(iWidth/2 - 1024/2 + 330, iHeight - 150 + 10, 100, 130, Color(255, 255, 255, 100));
#endif

	// Background for the attack info label
	CRootPanel::PaintRect(m_pAttackInfo->GetLeft()-3, m_pAttackInfo->GetTop()-9, m_pAttackInfo->GetWidth()+6, m_pAttackInfo->GetHeight()+6, Color(0, 0, 0, 100));

	for (size_t i = 0; i < DigitanksGame()->GetNumTeams(); i++)
	{
		CTeam* pTeam = DigitanksGame()->GetTeam(i);
		for (size_t j = 0; j < pTeam->GetNumTanks(); j++)
		{
			CDigitank* pTank = pTeam->GetTank(j);

			Vector vecOrigin;
			if (pTank->HasDesiredMove())
				vecOrigin = pTank->GetDesiredMove();
			else
				vecOrigin = pTank->GetOrigin();

			Vector vecScreen = CDigitanksWindow::Get()->ScreenPosition(vecOrigin);

			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 61, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 60, (int)(100.0f*pTank->GetHealth()/pTank->GetTotalHealth()), 3, Color(100, 255, 100));

			float flAttackPower = pTank->GetAttackPower(true);
			float flDefensePower = pTank->GetDefensePower(true);
			float flMovementPower = pTank->GetMovementPower(true);
			float flTotalPower = flAttackPower + flDefensePower + flMovementPower;
			flAttackPower = flAttackPower/flTotalPower;
			flDefensePower = flDefensePower/flTotalPower;
			flMovementPower = flMovementPower/flTotalPower;
			CRootPanel::PaintRect((int)vecScreen.x - 51, (int)vecScreen.y - 51, 102, 5, Color(255, 255, 255, 128));
			CRootPanel::PaintRect((int)vecScreen.x - 50, (int)vecScreen.y - 50, (int)(100.0f*flAttackPower), 3, Color(255, 0, 0));
			CRootPanel::PaintRect((int)vecScreen.x - 50 + (int)(100.0f*flAttackPower), (int)vecScreen.y - 50, (int)(100.0f*flDefensePower), 3, Color(255, 255, 0));
			CRootPanel::PaintRect((int)vecScreen.x - 50 + (int)(100.0f*(1-flMovementPower)), (int)vecScreen.y - 50, (int)(100.0f*flMovementPower), 3, Color(0, 0, 255));

			if (pTank == DigitanksGame()->GetCurrentTank() && CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
			{
				int iHeight = (int)(200 * (pTank->GetBasePower()-pTank->GetBaseMovementPower())/pTank->GetBasePower());

				if (iHeight < 20)
					iHeight = 20;

				int iTop = (int)vecScreen.y - iHeight/2;
				int iBottom = (int)vecScreen.y + iHeight/2;

				int mx, my;
				glgui::CRootPanel::GetFullscreenMousePos(mx, my);

				float flAttackPercentage = RemapValClamped((float)my, (float)iTop, (float)iBottom, 0, 1);

				CRootPanel::PaintRect((int)vecScreen.x + 60, iTop, 20, iHeight, Color(255, 255, 255, 128));

				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1, 18, (int)(flAttackPercentage*(iHeight-2)), Color(255, 0, 0, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + 1 + (int)(flAttackPercentage*(iHeight-2)), 18, (int)((1-flAttackPercentage)*(iHeight-2)), Color(255, 255, 0, 255));
				CRootPanel::PaintRect((int)vecScreen.x + 61, iTop + (int)(flAttackPercentage*(iHeight-2)) - 2, 18, 6, Color(128, 128, 128, 255));

				DigitanksGame()->GetCurrentTank()->SetAttackPower(flAttackPercentage * (pTank->GetBasePower()-pTank->GetBaseMovementPower()));

				UpdateAttackInfo();
			}
		}
	}

	CPanel::Paint(x, y, w, h);

	if (m_eMenuMode == MENUMODE_MAIN)
	{
		if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}
		else
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 30.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 700 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}
		else
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 20.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 20.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 20.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 10.0f, iHeight - 100 + 30.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 20.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 20.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 30.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 30.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 30.0f, iHeight - 100 + 30.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 760 + 40.0f, iHeight - 100 + 30.0f, 0), Color(255, 255, 255));
		}

		if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		{
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
		}
		else
		{
			glPushMatrix();
			glTranslatef(iWidth/2 - 1024/2 + 820 + 25.0f, iHeight - 100 + 25.0f, 0);
			glRotatef(90, 1, 0, 0);
			DebugCircle(Vector(0, 0, 0), 15, Color(255, 255, 255));
			glPopMatrix();
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 5.0f, iHeight - 100 + 25.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 45.0f, iHeight - 100 + 25.0f, 0), Color(255, 255, 255));
			DebugLine(Vector(iWidth/2 - 1024/2 + 820 + 25.0f, iHeight - 100 + 5.0f, 0), Vector(iWidth/2 - 1024/2 + 820 + 25.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
		}

		DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 5.0f, iHeight - 100 + 25.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 45.0f, iHeight - 100 + 25.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 25.0f, iHeight - 100 + 5.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 25.0f, iHeight - 100 + 45.0f, 0), Color(255, 255, 255));
	}
	else
	{
		DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 10.0f, iHeight - 100 + 10.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 40.0f, iHeight - 100 + 40.0f, 0), Color(255, 255, 255));
		DebugLine(Vector(iWidth/2 - 1024/2 + 880 + 10.0f, iHeight - 100 + 40.0f, 0), Vector(iWidth/2 - 1024/2 + 880 + 40.0f, iHeight - 100 + 10.0f, 0), Color(255, 255, 255));
	}
}

void CHUD::UpdateAttackInfo()
{
	m_pAttackInfo->SetText("");

	CDigitank* pCurrentTank = DigitanksGame()->GetCurrentTank();

	if (!pCurrentTank)
		return;

	CDigitank* pTargetTank = pCurrentTank->GetTarget();

	if (!pTargetTank)
		return;

	Vector vecOrigin;
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE && pCurrentTank->GetPreviewMoveTurnPower() <= pCurrentTank->GetTotalMovementPower())
		vecOrigin = pCurrentTank->GetPreviewMove();
	else
		vecOrigin = pCurrentTank->GetDesiredMove();

	Vector vecAttack = vecOrigin - pTargetTank->GetOrigin();
	float flAttackDistance = vecAttack.Length();

	int iHitOdds = (int)RemapValClamped(flAttackDistance, 30, 50, 100, 0);

	if (iHitOdds <= 0)
	{
		m_pAttackInfo->SetText("Hit odds: 0%");
		return;
	}

	float flDamageBlocked = (*pTargetTank->GetShieldForAttackDirection(vecAttack/flAttackDistance)) * pTargetTank->GetDefenseScale(true);
	float flAttackDamage = pCurrentTank->GetAttackPower(true);

	float flShieldDamage;
	float flTankDamage = 0;
	if (flAttackDamage - flDamageBlocked <= 0)
		flShieldDamage = flAttackDamage;
	else
	{
		flShieldDamage = flDamageBlocked;
		flTankDamage = flAttackDamage - flDamageBlocked;
	}

	char szAttackInfo[1024];
	sprintf(szAttackInfo,
		"Hit odds: %d%%\n"
		" \n"
		"Shield Damage: %.1f/%.1f\n"
		"Digitank Damage: %.1f/%.1f\n",
		iHitOdds,
		flShieldDamage, pTargetTank->GetShieldMaxStrength() * pTargetTank->GetDefenseScale(true),
		flTankDamage, pTargetTank->GetHealth()
	);

	m_pAttackInfo->SetText(szAttackInfo);
}

void CHUD::SetGame(CDigitanksGame *pGame)
{
	m_pGame = pGame;
	m_pGame->SetListener(this);
}

void CHUD::SetupMenu(menumode_t eMenuMode)
{
	if (eMenuMode == MENUMODE_MAIN)
	{
		m_pButton1->SetClickedListener(this, Move);
		m_pButton2->SetClickedListener(this, Turn);
		m_pButton3->SetClickedListener(this, Fire);
		m_pButton4->SetClickedListener(this, Promote);
	}
	else if (eMenuMode == MENUMODE_PROMOTE)
	{
		m_pButton1->SetClickedListener(this, PromoteAttack);
		m_pButton2->SetClickedListener(this, PromoteDefense);
		m_pButton3->SetClickedListener(this, PromoteMovement);
		m_pButton4->SetClickedListener(this, GoToMain);
	}

	m_eMenuMode = eMenuMode;
}

void CHUD::GameStart()
{
	CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
}

void CHUD::GameOver()
{
}

void CHUD::NewCurrentTeam()
{
}

void CHUD::NewCurrentTank()
{
	if (!DigitanksGame()->GetCurrentTank()->HasDesiredMove() && !DigitanksGame()->GetCurrentTank()->HasDesiredTurn())
		CDigitanksWindow::Get()->SetControlMode(MODE_MOVE, true);
}

void CHUD::MoveCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_MOVE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_MOVE);
}

void CHUD::TurnCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_TURN)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_TURN);
}

void CHUD::FireCallback()
{
	if (CDigitanksWindow::Get()->GetControlMode() == MODE_FIRE)
		CDigitanksWindow::Get()->SetControlMode(MODE_NONE);
	else
		CDigitanksWindow::Get()->SetControlMode(MODE_FIRE);
}

void CHUD::PromoteCallback()
{
	SetupMenu(MENUMODE_PROMOTE);
}

void CHUD::PromoteAttackCallback()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteAttack();

	SetupMenu(MENUMODE_MAIN);

	UpdateAttackInfo();
}

void CHUD::PromoteDefenseCallback()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteDefense();

	SetupMenu(MENUMODE_MAIN);

	UpdateAttackInfo();
}

void CHUD::PromoteMovementCallback()
{
	if (!DigitanksGame())
		return;

	if (!DigitanksGame()->GetCurrentTank())
		return;

	CDigitank* pTank = DigitanksGame()->GetCurrentTank();

	pTank->PromoteMovement();

	SetupMenu(MENUMODE_MAIN);

	UpdateAttackInfo();
}

void CHUD::GoToMainCallback()
{
	SetupMenu(MENUMODE_MAIN);
}