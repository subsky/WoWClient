#include "stdafx.h"
#include "ShadowfangKeep.h"
#include <vector>
#include "Opcodes.h"
#include "UpdateFields.h"

BaronAshbury::BaronAshbury()
{
	m_BossRooms.push_back(BossRoom(-255.185196, 2119.805176, 81.179550, 10.f, 2.f));

	m_PathToBoss.push_back(PathStep(-228.190994, 2111.409912, 76.890404, 0.f));
	//m_PathToBoss.push_back(PathStep(-187.261368, -426.928436, 53.924702, 1.f));//1.f
	m_PathToBoss.push_back(PathStep(-255.185196, 2119.805176, 81.179550, 1.f));
}
BOOL __fastcall BaronAshburyAttack(IGamePlay *pGame, const WoWGUID &monster)
{
	WoWGUID geist = pGame->GetObject(_T("Frantic Geist"), true, 5.f);
	if (!geist.IsEmpty())
	{
		return TRUE;
	}
	return FALSE;
}
BOOL  BaronAshbury::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (step == 1)
	{
		while (!pGame->WaitAllPlayerToPosition(m_PathToBoss[step - 1], 2000, 1.f))
		{
			if (IsCompleted(pGame) || pGame->IsInEngageUnit())
				return TRUE;
			if (GetWoWTickCount() > startTime + 30000)
				return TRUE;
		}
	}
	return TRUE;
}
BOOL BaronAshbury::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->DisableAreaTrigger(0x966);
	pGame->SetMonsterExclusive(NULL);
	pGame->Log(2, _T("Baron Ashbury Started!"));
	pGame->AutoEquip();
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Baron Ashbury is finished"));
		return TRUE;
	}
	WoWGUID ashbury;
	uint32 start = GetWoWTickCount();
	;
	while ((ashbury = pGame->GetObject(_T("Baron Ashbury"), false)).IsEmpty())
	{
		Sleep(200);
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
	}
	/*if (GetWoWTickCount() > startTime + 60000)
		return false;*/

	//if (GetWoWTickCount() > startTime + 60000)
	//	return false;
	if (!pGame->IsScenarioComplete() && !IsCompleted(pGame) && !(pGame->GetRole() & 2))//not tank
	{
		pGame->Log(2, _T("waiting tank pull boss Baron Ashbury"));
		if (!pGame->WaitSomeonePull(ashbury, 30000))//target
		{
			pGame->Log(2, _T("none pull the boss, exit"));
			return false;
		}
	}
	if (pGame->GetRole() & 2)
	{
		if ((!pGame->WaitHealerToPosition(m_BossRooms[m_BossRooms.size() - 1], 15000, 50.f) && !pGame->WaitHealerToPosition(m_PathToBoss[m_PathToBoss.size() - 1], 15000, 50.f)) || !pGame->WaitHealerOnline(30000))
		{
			pGame->Log(2, _T("healer is not ready"));
			return false;
		}
	}
	//[Pain and Suffering] interrupt
	pGame->SetFightMode(true, true, true);
	pGame->AddDispelSpell("Pain and Suffering");
	pGame->AddInterruptSpell("Pain and Suffering");
	while (!IsCompleted(pGame))
	{
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
		pGame->Attack(ashbury, 5.f, BaronAshburyAttack);
		WoWGUID giest = pGame->GetObject(_T("Frantic Geist"), true, 5.f);
		if (!giest.IsEmpty())
		{
			pGame->SetFightMode(true, false, false);
			pGame->Attack(giest, 5.f);
			pGame->SetFightMode(true, true, true);
		}

	}
	pGame->ResetDispelSpells();
	pGame->ResetInterruptSpells();
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);	
	pGame->AutoEquip();
	return TRUE;
}
BaronSilverlaine::BaronSilverlaine()
{
	m_BossRooms.push_back(BossRoom(-276.419006, 2298.489502, 76.153763, 10.f));

	m_PathToBoss.push_back(PathStep(-255.185196, 2119.805176, 81.179550, 1.f));
	m_PathToBoss.push_back(PathStep(-241.831467, 2157.964844, 90.624199, 1.f));//door position, waiting open
	m_PathToBoss.push_back(PathStep(-178.104187, 2225.915771, 78.273933, 1.f));
	//m_PathToBoss.push_back(PathStep(-235.804321, 2232.552246, 79.778709, 0.f));//ÂíÅï½ÇÂä
	//m_PathToBoss.push_back(PathStep(-210.061539, 2247.186035, 79.772903, 0.f));//ÂíÅï½ÇÂä ÁíÒ»²à
	//m_PathToBoss.push_back(PathStep(-180.592300, 2234.436035, 76.241226, 0.f));//165a AT, kill Shadowy Attendant/Haunted Servitor
	//x=-178.063065, y=2224.296387, z=79.194443 CMSG_AREA_TRIGGER]:5A 16 00 00 C0 
	//m_PathToBoss.push_back(PathStep(-210.061539, 2247.186035, 79.772903, 0.f));//ÂíÅï½ÇÂä ÁíÒ»²à
	//m_PathToBoss.push_back(PathStep(-288.262268, 2280.708008, 79.767525, 0.f));
	//-274.380493, 2284.632813, 79.845154
	m_PathToBoss.push_back(PathStep(-276.419006, 2298.489502, 76.153763, 1.f));//boss fighting position
}
BOOL  BaronSilverlaine::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
{
	if (step == 2)
	{
		//waiting for door open
		WoWGUID door = pGame->GetGameObject(18895);
		if (!door.IsEmpty())
		{
			while (pGame->GetObjectValue(door, GAMEOBJECT_BYTES_1) & 0x00FF)//1 closed, 0 open
				Sleep(200);
		}
		//it's too close to the pior point, base class cant handle this
		pGame->RunTo(-240.327881, 2161.715820, 90.335617, 1.f, 1.f);//CMSG_AREA_TRIGGER]:C4 15 00 00 C0
	}
	else if (step == 3)
	{
		//Sleep(15000);
	}
	else if (step == 5)
	{/*
		LPCTSTR monsters[] = { _T("Shadowy Attendant"),
			_T("Haunted Servitor") };
		pGame->Log(2, _T("Start killing two guards"));
		uint32 start_time = GetWoWTickCount();
		int len = 0;
		pGame->SetFightMode(false, true, false);
		WoWGUID monster;
		pGame->Lock();
		WoWGUID tank = pGame->GetWorkingGroup().GetTank();
		pGame->Unlock();
		if (!(monster = pGame->GetObject(monsters, 2, true, 15.f)).IsEmpty())
			pGame->Attack(monster);
		pGame->RunTo(-178.104187, 2225.915771, 78.273933, 0.f, 0.f);//area_trigger 5A 16 00 00
		uint32 startTick = GetWoWTickCount();
		bool allatposition = false;
		while (1)
		{
			if (pGame->IsGameEnd() || pGame->IsDead())
				return FALSE;
			//-178.093719, 2223.915039, 79.405464
			pGame->RunTo(m_PathToBoss[step - 1].x, m_PathToBoss[step - 1].y, m_PathToBoss[step - 1].z, m_PathToBoss[step - 1].speed, 0.f);
			if (IsCompleted(pGame))
				return TRUE;
			if (!allatposition && pGame->WaitAllPlayerToPosition(m_PathToBoss[step - 1], 500, 1.f))
			{
					allatposition = true;
					startTick = GetWoWTickCount();
			}
			if (!(monster = pGame->GetObject(monsters, 2, true, 15.f)).IsEmpty() || !(monster = pGame->NeedAssist()).IsEmpty())
			{
				if (!pGame->WaitUnitToPosition(monster, m_PathToBoss[step - 1], 200, 5.f))
					continue;
				pGame->Attack(monster);
				startTick = GetWoWTickCount();
			}
			//killing the patrol monster if it's close to us
			WoWGUID patrolingGuard = pGame->IsOnMonsterPatrolPath(_T("Haunted Servitor"), 40.f, 25.f);
			if (!patrolingGuard.IsEmpty())
			{
				if (!pGame->WaitUnitToPosition(monster, m_PathToBoss[step - 1], 200, 15.f))
				{
					startTick = GetWoWTickCount();
					continue;
				}
				pGame->Attack(monster);
				startTick = GetWoWTickCount();
			}
			int waitticks = allatposition ? 2000 : 5000;
			if (GetWoWTickCount() > waitticks + startTick)
				return TRUE;
		}*/
	}
	if (step == 6)
	{
		/*WoWGUID attackus = pGame->NeedAssist();
		while (!attackus.IsEmpty())
		{
			uint32 health = 0;
			while (!pGame->WaitUnitToPosition(attackus, m_PathToBoss[step - 1], 200, 5.f))
			{
				if (pGame->IsGameEnd() || pGame->IsDead())
					return TRUE;
				health = pGame->GetUnitHealth(attackus);
				if (health == 0)
				{
					attackus = pGame->NeedAssist();
					if (attackus.IsEmpty())
						break;
				}
			}
			if (!attackus.IsEmpty())
			{
				pGame->Attack(attackus);
				attackus = pGame->NeedAssist();
			}
		}*/
	}
	return TRUE;
}

BOOL BaronSilverlaine::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Baron Silverlaine is finished"));
		return TRUE;
	}
	pGame->SetMonsterExclusive(NULL);
	pGame->DisableAreaTrigger(0x966);
	pGame->Log(2, _T("Baron Silverlaine Started!"));
	WoWGUID BaronSilverlaine;
	uint32 start = GetWoWTickCount();
	while ((BaronSilverlaine = pGame->GetObject(_T("Baron Silverlaine"), true)).IsEmpty())
	{
		Sleep(200);
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
	}
	if (!(pGame->GetRole() & 2))//not tank
	{
		while (!pGame->WaitSomeonePull(BaronSilverlaine, 500))
		{
			if (pGame->IsGameEnd() || pGame->IsDead())
				return false;
			Sleep(100);
			if (GetWoWTickCount() - start > 120000)
			{
				pGame->Log(2, _T("tank is not ready"));
				return false;
			}
		}
	}
	else
	{
		while (!pGame->WaitAllPlayerToPosition(m_PathToBoss[m_PathToBoss.size() - 1], 300, 1.f) || !pGame->WaitHealerOnline(300))
		{
			if (pGame->IsGameEnd() || pGame->IsDead())
				return false;
			if (GetWoWTickCount() - start > 120000)
			{
				pGame->Log(2, _T("healer is not ready"));
				return false;
			}
			Sleep(100);
		}
		pGame->Pull(BaronSilverlaine);
	}
	//WoWPoint fight_pos(-274.026825, 2297.096436, 76.153336);
	//pGame->RunTo(fight_pos.x, fight_pos.y, fight_pos.z, 0.f, 0.f);
	WoWPoint fight_pos(-274.026825, 2297.096436, 76.153336);
	float min_distance = 3.f;
	while (!IsCompleted(pGame))
	{
		pGame->WaitUnitToPosition(BaronSilverlaine, fight_pos, 10000, min_distance);
		pGame->SetFightMode(true, false, false);
		pGame->Log(2, _T("start attack boss Baron Silverlaine!!!"));
		if (pGame->Attack(BaronSilverlaine, min_distance) != 0)
			Sleep(100);
	}
	pGame->Loot(BaronSilverlaine);
	pGame->AutoEquip();
	return TRUE;
}
CommanderSpringvale::CommanderSpringvale()
{
	m_BossRooms.push_back(BossRoom(-224.635651, 2268.130371, 102.758072, 10.f, 2.f));

	m_PathToBoss.push_back(PathStep(-274.026825, 2297.096436, 76.153336, 1.f));
	m_PathToBoss.push_back(PathStep(-275.753937, 2285.172607, 80.598976, 0.f));
	m_PathToBoss.push_back(PathStep(-290.814728, 2303.451172, 90.608582, 0.f));//AT 1916 
	m_PathToBoss.push_back(PathStep(-224.635651, 2268.130371, 102.758072, 0.f));
}
BOOL CommanderSpringvale::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
{
	if (step == 3)
	{
		uint32 startTick = GetWoWTickCount();
		bool allatposition = false;
		while (1)
		{
			if (IsCompleted(pGame))
				return TRUE;
			if (pGame->IsGameEnd() || pGame->IsDead())
				return FALSE;
			WoWGUID spitebone = pGame->GetObject(_T("Spitebone Skeleton"), true, 20.f);
			if (!spitebone.IsEmpty())
			{
				pGame->WaitUnitToPosition(spitebone, m_PathToBoss[step - 1], 30000, 15.f);
				if (pGame->GetRole() & 2)
					pGame->Pull(spitebone);
				pGame->WaitUnitToPosition(spitebone, m_PathToBoss[step - 1], 30000, pGame->GetRange());
				pGame->Attack(spitebone);
				startTick = GetWoWTickCount();
			}
			pGame->RunTo(m_PathToBoss[step - 1].x, m_PathToBoss[step - 1].y, m_PathToBoss[step - 1].z, m_PathToBoss[step - 1].speed, 0.f);
			if (!allatposition && pGame->WaitAllPlayerToPosition(m_PathToBoss[step - 1], 500, 1.f))
			{
					allatposition = true;
					startTick = GetWoWTickCount();
			}
			if (allatposition && GetWoWTickCount() > startTick + 2000)
				return TRUE;
		}
	}
	return TRUE;
}
BOOL CommanderSpringvale::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Commander Springvale is finished"));
		return TRUE;
	}
	pGame->SetMonsterExclusive(NULL);
	WoWGUID CommanderSpringvale;
	uint32 start = GetWoWTickCount();
	while ((CommanderSpringvale = pGame->GetObject(_T("Commander Springvale"), true)).IsEmpty())
	{
		Sleep(200);
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
	}

	pGame->Log(2, _T("start attack boss Commander Springvale!!!"));
	pGame->SetFightMode(true, true, true);
	pGame->AddAvoidCastingSpell("Shield of the Perfidious");
	pGame->AddAvoidGroundEffect("Desecration");
	pGame->Attack(CommanderSpringvale, 5.f);
	pGame->ResetAvoidCastingSpell();
	pGame->ResetAvoidGroundEffect();
	pGame->AutoEquip();
	return TRUE;
}

LordWalden::LordWalden()
{
	m_BossRooms.push_back(BossRoom(-146.499817, 2172.964111, 127.953384, 10.f, 2.f));


	m_PathToBoss.push_back(PathStep(-227.731369, 2256.845215, 102.755676, 1.f));
	//x=-235.806747, y=2213.677002, z=98.355301 CMSG_AREA_TRIGGER]:19 19 00 00
	//x=-235.787064, y=2206.799072, z=97.345322 CMSG_AREA_TRIGGER]:FE 00 00 00 C0 
	//x=-250.453644, y=2168.786621, z=93.935493 CMSG_AREA_TRIGGER]:18 19 00 00 C0 
	//x=-248.150192, y=2155.376221, z=92.243935 CMSG_AREA_TRIGGER]:18 19 00 00 C0
	m_PathToBoss.push_back(PathStep(-268.120453, 2148.503174, 95.876373, 1.f));
	//x=-227.526321, y=2102.975342, z=97.390251 CMSG_AREA_TRIGGER]:FF 00 00 00 C0
	//x=-192.496277, y=2140.439453, z=97.390251 CMSG_AREA_TRIGGER]:00 01 00 00 C0
	m_PathToBoss.push_back(PathStep(-183.533966, 2162.527100, 97.390251, 1.f));
	m_PathToBoss.push_back(PathStep(-126.145851, 2177.065430, 112.694931, 1.f));
	m_PathToBoss.push_back(PathStep(-171.099899, 2173.290039, 109.255737, 1.f));
	m_PathToBoss.push_back(PathStep(-146.499817, 2172.964111, 127.953384, 1.f));//
}

BOOL LordWalden::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->DisableAreaTrigger(0x966);
	pGame->SetMonsterExclusive(NULL);
	//"Oaf Lackey"//out of the room there another 2
	pGame->Log(2, _T("Lord Walden Started!"));
	if (step != 0)
	{
		if (IsCompleted(pGame) || (pGame->GetRole() & 2))
		{
			for (uint32 i = step; i < m_PathToBoss.size(); i++)
			{
				/*if (GetWoWTickCount() > startTime + 60000)
					return false;*/
				pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
			}
		}
		else
		{
			pGame->Lock();
			WoWGUID tank = pGame->GetLFGroup().GetTank();
			if (tank.IsEmpty())
				tank = pGame->GetGroup().GetTank();
			pGame->Unlock();
			pGame->AssistPlayer(tank);
			//if (GetWoWTickCount() > startTime + 60000)
			//	return false;
		}
	}
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Lord Walden is finished"));
		return TRUE;
	}
	WoWGUID LordWalden;
	uint32 start = GetWoWTickCount();
	while ((LordWalden = pGame->GetObject(_T("Lord Walden"), true)).IsEmpty())
	{
		Sleep(200);
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
	}

	pGame->Log(2, _T("start attack boss Lord Walden!!!"));
	pGame->SetFightMode(true, true, true);
	pGame->Attack(LordWalden, 0.f);
	pGame->AutoEquip();
	return TRUE;
}

LordGodfrey::LordGodfrey()
{
	//-129.505753, y=2166.969238, z=155.679031 CMSG_AREA_TRIGGER]:1F 19 00 00 C0 
	//x=-125.305008, y=2158.530518, z=155.679031 CMSG_AREA_TRIGGER]:29 19 00 00 C0 
	m_BossRooms.push_back(BossRoom(-80.418076, 2151.784668, 155.689774, 10.f, 2.f));

	m_PathToBoss.push_back(PathStep(-146.499817, 2172.964111, 127.953384, 1.f));
	m_PathToBoss.push_back(PathStep(-129.737640, 2167.168945, 138.697159, 1.f));
	m_PathToBoss.push_back(PathStep(-121.467247, 2161.364502, 155.679031, 1.f));//wait door open
	m_PathToBoss.push_back(PathStep(-80.418076, 2151.784668, 155.689774, 1.f));
}

BOOL LordGodfrey::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->SetMonsterExclusive(NULL);

	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Lord Godfrey is finished"));
		return TRUE;
	}
	pGame->DisableAreaTrigger(0x966);
	WoWGUID godfrey;
	uint32 start = GetWoWTickCount();
	while ((godfrey = pGame->GetObject(_T("Lord Godfrey"), true)).IsEmpty())
	{
		Sleep(200);
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
	}
	/*if (GetWoWTickCount() > startTime + 60000)
		return false;*/
	pGame->SetMonsterExclusive(NULL);
	/*if (GetWoWTickCount() > startTime + 60000)
		return false;*/
	pGame->SetFightMode(true, true, true);
	pGame->Lock();
	WoWGUID tank = pGame->GetWorkingGroup().GetTank();
	pGame->Unlock();
	if (!(pGame->GetRole() & 2))//not tank
	{
		while (!tank.IsEmpty() && pGame->GetUnitHealth(tank) > 0 && pGame->GetUnitTarget(tank).IsEmpty())
			Sleep(100);
	}
	else
	{
		if ((!pGame->WaitHealerToPosition(m_BossRooms[m_BossRooms.size() - 1], 15000, 50.f) && !pGame->WaitHealerToPosition(m_PathToBoss[m_PathToBoss.size() - 1], 15000, 50.f)) || !pGame->WaitHealerOnline(30000))
		{
			pGame->Log(2, _T("healer is not ready"));
			return false;
		}
	}
	pGame->AddInterruptSpell("Cursed Bulletse");
	pGame->AddAvoidCastingSpell("Pistol Barrage");
	pGame->Attack(godfrey, 0.f);
	pGame->ResetInterruptSpells();
	pGame->ResetAvoidCastingSpell();
	pGame->AutoEquip();	
	return TRUE;
}

///////////////////////////////////////////////////////////////////////
////
///////////////////////////////////////////////////////////////////////
void CShadowfangKeep::Load(IBotConfig *config)
{
	CGame::Load(config);
	TCHAR buf[512];
	if (GetPrivateProfileString(config->m_strSection, _T("Vendor"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_strVendor = buf;
}

BOOL CShadowfangKeep::Run(IGamePlay *pGame)
{
	return CDungeonGame::Run(pGame);
}

BOOL CShadowfangKeep::Prepare(IGamePlay *pGame)
{
	return TRUE;
}
