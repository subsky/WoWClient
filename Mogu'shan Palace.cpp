#include "stdafx.h"
#include "Mogu'shan Palace.h"
#include <vector>
#include "Opcodes.h"

TrialoftheKing::TrialoftheKing()
{
	m_BossRooms.push_back(BossRoom(-4219.512207, -2613.556641, 16.479799, 50.f));
	m_BossRooms.push_back(BossRoom(-4214.3350, -2667.0847, 17.565369, 50.f));

	m_PathToBoss.push_back(PathStep(-3969.670166, -2542.711914, 26.753700, 1.f));//1.f
	//for not force transfer out
	//m_PathToBoss.push_back(PathStep(-3970.513184, -2614.167969, 37.346571, 0.f));//1.f
	//m_PathToBoss.push_back(PathStep(-4165.642578, -2613.799072, 37.346571, 0.f));//1.f
	
	m_PathToBoss.push_back(PathStep(-3999.812744, -2579.875488, 22.347048, 1.f));//1.f
	m_PathToBoss.push_back(PathStep(-4049.104736, -2585.520020, 22.347105f, 0.f));//1.f
	m_PathToBoss.push_back(PathStep(-4072.375977, -2585.409180, 22.346989f, 1.f));//1.f
	m_PathToBoss.push_back(PathStep(-4165.642578, -2585.711914, 22.5f, 0.f));//1.f
	m_PathToBoss.push_back(PathStep(-4219.512207, -2613.556641, 16.479799, 0.f));//1.f
}
BOOL __fastcall PositionCheckCallback(IGamePlay *pGame, const WoWGUID &monster)
{
	WoWPoint wp;
	if (!pGame->GetUnitPosition(monster, wp))
		return FALSE;
	if (wp.x < -4240.f || wp.y > -2603.f || wp.y < -2623.f)
		pGame->RunTo(-4219.512207, -2613.556641, 16.479799, 1.f, 0.f);
	return FALSE;
}
BOOL TrialoftheKing::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->Log(2, _T("Trial of the King Started!"));
	//pGame->RunTo
	if (step != 0)
	{
		//if (bLastMoveChar)
		//	Sleep(30000);
		if (step == 1)
		{
			ByteBuffer packet;
			packet << (uint32)0x1FC9;
			packet.WriteBit(1);
			packet.WriteBit(1);
			packet.FlushBits();
			pGame->SendPacket(CMSG_AREA_TRIGGER, packet);
		}
		for (uint32 i = step; i < m_PathToBoss.size(); i++)
		{
			//if (i == 2)
			//	pGame->GetOnMount(FALSE);
			//if (m_PathToBoss[i].speed == 0.f && bLastMoveChar)
			//	Sleep(6000);
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
			if (i == 1)
			{
				ByteBuffer packet;
				packet << (uint32)0x1FC9;
				packet.WriteBit(0);//enter or leave
				packet.WriteBit(1);//from client
				packet.FlushBits();
				pGame->SendPacket(CMSG_AREA_TRIGGER, packet);
			}
		}
		if (GetWoWTickCount() >(startTime + 30000) && !pGame->GetObject(_T("Xin the Weaponmaster"), true).IsEmpty())
		{
			return FALSE;
		}
	}
	if (!IsCompleted(pGame) && !pGame->IsScenarioComplete())
	{
		list<LPCTSTR> monsters;
		monsters.push_back(_T("Kuai the Brute"));
		monsters.push_back(_T("Ming the Cunning"));
		monsters.push_back(_T("Haiyan the Unstoppable"));
		WoWGUID chest;
		while ((chest = pGame->GetObject(_T("Legacy of the Clan Leaders"), false)).IsEmpty())
		{
			if (pGame->IsGameEnd())
				return FALSE;
			if (pGame->IsDead() != 0)
				return FALSE;
			if (!(pGame->GetRole() & 2) && GetWoWTickCount() > (startTime + 30000))
				return FALSE;
			list<LPCTSTR>::iterator i = monsters.begin();
			WoWGUID monster;
			for (; i != monsters.end(); i++)
			{
				monster = pGame->GetObject(*i, true);
				if (!monster.IsEmpty() && !pGame->IsNeutralTo(monster) && pGame->GetUnitHealthPercentage(monster) > 1)
					break;
			}
			if (i == monsters.end())
			{
				pGame->Log(2, _T("no clan monster available, quit Trial of the King"));
				break;
			}
			i = monsters.begin();
			for (; i != monsters.end(); i++)
			{
				monster = pGame->GetObject(*i, false);
				if (!monster.IsEmpty() && !pGame->IsNeutralTo(monster) && pGame->GetUnitHealthPercentage(monster) > 1 && pGame->IsAttackable(monster))
					break;
			}
			if (i == monsters.end())
			{
				Sleep(200);
				continue;
			}
			pGame->SetFightMode(false, true, true);
			while (!pGame->Attack(monster, 0.f, (pGame->GetRole() & 2) ? PositionCheckCallback : NULL))
			{
				Sleep(200);
				break;
			}
		}
	}
	else
		pGame->Log(2, _T("TrialoftheKing is finished"));
	if (GetWoWTickCount() > (startTime + 45000))
		return FALSE;
	while (pGame->IsInCombat() && !pGame->IsGameEnd() && (pGame->GetRole() & 2))
	{
		if (pGame->IsInCombat())
			pGame->Log(2, _T("Wait out of combat"));
		Sleep(200);
	}
	//uint32 trycount = 0;
	if (pGame->GetRole() & 2)//tank
	{
		if (!pGame->OpenChest(_T("Legacy of the Clan Leaders")))
		{
			pGame->Log(2, _T("Legacy of the Clan Leaders: no loot??? sleep some ticks and try again!"));
			//Sleep(500);
		}
	}
	else
	{
		/*while (pGame->GetObject(_T("Legacy of the Clan Leaders")).IsEmpty() && trycount ++ < 10)//
		{
			Sleep(500);
			pGame->Log(2, _T("wait for chest loot!"));
		}
		trycount = 0;
		while (!pGame->GetObject(_T("Legacy of the Clan Leaders")).IsEmpty() && trycount ++ < 50)
		{
			Sleep(500);
			pGame->Log(2, _T("wait for tank open chest!"));
		}*/
	}
	if (GetWoWTickCount() > (startTime + 45000))
	{
		return FALSE;
	}
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
	return TRUE;
}

Gekkan::Gekkan()
{
	m_BossRooms.push_back(BossRoom(-4398.502, -2607.031, -54.8428, 40.f));

	m_PathToBoss.push_back(PathStep(-4219.512207, -2613.556641, 16.479799, 1.f));//first boss location
	m_PathToBoss.push_back(PathStep(-4398.502, -2607.031, -54.8428, 0.f));//1.f
}

enum GameObjectFields
{
    GAMEOBJECT_FLAGS                                       = 0x0C + 0x005, // Size: 1, Flags: PUBLIC, URGENT
};
enum GameObjectFlags
{
    GO_FLAG_INTERACT_COND   = 0x00000004,                   // cannot interact (condition to interact)
};
BOOL __fastcall PreventAttackOther(IGamePlay *pGame, const WoWGUID &monster);
BOOL Gekkan::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->Log(2, _T("Boss Gekkan Started!"));
	if (step != 0)
	{
		//if (step == 1)
			//pGame->GetOnMount(FALSE);
		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
		pGame->RunTo(m_BossRooms[0].x, m_BossRooms[0].y, m_BossRooms[0].z, 1.f, 0.f);
	}
	if (!(pGame->GetRole() & 2) && GetWoWTickCount() > (startTime + 5000))
		return FALSE;
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("gekkan is finished"));
		return TRUE;
	}
	WoWGUID monster;
	CMonsterExclude excludes;
	pGame->SetMonsterExclusive(&excludes);
	if (!(pGame->GetRole() & 2))
	{
		monster = pGame->GetObject(_T("Gekkan"), true, 50.f);
		if (!pGame->WaitSomeonePull(monster, 30000))//target
		{
			pGame->Log(2, _T("none pull the boss, exit"));
			return false;
		}
	}
	else if (!pGame->WaitHealerToPosition(m_PathToBoss[m_PathToBoss.size() - 1], 30000, 2.f) || !pGame->WaitHealerOnline(30000))
	{
		pGame->Log(2, _T("healer is not ready"));
		return false;
	}
	if (GetWoWTickCount() > (startTime + 45000))
	{
		return FALSE;
	}
	list<LPCTSTR> monsters;
	monsters.push_back(_T("Glintrok Oracle"));
	monsters.push_back(_T("Glintrok Skulker"));
	monsters.push_back(_T("Glintrok Ironhide"));
	monsters.push_back(_T("Gekkan"));
	monsters.push_back(_T("Glintrok Hexxer"));
	pGame->AddNoThreatMonster("Glintrok Hexxer");
	WoWGUID loot;
	bool pulled = false;
	while (1)
	{
		if (pGame->IsGameEnd())
			return FALSE;
		if (!pGame->IsScenarioComplete() && !IsCompleted(pGame))//tank
		{
		}

		list<LPCTSTR>::iterator i = monsters.begin();
		for (; i != monsters.end(); i ++)
		{
			monster = pGame->GetObject(*i, true, 50.f);
			if (!monster.IsEmpty() && pGame->GetUnitHealth(monster) > 0)
				break;
		}
		if (!pulled)
		{
			WoWGUID hexxer = pGame->GetObject(_T("Glintrok Hexxer"), true, 50.f);
			if (!hexxer.IsEmpty() && pGame->GetUnitHealth(monster) > 0)
			{
				WoWPoint pos;
				if (pGame->GetUnitPosition(hexxer, pos) && fabs(pos.z - -54.8428) > 3.f)
				{
					excludes.m_excludes[hexxer] = 0;
					continue;
				}
				if (!pGame->Pull(hexxer))
					pGame->Log(2, _T("pull Glintrok Hexxer failed"));
				else
					pulled = true;
			}
		}
		if (i == monsters.end())
		{
			pGame->Log(2, _T("All monsters killed"));
			break;
		}
		WoWPoint pos;
		if (pGame->GetUnitPosition(monster, pos) && fabs(pos.z - -54.8428) > 3.f)
		{
			excludes.m_excludes[monster] = 0;
			continue;
		}
		pGame->SetFightMode(false, true, true);
		if (pGame->Attack(monster, 0.f, PreventAttackOther) != 0)
			Sleep(200);
	}
	if (pGame->GetRole() & 2)//tank
	{
		pGame->RunTo(-4397.961914, -2567.636230, -54.531246, 0.f, 0.f);
		if (!pGame->OpenChest(_T("Ancient Mogu Treasure")))
		{
			pGame->Log(2, _T("Ancient Mogu Treasure: no loot??? sleep some ticks and try again!"));
			//Sleep(500);
		}	
	}
	else
	{/*
		uint32 trycount = 0;
		while (pGame->GetObject(_T("Ancient Mogu Treasure")).IsEmpty() && trycount ++ < 10)//
		{
			Sleep(500);
			pGame->Log(2, _T("wait for Treasure loot!"));
		}
		trycount = 0;
		while (!pGame->GetObject(_T("Ancient Mogu Treasure")).IsEmpty() && trycount ++ < 50)
		{
			Sleep(500);
			pGame->Log(2, _T("wait for tank open Treasure!"));
		}*/
	}
	if (GetWoWTickCount() > (startTime + 30000))
	{
		return FALSE;
	}

	pGame->SetMonsterExclusive(NULL);
	pGame->ResetAvoidCastingSpell();
	pGame->ResetNoThreatMonster();
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
	return TRUE;
}

Xin::Xin()
{
	m_BossRooms.push_back(BossRoom(-4695.984863, -2614.093262, 27.470495, 40.f));
	m_BossRooms.push_back(BossRoom(-4630.084473, -2612.998291, 21.899673, 40.f));

	m_PathToBoss.push_back(PathStep(-4397.854, -2563.38, -54.8428, 1.f));//chest position
	m_PathToBoss.push_back(PathStep(-4686.484375, -2656.521484, 21.31, 0.f));//1.f
	//m_PathToBoss.push_back(PathStep(-4727.199707, -2635.435791, 27.420256, 0.f));//1.f
}
BOOL Xin::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->Log(2, _T("Boss Xin the Weaponmaster Started!"));
	if (step != 0)
	{
		for (uint32 i = step; i < m_PathToBoss.size(); i++)
		{
			if (i == 2)
			{
				if (GetWoWTickCount() >(startTime + 60000))
				{
					return FALSE;
				}
			}
			//if (i != (m_PathToBoss.size() - 1) || (pGame->GetRole() & 2))//tank
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
			//else
			//pGame->RunTo(-4701.042969, -2622.886963, 28.875761, 0.f);
			//pGame->RunTo(-4727.199707, -2656.435791, 15, 0.f);
		}
	}
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("xin is finished"));
		return TRUE;
	}
	if (GetWoWTickCount() > (startTime + 5000))
		return FALSE;

	WoWGUID xin;
	uint32 start = GetWoWTickCount();
	while (GetWoWTickCount() - start < 5000)
	{
		xin = pGame->GetObject(_T("Xin the Weaponmaster"), true);
		if (!xin)
			Sleep(100);
		else
			break;
	}
	if (xin.IsEmpty())
	{
		pGame->Log(2, _T("not found boss Xin the Weaponmaster"));
		return TRUE;
	}

	WoWGUID quilen = pGame->GetObject(_T("Quilen Guardian"), true);
	if (quilen.IsEmpty() || pGame->GetUnitHealth(quilen) == 0)
		pGame->Log(2, _T("not found quilen guardian, are they killed?"));
	else if (!(pGame->GetRole() & 2))//not tank
	{
		if (!pGame->WaitSomeonePull(quilen, 60000))
			return false;
	}
	else if (!pGame->WaitHealerToPosition(m_PathToBoss[m_PathToBoss.size() - 1], 30000, 1.f) || !pGame->WaitHealerOnline(30000))
	{
		pGame->Log(2, _T("healer is not ready"));
		return false;
	}

	list<LPCTSTR> monsters;
	CMonsterExclude excludes;
	pGame->SetMonsterExclusive(&excludes);
	monsters.push_back(_T("Quilen Guardian"));
	uint32 start_time = GetWoWTickCount();
	WoWGUID guardians[4];
	int len = 0;
	while ((len = pGame->GetObjects(_T("Quilen Guardian"), guardians, 4)) > 0)
	{
		int i = 0;
		for (; i < len; i++)
		{
			if (pGame->GetUnitHealth(guardians[i]) > 0)
				break;
		}
		if (i == len)
			break;
		pGame->SetFightMode(false, true, true);
		if (!pGame->KillClosestMonster(0, &monsters, 100))
		{
			if (GetWoWTickCount() > start_time + 120000)
			{
				pGame->Log(2, _T("killing Quilen Guardian timeout!!!"));
				break;
			}
			Sleep(200);
		}
	}
	//if (GetWoWTickCount() >(startTime + 5000))
	//	return FALSE;

	excludes.m_excludes.clear();
	monsters.push_back(_T("Gurthan Swiftblade"));
	monsters.push_back(_T("Harthak Flameseeker"));
	monsters.push_back(_T("Kargesh Highguard"));
	WoWGUID monster;
	start_time = GetWoWTickCount();
	while (1)
	{
		if (pGame->IsGameEnd())
			return FALSE;
		if (pGame->IsDead())
			return FALSE;
		list<LPCTSTR>::iterator i = monsters.begin();
		for (; i != monsters.end(); i++)
		{
			monster = pGame->GetObject(*i, true, 100.f);
			if (!monster.IsEmpty() && pGame->GetUnitHealth(monster) > 0)
				break;
		}
		if (i == monsters.end())
		{
			pGame->Log(2, _T("All guards killed"));
			break;
		}
		WoWPoint pos;
		if (pGame->GetUnitPosition(monster, pos) && fabs(pos.x - (-4630.084473)) > 50.f)
		{
			excludes.m_excludes[monster] = 0;
			continue;
		}
		pGame->SetFightMode(false, true, true);
		if (pGame->Attack(monster, 0.f, PreventAttackOther) != 0)
			Sleep(200);
	}

	pGame->SetMonsterExclusive(NULL);
	if (GetWoWTickCount() >(startTime + 5000))
		return FALSE;

	pGame->AddAvoidGroundEffect("Ring of Fire");//ring of fire
	pGame->AddAvoidCastingSpell("Ground Slam");//ground slam
	pGame->AddAvoidUnit(61499, "Ring of Fire", true, 6.5f);
	pGame->AddAvoidUnit(61499, "Ring of Fire", false, 6.5f);
	pGame->AddAvoidUnit(61433, "Ring of Fire", true, 6.5f);
	pGame->AddAvoidUnit(61433, "Ring of Fire", false, 6.5f);
	pGame->AddAvoidUnit(61451, "Whirlwind", true, 8.f);
	pGame->AddAvoidLine(WoWPoint(-4653.144, -2571.559, 27.28992), WoWPoint(-4635.11, -2642.283, 27.75975), 4.f);
	pGame->AddAvoidLine(WoWPoint(-4612.325, -2655.249, 27.23762), WoWPoint(-4594.599, -2584.786, 27.18633), 4.f);
	pGame->AddAvoidLine(WoWPoint(-4613.093, -2655.249, 27.23765), WoWPoint(-4630.541, -2584.572, 27.10564), 4.f);
	pGame->AddAvoidLine(WoWPoint(-4654.006, -2571.327, 27.28995), WoWPoint(-4671.301, -2642.371, 27.81245), 4.f);
	if (!pGame->IsScenarioComplete() && !IsCompleted(pGame) && !(pGame->GetRole() & 2))//not tank
	{
		if (!pGame->WaitSomeonePull(xin, 30000))//target
		{
			pGame->Log(2, _T("none pull the boss, exit"));
			return false;
		}
		WoWPoint pos;
		uint32 trycount = 0;
		while (pGame->GetUnitPosition(xin, pos) && pos.z > 24.f && trycount ++ < 20)
		{
			Sleep(500);
		}
	}
	/*if (pGame->GetRole() & 2)
		pGame->RunTo(-4710.471680, -2613.502197, 28.875629, 0.f);//1.f
	else
		pGame->RunTo(-4701.042969, -2622.886963, 28.875761, 0.f);*/
	if (pGame->GetRole() & 2)
	{
		if ((!pGame->WaitHealerToPosition(m_BossRooms[m_BossRooms.size() - 1], 15000, 50.f) && !pGame->WaitHealerToPosition(m_PathToBoss[m_PathToBoss.size() - 1], 15000, 50.f)) || !pGame->WaitHealerOnline(30000))
		{
			pGame->Log(2, _T("healer is not ready"));
			return false;
		}

		pGame->RunTo(-4675.803223, -2613.835693, 22.325666, 1.f, 0.f);//1.f
		uint32 current = GetWoWTickCount();
		while (!pGame->Pull(xin))
		{
			if ((GetWoWTickCount() - current) > 30000)
			{
				pGame->Log(2, _T("pull xin failed! exit"));
				return false;
			}
			Sleep(500);
		}
		pGame->RunTo(-4630.084473, -2612.998291, 21.899673, 0.f, 0.f);
	}
	pGame->SetFightMode(true, true, true);
	pGame->Attack(xin, 0.f);
	pGame->ResetAvoidCastingSpell();
	pGame->ResetAvoidGroundEffect();
	pGame->ResetAvoidUnit();
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
	return TRUE;
}


ExitMogushan::ExitMogushan()
{
	m_BossRooms.push_back(BossRoom(-3969.258, -2520.795, 26.80361, 10.f));//exit point
	m_BossRooms.push_back(BossRoom(1400.152, 428.4431, 479.0349, 30.f));//enter point, from pandaria
	//m_BossRooms.push_back(PathStep(3529.825439, 2699.591064, 755.938599, 30.f));//tomb location, from pandaria

	m_PathToBoss.push_back(PathStep(-4710.471680, -2613.502197, 28.875629, 0.f));//1.f
	m_PathToBoss.push_back(PathStep(-4724.474121, -2526.711914, 27.417645, 0.f));//first boss location
	m_PathToBoss.push_back(PathStep(-3969.670166, -2526.711914, 26.753700, 0.f));//0.f
	//m_PathToBoss.push_back(PathStep(3649.855, 2548.755, 766.9684, 0.f));//exit point
}
BOOL ExitMogushan::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (step != 0)
	{
		if (GetWoWTickCount() > (startTime + 75000))
		{
			return FALSE;
		}

		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
		{
			if (i == 2)
				pGame->GetOnMount(FALSE);
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
		}
	}
	if (pGame->GetMapID() == 0x03E2)
		pGame->ChangeArea();//the position of the areatrigger we got is not the with the game, use this function in fixed style
		//pGame->ChangeArea();
	if (pGame->GetMapID() == 0x0366)
	{
		pGame->ResetInstances();
		//pGame->RunTo(3643.886, 2544.806, 769.9496, 1.f);
		if (!pGame->ChangeArea() || pGame->GetMapID() != 0x03E2)
		{
			pGame->Log(2, _T("chaning area failed, from pandaria to sha-do pan monastery"));
			//pGame->RunTo(3631.290039, 2538.409912, 769.911987, 1.f);//back to enter position
			return FALSE;
		}
	}
	if (GetWoWTickCount() > (startTime + 45000))
	{
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////
////
///////////////////////////////////////////////////////////////////////
void CMogushanPalace::Load(IBotConfig *config)
{
	CGame::Load(config);
}

BOOL CMogushanPalace::Run(IGamePlay *pGame)
{
	return CDungeonGame::Run(pGame);
}

BOOL CMogushanPalace::Prepare(IGamePlay *pGame)
{
	return CGame::Prepare(pGame);
}
