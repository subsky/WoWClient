#include "stdafx.h"
#include "Deadmines.h"
#include <vector>
#include "Opcodes.h"

Glubtok::Glubtok()
{
	m_BossRooms.push_back(BossRoom(-192.599884, -444.906769, 54.065475, 50.f));

	m_PathToBoss.push_back(PathStep(-14.573200, -385.475006, 62.456100, 1.f));
	//m_PathToBoss.push_back(PathStep(-187.261368, -426.928436, 53.924702, 1.f));//1.f
	m_PathToBoss.push_back(PathStep(-192.599884, -444.906769, 54.065475, 1.f));
}

BOOL Glubtok::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
{
	return TRUE;
}

BOOL Glubtok::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->SetMonsterExclusive(NULL);
	pGame->Log(2, _T("Glubtok Started!"));
	pGame->AutoEquip();
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Glubtok is finished"));
		return TRUE;
	}
	WoWGUID glubtok;
	uint32 start = GetWoWTickCount();
	/*if (GetWoWTickCount() > startTime + 60000)
		return false;*/
	list<LPCTSTR> monsters;
	monsters.push_back(_T("Ogre Bodyguard"));
	uint32 start_time = GetWoWTickCount();
	WoWGUID guardians[2];
	int len = 0;
	while ((len = pGame->GetObjects(_T("Ogre Bodyguard"), guardians, 2)) > 0)
	{
		if (pGame->IsGameEnd())
			return FALSE;
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
			if (GetWoWTickCount() > start_time + 60000)
			{
				pGame->Log(2, _T("killing Ogre Bodyguard timeout!!!"));
				break;
			}
			Sleep(200);
		}
	}
	glubtok = pGame->GetObject(_T("Glubtok"), true);
	while (!IsCompleted(pGame) && glubtok.IsEmpty())
	{
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
		Sleep(100);
		glubtok = pGame->GetObject(_T("Glubtok"), true);
	}
	//if (GetWoWTickCount() > startTime + 60000)
	//	return false;
	if (!pGame->IsScenarioComplete() && !IsCompleted(pGame) && !(pGame->GetRole() & 2))//not tank
	{
		pGame->Log(2, _T("waiting tank pull boss glubtok"));
		if (!pGame->WaitSomeonePull(glubtok, 30000))//target
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
	pGame->SetFightMode(true, true, true);
	pGame->Attack(glubtok, 0.f);
	pGame->ResetAvoidCastingSpell();
	pGame->ResetAvoidGroundEffect();
	pGame->ResetAvoidUnit();
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);	return TRUE;
	pGame->AutoEquip();
}

HelixGearbreaker::HelixGearbreaker()
{
	m_BossRooms.push_back(BossRoom(-291.350647, -518.363525, 49.542484, 40.f));

	m_PathToBoss.push_back(PathStep(-192.599884, -444.906769, 54.065475, 1.f));
	m_PathToBoss.push_back(PathStep(-260.448364, -482.577728, 49.447178, 1.f));//Before Heavy Door
}
class CHelixGearbreakerExclude : public CMonsterExclude
{
public:
	virtual bool Callback(IGamePlay *pGame, const WoWGUID &monster, bool loot)
	{
		if (monster.GetTypeId() != TYPEID_UNIT)
			return true;
		if (!pGame->IsAttackable((WoWGUID &)monster) || pGame->GetUnitHealth((WoWGUID &)monster) == 0)
		{
			m_excludes[monster] = 0;
		}
		return m_excludes.find(monster) != m_excludes.end();
	}

};
BOOL  HelixGearbreaker::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
{
	return TRUE;
}
BOOL HelixGearbreaker::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (IsCloseToPathPoint(pGame, 1) && (pGame->GetRole() & 2))
		pGame->OpenGameObject(_T("Heavy Door"), true);
	//"Oaf Lackey"//out of the room there another 2
	pGame->Log(2, _T("Helix Gearbreaker Started!"));

	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Helix Gearbreaker is finished"));
		return TRUE;
	}
	WoWGUID helix;
	uint32 start = GetWoWTickCount();

	CHelixGearbreakerExclude excludes;
	pGame->SetMonsterExclusive(&excludes);
	list<LPCTSTR> monsters;
	monsters.push_back(_T("Oaf Lackey"));
	uint32 start_time = GetWoWTickCount();
	int len = 0;
	pGame->SetFightMode(false, true, true);
	WoWGUID OafLackey;
	WoWGUID tank = pGame->GetLFGroup().GetTank();
	while (!(OafLackey = pGame->GetObject(_T("Oaf Lackey"), true)).IsEmpty())
	{
		if (pGame->IsGameEnd())
			return FALSE;
		if ((pGame->GetRole() & 2))
			pGame->Attack(OafLackey);
		else
		{
			pGame->Lock();
			WoWGUID tank = pGame->GetWorkingGroup().GetTank();
			pGame->Unlock();
			pGame->AssistPlayerOneTarget(tank);
		}
		/*if (GetWoWTickCount() > start_time + 60000)
		{
			pGame->Log(2, _T("killing Oaf Lackey timeout!!!"));
			break;
		}*/
		Sleep(100);
	}
	pGame->SetMonsterExclusive(NULL);
	pGame->Log(2, _T("start attack boss Lumbering Oaf/Helix Gearbreaker!!!"));
	pGame->SetFightMode(true, true, false);
	while (!IsCompleted(pGame))
	{
		helix = pGame->GetObject(_T("Lumbering Oaf"), true);
		if (!helix.IsEmpty() && pGame->GetUnitHealth(helix) > 0)
			pGame->Attack(helix, 0.f);
		helix = pGame->GetObject(_T("Helix Gearbreaker"), true);
		if (!helix.IsEmpty() && pGame->GetUnitHealth(helix) > 0)
			pGame->Attack(helix);
	}
	helix = pGame->GetObject(_T("Helix Gearbreaker"), false);
	if (!helix.IsEmpty())
		pGame->Loot(helix);
	pGame->AutoEquip();
	return TRUE;
}

FoeReaper5000::FoeReaper5000()
{
	m_BossRooms.push_back(BossRoom(-196.868866, -573.853149, 20.976995, 40.f));

	m_PathToBoss.push_back(PathStep(-291.350647, -518.363525, 49.542484, 1.f));
	m_PathToBoss.push_back(PathStep(-244.898895, -578.380005, 51.148380, 1.f));
	m_PathToBoss.push_back(PathStep(-220.052063, -597.597290, 22.386599, 1.f));//chest position
}
class CFoeReaper5000Exclude : public CMonsterExclude
{
public:
	virtual bool Callback(IGamePlay *pGame, const WoWGUID &monster, bool loot)
	{
		if (monster.GetTypeId() != TYPEID_UNIT)
			return true;
		if (!pGame->IsAttackable((WoWGUID &)monster) || pGame->GetUnitHealth((WoWGUID &)monster) == 0)
		{
			m_excludes[monster] = 0;
		}
		return m_excludes.find(monster) != m_excludes.end();
	}

};

BOOL  FoeReaper5000::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
{
	if (step == 2 && (pGame->GetRole() & 2))
		pGame->OpenGameObject(_T("Heavy Door"), true);
	return TRUE;
}
BOOL FoeReaper5000::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->SetMonsterExclusive(NULL);
	//"Defias Watcher"
	//"Defias Reaper"
	//"Foo Reaper 5000"
	pGame->Log(2, _T("Foe Reaper 5000 Started!"));
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Foe Reaper 5000 is finished"));
		return TRUE;
	}
	WoWGUID reaper5000;
	uint32 start = GetWoWTickCount();
	/*if (GetWoWTickCount() > startTime + 60000)
		return false;*/

	LPCTSTR monsters[] = { _T("Defias Watcher"),
							_T("Defias Reaper")};
	uint32 start_time = GetWoWTickCount();
	int len = 0;
	pGame->SetFightMode(false, true, false);
	CFoeReaper5000Exclude excludes;
	pGame->SetMonsterExclusive(&excludes);
	WoWGUID monster;
	pGame->Lock();
	WoWGUID tank = pGame->GetWorkingGroup().GetTank();
	pGame->Unlock();
	while (!(monster = pGame->GetObject(monsters, 2, true)).IsEmpty())
	{
		if (pGame->IsGameEnd())
			return FALSE;
		if ((pGame->GetRole() & 2))
			pGame->Attack(monster);
		else
		{
			pGame->AssistPlayerOneTarget(tank);
		}
		/*if (GetWoWTickCount() > start_time + 60000)
		{
		pGame->Log(2, _T("killing Oaf Lackey timeout!!!"));
		break;
		}*/
		Sleep(100);
	}
	pGame->SetMonsterExclusive(NULL);
	/*if (GetWoWTickCount() > startTime + 60000)
		return false;*/
	pGame->SetFightMode(true, true, false);
	while (!IsCompleted(pGame))
	{
		if (pGame->IsGameEnd() || pGame->IsDead())
			return FALSE;
		reaper5000 = pGame->GetObject(_T("Foe Reaper 5000"), true);
		if (reaper5000.IsEmpty() || !pGame->IsAttackable(reaper5000))
		{
			pGame->Log(2, _T("not found boss Foe Reaper 5000/not attackable"));
			Sleep(200);
			continue;
		}
		break;
	}

	if (!(pGame->GetRole() & 2))//not tank
	{
		while (!tank.IsEmpty() && pGame->GetUnitHealth(tank) > 0 && pGame->GetUnitTarget(tank).IsEmpty())
			Sleep(100);
	}
	else
	{
		if (!pGame->WaitHealerOnline(30000))
		{
			pGame->Log(2, _T("healer is not ready"));
			return false;
		}
		else
		{
			pGame->Lock();
			Group group = pGame->GetWorkingGroup();
			pGame->Unlock();
			uint32 current = GetWoWTickCount();
			for (list<Member>::iterator i = group.members.begin(); i != group.members.end(); i++)
			{
				if (i->guid != pGame->GetID())
				{
					WoWPoint pos;
					while (pGame->GetUnitPosition(i->guid, pos, NULL))
					{
						if (pos.z < (m_PathToBoss[2].z + 1))
							break;
						/*if (GetWoWTickCount() - 60000 > current)
						{
							pGame->Log(2, _T("wait for other player:%s in boss room failed!"), (LPCTSTR)i->name);
							return false;
						}*/
						Sleep(500);
					}
				}
			}

		}
	}

	pGame->Attack(reaper5000, 0.f);
	pGame->Loot(reaper5000);
	pGame->AutoEquip();	
	pGame->SetFightMode(true, true, true);
	return TRUE;
}

AdmiralRipsnarlVanish::AdmiralRipsnarlVanish()
{
	m_BossRooms.push_back(BossRoom(-60.616253, -818.848999, 41.512390, 10.f));//exit point

	m_PathToBoss.push_back(PathStep(-196.868866, -573.853149, 20.976995, 1.f));
	m_PathToBoss.push_back(PathStep(-107.781738, -655.816345, 7.423781, 1.f));
	m_PathToBoss.push_back(PathStep(-96.189331, -688.997620, 8.043372, 1.f));
	m_PathToBoss.push_back(PathStep(-95.852509, -723.298218, 8.886811, 1.f));

	m_PathToBoss.push_back(PathStep(-90.947250, -806.639526, 38.906326, 0.f));
	//m_PathToBoss.push_back(PathStep(-92.456772, -795.803406, 35.330044, 0.f));
	//m_PathToBoss.push_back(PathStep(-60.616253, -818.848999, 41.512390, 0.f));//1.f
}
class AdmiralRipsnarCallback : public StopAssistCallback
{
private:
	FightModule *m_pFight;
public:
	AdmiralRipsnarCallback(FightModule *pFight) : m_pFight(pFight) {};
	bool Callback(IGamePlay *pGame)
	{
		WoWPoint pos;
		pGame->GetPosition(&pos.x, &pos.y, &pos.z);
		float x = m_pFight->m_PathToBoss[3].x - pos.x;
		float y = m_pFight->m_PathToBoss[3].y - pos.y;
		if ((fabs(x) < 30.f && fabs(y) < 30.f))
			return true;
		//if (GetWoWTickCount() > (m_pFight->m_startTime + 50000))
		//	return true;
		return false;
	};
};
/*BOOL __fastcall AttackAdmiralRipsnarCallback(IGamePlay *pGame, WoWGUID &monster)
{
}*/
class AdmiralRipsnarMonsterExcludeCallback : public MonsterExcludeCallback
{
public:
	bool Callback(IGamePlay *pGame, const WoWGUID &monster, bool loot)
	{
		WoWPoint pos;
		if (pGame->GetUnitPosition(monster, pos) && pos.z < 40.f)
			return true;
		if (!loot && pGame->GetUnitHealth(monster) == 0)
			return true;
		return false;
	};
	void UpdateFailed(const WoWGUID &monster)
	{
	};
};
class ParrotExcludeCallback : public MonsterExcludeCallback
{
public:
	bool Callback(IGamePlay *pGame, const WoWGUID &monster, bool loot)
	{
		WoWPoint pos;
		if (pGame->GetUnitPosition(monster, pos))
		{
			if (fabs(pos.x + 95.852509) > 45.f || fabs(pos.y + 703.298218) > 45.f)
				return true;
			if (pos.z < 8.5f)//not in combat and beneath us
				return true;
			if (loot)
				pGame->Log(5, _T("parrot loot check pos.z:%f"), pos.z);
			if (!loot && pGame->GetUnitHealth(monster) == 0)
				return true;
			return false;
		}
		return true;
	};
	void UpdateFailed(const WoWGUID &monster)
	{
	};
};
BOOL  AdmiralRipsnarlVanish::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
{
	AdmiralRipsnarCallback cb(this);
	if (step == 2)
	{
		if (pGame->GetRole() & 2)
		{
			pGame->OpenGameObject(_T("Defias Cannon"), true);
			//player is rooted and wait the monster generated
			Sleep(3000);
		}
	}
	else if (step == 4)
	{
		LPCTSTR monsters[] = { _T("Monstrous Parrot"),
			_T("Sunwing Squawker") };
		ParrotExcludeCallback excludes;
		pGame->SetMonsterExclusive(&excludes);
		pGame->SetFightMode(false, false, false);
		WoWGUID monster;
		while (!(monster = pGame->GetObject(monsters, 2, true)).IsEmpty())
		{
			if (pGame->IsGameEnd() || pGame->IsDead())
			{
				pGame->SetMonsterExclusive(NULL);
				return FALSE;
			}
			pGame->Attack(monster);
		}
		pGame->SetMonsterExclusive(NULL);
		pGame->RunTo(m_PathToBoss[step - 1].x, m_PathToBoss[step - 1].y, m_PathToBoss[step - 1].z, m_PathToBoss[step - 1].speed, 0.f);
	}
	return TRUE;
}
BOOL AdmiralRipsnarlVanish::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->SetMonsterExclusive(NULL);
	//"Admiral Ripsnarl"
	pGame->Log(2, _T("Admiral Ripsnarl Started!"));
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Admiral Ripsnarl is finished"));
		return TRUE;
	}
	/*if (GetWoWTickCount() > startTime + 45000)
	{
		pGame->Log(2, _T("killing Admiral Ripsnarl timeout!!!"));
		return FALSE;
	}*/	
	WoWGUID AdmiralRipsnarl;
	while (!IsCompleted(pGame) && AdmiralRipsnarl.IsEmpty())
	{
		AdmiralRipsnarl = pGame->GetObject(_T("Admiral Ripsnarl"), true);
		if (AdmiralRipsnarl.IsEmpty())
			Sleep(100);
	}
	uint32 start = GetWoWTickCount();
	pGame->SetFightMode(true, false, false);
	AdmiralRipsnarMonsterExcludeCallback vaporCallback;
	pGame->SetMonsterExclusive(&vaporCallback);
	if (!(pGame->GetRole() & 2))//not tank
	{
		while (!pGame->WaitSomeonePull(AdmiralRipsnarl, 500))
		{
			WoWGUID other = pGame->NeedAssist();
			if (pGame->IsGameEnd() || pGame->IsDead())
				return false;
			if (!other.IsEmpty())
			{
				pGame->Attack(other);
				continue;
			}
			pGame->RunTo(m_PathToBoss[m_PathToBoss.size() - 1].x, m_PathToBoss[m_PathToBoss.size() - 1].y, m_PathToBoss[m_PathToBoss.size() - 1].z, 1.f, 0.f);
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
			WoWGUID other = pGame->NeedAssist();
			if (!other.IsEmpty())
			{
				pGame->Attack(other);
				continue;
			}
			pGame->RunTo(m_PathToBoss[m_PathToBoss.size() - 1].x, m_PathToBoss[m_PathToBoss.size() - 1].y, m_PathToBoss[m_PathToBoss.size() - 1].z, 1.f, 0.f);
			if (GetWoWTickCount() - start > 120000)
			{
				pGame->Log(2, _T("healer is not ready"));
				return false;
			}
			Sleep(100);
		}
	}
	while (!IsCompleted(pGame))
	{
		if (pGame->IsGameEnd() || pGame->IsDead())
			return false;
		//if (GetWoWTickCount() > startTime + 80000)
		//	return false;
		WoWGUID vapor = pGame->GetObject(_T("Vapor"), true, 50.f);
		if (!vapor.IsEmpty())
		{
			pGame->Attack(vapor, 0.f);
			continue;
		}
		AdmiralRipsnarl = pGame->GetObject(_T("Admiral Ripsnarl"), true);
		if (!AdmiralRipsnarl.IsEmpty())
		{
			pGame->Attack(AdmiralRipsnarl, 0.f);
			continue;
		}
		Sleep(100);
		//pGame->KillClosestMonster(0, &monsters, 15, -1, -1);
	}
	pGame->SetFightMode(true, true, false);
	if (!AdmiralRipsnarl.IsEmpty() && pGame->GetUnitHealth(AdmiralRipsnarl) == 0)
		pGame->Loot(AdmiralRipsnarl);
	pGame->SetMonsterExclusive(NULL);
	WoWGUID target;
	while (!(target = pGame->NeedAssist()).IsEmpty())
	{
		pGame->Attack(target);
	}

	pGame->AutoEquip();
	return TRUE;
}

CaptainCookie::CaptainCookie()
{
	m_BossRooms.push_back(BossRoom(-60.616253, -818.848999, 41.512390, 30.f));//exit point

	//m_PathToBoss.push_back(PathStep(-83.833939, -819.770020, 39.478390, 1.f));//1.f
}
BOOL  CaptainCookie::BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
{
	return TRUE;
}
BOOL CaptainCookie::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->SetMonsterExclusive(NULL);
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
	{
		pGame->Log(2, _T("Captain Cookie is finished"));
		return TRUE;
	}
	WoWGUID cookie;
	uint32 start = GetWoWTickCount();
	pGame->Log(2, _T("Captain Cookie is started"));
	cookie = pGame->GetObject(_T("\"Captain\" Cookie"), true);
	if (cookie.IsEmpty() || !pGame->IsAttackable(cookie))
	{
		if (pGame->GetRole() & 2)
			pGame->RunTo(-83.833939, -819.770020, 39.478390, 1.f, 0.f);
	}
	while (!IsCompleted(pGame) && (cookie.IsEmpty() || !pGame->IsAttackable(cookie)))
	{
		if (pGame->IsGameEnd() || pGame->IsDead())
			return false;
		//if (GetWoWTickCount() > startTime + 80000)
		//	return false;
		Sleep(100);
		cookie = pGame->GetObject(_T("\"Captain\" Cookie"), true);
	}
	pGame->SetFightMode(true, false, false);
	pGame->Attack(cookie, 0.f);
	pGame->Loot(cookie);

	//if (pGame->IsScenarioComplete())
	//	pGame->RefreshGameServer();

	return TRUE;
}
///////////////////////////////////////////////////////////////////////
////
///////////////////////////////////////////////////////////////////////
void CDeadmines::Load(IBotConfig *config)
{
	CGame::Load(config);
}

BOOL CDeadmines::Run(IGamePlay *pGame)
{
	return CDungeonGame::Run(pGame);
}

BOOL CDeadmines::Prepare(IGamePlay *pGame)
{
	return CGame::Prepare(pGame);
}
