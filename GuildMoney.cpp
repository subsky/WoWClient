#include "stdafx.h"
#include "GuildMoney.h"
#include "ShadowfangKeep.h"
#include "Config.h"

void GuildMoney::OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg)
{
	if (_tcscmp(strMsg, _T("debug")) == 0)
		debug = true;
}
void GuildMoney::Load(IBotConfig *config)
{
}
class CZeroHealthMonsterExclude : public MonsterExcludeCallback
{
public:
	virtual bool Callback(IGamePlay *pGame, const WoWGUID &monster, bool loot)
	{
		return !loot && pGame->GetUnitHealth(monster) == 0;
	}
	virtual void UpdateFailed(const WoWGUID &monster)
	{
		m_excludes[monster] = 0;
	}

	map<WoWGUID, uint32> m_excludes;
};
BOOL GuildMoney::Run(IGamePlay *pGame)
{
	if (!pGame->HasGuild() || pGame->HasCompleteChallenge())
	{
		if (pGame->HasGuild())
			pGame->QuitGuild();
		int newGuild = 0;
		while ((newGuild = pGame->RequestNewGuild()) != 0)
		{
			if (newGuild == 2)
			{
				pGame->Log(1, _T("Error: not found guild creator/not available!"));
				break;
			}
			pGame->Log(2, _T("info: may other realm visiting the guild creator? wait some seconds!"));
			Sleep(500);
		}
		while (newGuild == 0 && !pGame->HasGuild())
		{
			/*if (!m_guildCreator.IsEmpty())
			{
			if (m_pGame->GetMapID() != 0)//human only for orc, should update
			m_pGame->LFGTeleport(true);
			list<uint32> items;
			m_pGame->Trade(m_guildCreator, items, 1000);
			}*/
			Sleep(500);
		}
	}

	LordWalden walden;

	LordGodfrey godfrey;
	if (pGame->IsScenarioComplete())
	{
		return TRUE;
	}
	if (pGame->GetMapID() != pGame->GetLFGMapID())
	{
		uint32 result = 19;//TRANSFER_ABORT_ZONE_IN_COMBAT
		while (result == 19)
		{
			result = pGame->LFGTeleport(false);
			if (result != 0)
			{
				Sleep(500);
				pGame->Log(2, _T("can't zone in dungeon:%d"), result);
			}
		}
	}
	if (pGame->GetMapID() != 0x21)
	{
		pGame->Log(2, _T("Error: cant zone to map Shadowfang Keep"));
		return FALSE;
	}

	BaronAshbury ashbury;
	if (!pGame->CastSpell(_T("Dismiss Pet"), WoWGUID(0)))
	{
		//flag == 7 delete, flag == C1 castspell
		pGame->PetAction(0x03, 0x07, WoWGUID(0), NULL);
	}
	pGame->RunTo(-255.185196, 2119.805176, 81.179550, 0.f, 0.f);
	ashbury.Fight(pGame, 0, GetWoWTickCount());

	if (!pGame->CastSpell(_T("Dismiss Pet"), WoWGUID(0)))
	{
		//flag == 7 delete, flag == C1 castspell
		pGame->PetAction(0x03, 0x07, WoWGUID(0), NULL);
	}
	
	if (pGame->GetClass() == CLASS_MAGE || pGame->GetClass() == CLASS_ROGUE)
	{
		pGame->RunTo(-240.327881, 2161.715820, 90.335617, 0.f, 0.f);//for area_trigger C4 15 00 00 
		pGame->RunTo(-235.806747, 2213.677002, 98.355301, 0.f, 0.f);//for area_trigger 19 19 00 00
		pGame->RunTo(-235.787064, 2206.799072, 97.345322, 0.f, 0.f);//for area_trigger FE 00 00 00
		pGame->RunTo(-250.453644, 2168.786621, 93.935493, 0.f, 0.f);//for area_trigger 18 19 00 00
		pGame->RunTo(-227.526321, 2102.975342, 97.390251, 0.f, 0.f);//for area_trigger FF 00 00 00
		pGame->RunTo(-192.496277, 2140.439453, 97.390251, 0.f, 0.f);//for area_trigger 00 01 00 00
	}
	//x=-130.139420, y=2166.222900, z=129.191605 lord Walden to upstair door
	//
	//pGame->RunTo(-129.737640, 2167.168945, 138.697159, 0.f); upstair
	//pGame->RunTo(-126.733261, 2130.939453, 155.676010, 0.f);
	pGame->RunTo(-128.578735, 2149.302490, 156.416382, 0.f, 0.f);// cornor of room fighting the 4 monster

	LPCTSTR monsters[] = { _T("Dread Scryer"),
		_T("Spitebone Flayer") };
	uint32 start_time = GetWoWTickCount();
	int len = 0;
	pGame->SetFightMode(false, true, true);
	WoWGUID monster;
	pGame->Lock();
	WoWGUID tank = pGame->GetWorkingGroup().GetTank();
	pGame->Unlock();
	CZeroHealthMonsterExclude excludes;
	pGame->SetMonsterExclusive(&excludes);
	while (!(monster = pGame->GetObject(monsters, 2, true)).IsEmpty())
	{
		if (pGame->IsGameEnd() || pGame->IsDead())
		{
			pGame->SetMonsterExclusive(NULL);
			return FALSE;
		}
		if ((pGame->GetRole() & 2))
			pGame->Attack(monster);
		else
		{
			if (!pGame->AssistPlayerOneTarget(tank))
				Sleep(100);
		}
	}
	pGame->RunTo(-170.492310, 2182.868896, 151.905869, 0.f, 0.f);
	LPCTSTR darkCreeper[] = { _T("Dark Creeper")};
	if (!(monster = pGame->GetObject(darkCreeper, 1, true)).IsEmpty())
	{
		if (pGame->IsGameEnd())
		{
			pGame->SetMonsterExclusive(NULL);
			return FALSE;
		}
		pGame->Attack(monster);
		/*if (GetWoWTickCount() > start_time + 60000)
		{
		pGame->Log(2, _T("killing Oaf Lackey timeout!!!"));
		break;
		}*/
	}
	pGame->SetMonsterExclusive(NULL);

	pGame->RunTo(-146.499817, 2172.964111, 127.953384, 0.f, 0.f);
	walden.Fight(pGame, 0, GetWoWTickCount());
	if (!pGame->CastSpell(_T("Dismiss Pet"), WoWGUID(0)))
	{
		//flag == 7 delete, flag == C1 castspell
		pGame->PetAction(0x03, 0x07, WoWGUID(0), NULL);
	}

	pGame->RunTo(-129.505753, 2166.969238, 155.679031, 0.f, 0.f);//first AT
	pGame->RunTo(-123.320839, 2164.091797, 155.678665, 1.f, 0.f);//second AT
	pGame->RunTo(-120.858505, 2162.897217, 155.678665, 1.f, 0.f);//before door
	pGame->RunTo(-97.420067, 2167.843506, 155.689499, 0.f, 0.f);
	while (!debug)
		Sleep(200);

	return godfrey.Fight(pGame, 0, GetWoWTickCount());
}

