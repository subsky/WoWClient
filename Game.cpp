#include "stdafx.h"
#include "Game.h"
#include <math.h>
#include "Opcodes.h"


///////////////////////////////////////////////////////////////////
////
///////////////////////////////////////////////////////////////////
BOOL ExitModule::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (step != 0)
	{
		pGame->GetOnMount(FALSE);
		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
	}
	if (pGame->GetMapID() == m_instanceId)
		pGame->ChangeArea();//the position of the areatrigger we got is not the with the game, use this function in fixed style
		//pGame->ChangeArea();
	if (pGame->GetMapID() == m_outsideId)
	{
		pGame->ResetInstances();
		if (!pGame->ChangeArea() || pGame->GetMapID() != m_instanceId)
		{
			pGame->Log(2, _T("changing area failed"));
			pGame->EndGame(10);
			return FALSE;
		}
	}
	if (GetWoWTickCount() > (startTime + 45000))
	{
		pGame->EndGame(0);
		return FALSE;
	}
	return TRUE;
}
BOOL LFGExitModule::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (pGame->GetMapID() == m_instanceId)
		pGame->LeaveLFG();
	return TRUE;
}

CGame::CGame(CConfig *config) : m_bClose(FALSE), m_config(config)
{

}

CGame::~CGame()
{

}
BOOL CGame::Prepare(IGamePlay *pGame)
{
	if (pGame->IsScenarioComplete())
	{
		uint32 bag = pGame->GetBagSpace();
		uint32 minDur = pGame->GetMinDurability();
		if ((bag >> 16) < 10 || minDur < 3)
		{
			if ((bag >> 16) < 10)
				pGame->Log(2, _T("Out of free bag space, clear them now..."));
			else
				pGame->Log(2, _T("Gear almost broken, repair them now..."));

			if (m_strVendor.GetLength() > 0)
			{
				pGame->Dismount();
				uint32 trycount = 0;
				while (pGame->IsInCombat() && trycount++ < 50)
					Sleep(200);
				if (pGame->CastSpell((LPCTSTR)m_strVendor, WoWGUID()))
				{
					trycount = 0;
					while (!pGame->Repair() && trycount++ < 3)
						Sleep(1000);//waiting for the npc come(packet delay???)
				}
				else
				{
					pGame->Log(2, _T("cast summon vendor spell failed!"));
				}
				uint32 bag = pGame->GetBagSpace();
				bag = (bag >> 8) | (bag & 0x0000FF);
				pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
			}
			else
			{
				pGame->Log(2, _T("Vendor is not set, can't repair or sell!"));
			}
		}
	}
	return TRUE;
}
BOOL CGame::Run(IGamePlay *pGame)
{
	return TRUE;
}
void CGame::Load(IBotConfig *config)
{
	TCHAR buf[512];
	if (GetPrivateProfileString(config->m_strSection, _T("Vendor"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_strVendor = buf;
}
BOOL CDungeonGame::Run(IGamePlay *pGame)
{
	BOOL result = TRUE;
	uint32 startTime = GetWoWTickCount();
	WoWPoint pos;
	while (TRUE)
	{
		if (pGame->IsGameEnd())
		{
			pGame->EndGame(0);
			return FALSE;
		}
		if (pGame->GetHealth() == 0)
		{
			if (!pGame->PickCorpse() || pGame->IsScenarioComplete())
			{
				pGame->EndGame(0);
				return FALSE;
			}
		}

		if (pGame->GetMapID() == GetMapId())
		{
			pGame->GetPosition(&pos.x, &pos.y, &pos.z);
			list<FightModule *>::iterator i = m_modules.begin();
			uint32 index = -1;
			BOOL fightresult = FALSE;
			float bestDist = FLT_MAX;
			list<FightModule *>::iterator moduleIndex = m_modules.end();
			uint32 stepIndex = -1;
		BOSSFIGHT:
			for (; i != m_modules.end(); i++)
			{
				if (index == -1)
				{
					if ((*i)->IsInBossRoom(pos))
						index = 0;
					else
						index = (*i)->IsAtPathLine(pos);
				}
				if (index == -1 && !(*i)->IsCompleted(pGame))
				{
					uint32 tmp = -1;
					float d = (*i)->BestDistanceInPath(pos, tmp);
					if (d < bestDist)
					{
						moduleIndex = i;
						bestDist = d;
						stepIndex = tmp;
					}
				}
				if (index != -1)
				{
					(*i)->m_startTime = startTime;
					DisableAreaTrigger(pGame);
					pGame->ResetGameState();
					if ((*i)->m_PathToBoss.size() > 0 && !(fightresult = (*i)->RunToBossRoom(pGame, index, startTime)))
						break;
					if (!(fightresult = (*i)->Fight(pGame, index, startTime)))
						break;
					index = 1;//next boss run normal(no searching)
				}
				else
					pGame->Log(2, _T("not found suitable fight for current position!"));
			}
			if (i == m_modules.end())
			{
				if (bestDist != FLT_MAX && moduleIndex != m_modules.end() && stepIndex != -1)
				{
					(*moduleIndex)->m_startTime = startTime;
					if ((*moduleIndex)->Fight(pGame, stepIndex, startTime))
					{
						i = moduleIndex;
						i++;
						bestDist = FLT_MAX;
						moduleIndex = m_modules.end();
						stepIndex = -1;
						goto BOSSFIGHT;
					}
				}
				else
					break;
			}
			if (!fightresult)
			{
				result = FALSE;
				break;
			}
		}
	}
	return result;
}

BOOL CFishing::Run(IGamePlay *pGame)
{
	while (1) pGame->GoFishing();
	return TRUE;
}
struct Position
{
	float x, y, z;
	float GetDistance(float xx, float yy)
	{
	    float dx = xx - x;
		float dy = yy - y;
		return sqrt((dx*dx) + (dy*dy));
	};
};


bool _CSTSM::ClearBag(IGamePlay *pGame)
{
	uint32 map;
	float x, y, z, o;
	pGame->GetPosition(&x, &y, &z, &o, &map);
	if (map == 0 && abs(x - 2299.13) < 5)//东瘟疫教堂
	{
		pGame->RunTo(2287, -5324.9, 90.9, 1, 0.f);
		pGame->RunTo(2283.46, -5318.75, 88.73, 1, 0.f);
		pGame->RunTo(2279.84, -5312.60, 87.62, 1, 0.f);
	}
	if (!pGame->Repair())
		pGame->Sell();
	else
	{
		pGame->RunTo(2264.600830, -5307.969727, 82.065712, 1, 0.f);
		pGame->RunTo(2280.011230, -5308.104492, 86.703186, 1, 0.f);
	}
	pGame->ReadMail();
	return true;
}

BOOL _CSTSM::Run(IGamePlay *pGame)
{
	if (m_strMember.GetLength() > 0)
		pGame->Accept(m_strMember);

	if (pGame->PickCorpse())
	{
		//pGame->PrintGameResult(IGameTunnel::RESULT_DEATH);
		pGame->CheckAuras();
		pGame->TakeRest();
	}
	uint32 map;
	float x, y, z, o;
	pGame->GetPosition(&x, &y, &z, &o, &map);
	if (map == 0 && abs(x - 2299.13) < 5)//东瘟疫教堂
		ClearBag(pGame);

	uint32 bag = pGame->GetBagSpace();
	uint32 minDur = pGame->GetMinDurability();

	pGame->GetPosition(&x, &y, &z, &o, &map);
	if ((bag >> 16) < 5 || minDur < 3 || map == 0) //0x8B=zoneID, 东瘟疫之地
	{
		if ((bag >> 16) < 5 || minDur < 3)
		{
			if ((bag >> 16) < 5)
				pGame->Log(2, _T("Out of free bag space, clear them now..."));
			else
				pGame->Log(2, _T("Gear almost broken, repair them now..."));
			if (!pGame->UseHomeStone())
			{
				pGame->Log(2, _T("Use home stone failed"));
				return FALSE;
			}
			ClearBag(pGame);
			pGame->GetPosition(&x, &y, &z, &o, &map);
		}
		Position pos[] = 
		{
			{2287.530762, -5317.674805, 88.758583},
			{2280.335693, -5275.609375, 82.047729},
			{2333.333496, -5213.919922, 84.021767},
			{2361.529297, -5211.441406, 80.129822},
			{2424.637207, -4980.277344, 74.798851},
			{2437.698242, -4953.218750, 63.575703},
			{2412.784424, -4836.464844, 63.938396},
			{2417.373047, -4685.673340, 63.762920},
			{2484.288574, -4496.375000, 66.946289},
			{2497.970947, -4396.742676, 66.998863},
			{2561.775635, -4328.000000, 69.268417},

			{2591.726563, -4230.046875, 71.501678},
			{2654.546631, -4102.178223, 84.416092},
			{2769.705566, -4017.117432, 98.943565},
			{2919.455078, -3752.197510, 114.180634},
			{3064.872070, -3717.579834, 119.834290},
			{3168.870850, -3681.043213, 135.755447},
			{3285.373535, -3708.717041, 149.059448},
			//{3289.391113, -3708.332764, 150.524277},
			{3288.320557, -3710.255127, 152.169098},
			{3291.414795, -3705.772461, 150.743362},
			{3342.877930, -3655.412354, 155.473145},
			{3344.565430, -3635.678467, 146.195053},
			{3348.900391, -3614.208984, 144.923416},
			{3364.596436, -3592.998779, 149.353760},
			{3373.485352, -3583.187988, 148.614532},
			{3352.088623, -3528.205566, 152.478348},
			{3368.629639, -3499.095459, 154.632980},
			{3366.892578, -3496.062012, 154.132874},
			{3352.739502, -3475.717041, 144.597351},
			{3355.179932, -3451.132568, 140.848541},
			{3365.774658, -3439.542236, 142.745041},
			{0, 0, 0}
		};
		if (!pGame->IsGameEnd())
		{
			/*pGame->GetPosition(&x, &y, &z, &o, &map);
			float distance[4] = 
			{
				pos[0].GetDistance(x, y),
				pos[1].GetDistance(x, y),
				pos[2].GetDistance(x, y),
				pos[3].GetDistance(x, y)
			};
			if (distance[3] < 20 || distance[2] < 20)
			{
				pGame->RunTo(pos[2].x, pos[2].y, pos[2].z, 1.0);
				pGame->RunTo(pos[3].x, pos[3].y, pos[3].z, 1.0);
			}
			else if (map == 0)
			{
				pGame->GetOnMount(FALSE);

				if (abs(x - 2261.7) < 20 && abs(y + 5319.2) < 20)
				{
					pGame->RunTo(2502.05, -4994.1, 77.24);
					pGame->RunTo(2505.68, -4933.86, 80.5);
					pGame->RunTo(2661, -4906.77, 82.95);
					pGame->RunTo(2901.66, -4552.49, 96.88);
					pGame->RunTo(2999.71, -4390.5, 97.4);
					pGame->RunTo(3141.03, -4050.31, 104.77);
				}
				if ((abs(x - 3339.80) < 100 && abs(y + 3229.78) < 100) || (abs(x - 3392.56) < 100 && abs(y + 3361.78) < 100))
					pGame->RunTo(3392.56, -3361.78, 142.8);
				else
				{
					pGame->RunTo(pos[0].x, pos[0].y, pos[0].z);
					pGame->Dismount();
					pGame->RunTo(pos[1].x, pos[1].y, pos[1].z, 1.0);
					pGame->OpenDoor(_T("长者广场入口"), _T("城市大门钥匙"));
					pGame->RunTo(pos[2].x, pos[2].y, pos[2].z, 1.0);
					pGame->RunTo(pos[3].x, pos[3].y, pos[3].z, 1.0);
				}
			}*/
			pGame->GetPosition(&x, &y, &z, &o, &map);
			if (map == 0)
			{
				pGame->GetOnMount(FALSE);

				if ((abs(x - 3339.80) < 100 && abs(y + 3229.78) < 100) || (abs(x - 3392.56) < 100 && abs(y + 3361.78) < 100))
					pGame->RunTo(3392.56, -3361.78, 142.8, 1.f, 0.f);
				else
				{
					int i = 0;
					float d = 9999999.99;
					int minindex = -1;
					while(pos[i].x != 0)
					{
						float dd = pos[i].GetDistance(x, y);
						if (dd < d)
						{
							d = dd;
							minindex = i;
						}
						i ++;
					}
					if (minindex == -1)
						return FALSE;
					while (pos[minindex].x != 0)
					{
						if (pGame->IsDead() != 0 && !pGame->PickCorpse())
							return FALSE;
						pGame->RunTo(pos[minindex].x, pos[minindex].y, pos[minindex].z, 1.f, 0.f);
						minindex++;
					}
					pGame->RunTo(3370.320801, -3432.949951, 142.517395, 1.0, 0.f);
					pGame->RunTo(3391.717773, -3403.194824, 142.251022, 1.0, 0.f);
				}
			}
			pGame->PickCorpse();
			if (!pGame->ChangeArea())//0x8A6 enter stsm, 0x8AD out stsm, 8a9 another enterance
				return FALSE;
		}
	}
	pGame->GetPosition(&x, &y, &z, &o, &map);
	if ((abs(x - 3590) > 5 || abs(y + 3643) > 5) && (abs(x - 3392) > 5 || abs(y + 3364) > 5) && (abs(x - 3392) > 5 || abs(y + 3395) > 5))
	{
		pGame->Log(2, _T("Not Reset Correctly?...logout now!"));
		//if (++m_badreset > 1) //make sure we are in STSM
		//{
			if (!GetOutOfSTSM(pGame))
				return FALSE;
		pGame->LeaveGroup();
		CheckGroup(pGame);
			if (!pGame->ChangeArea())
				return FALSE;
		//}
		//return FALSE;
	}
	//m_badreset = 0;
	if (pGame->NeedRest())
	{
		pGame->TakeRest();
		if (pGame->NeedRest())
		{
			pGame->Log(1, _T("no more food or bandage???...please cheak"));
			CheckGroup(pGame);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL _CSTSM::GetOutOfSTSM(IGamePlay *pGame)
{
	uint32 map;
	float x, y, z, o;
	if (pGame->IsDead() != 0)
		return FALSE;
	pGame->GetPosition(&x, &y, &z, &o, &map);
	if (map == 0x149) //make sure we are in STSM
	{
		pGame->RunTo(3590.8701, -3643.2195, 138.491, 20, 1.f, 0.f);
		return pGame->ChangeArea();
	}
	return FALSE;
}

BOOL _CSTSM::KillBalnazzar(IGamePlay *pGame, BOOL resetwait)
{
	pGame->Log(1, _T("Module:Balnazzar started"));
	if (!pGame->NeedRest())
	{
		pGame->RunTo(3428, -3214, 136.5, 20.0, 1.f, 0.f);
		pGame->RunTo(3404.170898, -3057.958984, 136.515396, 20, 1.f, 0.f);//if its too fast, we cant find the boss
		pGame->CheckAuras();
		if (pGame->GetRange() < 5)
			pGame->RunTo(3413.980225, -3046.572266, 136.814468, 1.0, 0.f);
		else
			pGame->RunTo(3405, -3055, 136.5, 1.0, 0.f);
		WoWGUID guid;
		uint32 start = GetWoWTickCount();
		while(guid.IsEmpty() && GetWoWTickCount() - start < 10000)
		{
			Sleep(1000);
			guid = pGame->GetObject(_T("大十字军战士达索汉"), true);
		}
		if (!guid.IsEmpty())
		{
			pGame->Attack(guid, 0.f);
			/*guid = 0;
			start = GetWoWTickCount();
			while(guid == 0 && GetWoWTickCount() - start < 5000)
			{
				guid = pGame->GetObject(_T("巴纳扎尔"));
				Sleep(300);
			}
			if (guid)
				pGame->Attack(guid);*/
		}
		else
			pGame->Log(2, _T("Not found the desired monster!"));
		uint32 bag = pGame->GetBagSpace();
		bag = (bag >> 8) | (bag & 0x0000FF);
		pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
		pGame->TakeRest(resetwait);
		pGame->SaveItems();
		return !guid.IsEmpty() && pGame->GetUnitHealth(guid) == 0;
	}
	else
		pGame->Log(2, _T("Player doesn't have enough health/mana to start the module!"));
	return FALSE;
}
BOOL _CSTSM::KillBaronRivendare(IGamePlay *pGame, BOOL resetwait)
{
	pGame->Log(1, _T("Module:BaronRivendare started"));
	if (!pGame->NeedRest())
	{
		pGame->RunTo(4013.0, -3300, 115.0, 20.0, 1.f, 0.f);
		pGame->RunTo(4013.0, -3360.5, 115.0, 10.0, 1.f, 0.f);
		pGame->CheckAuras();
		WoWGUID guid;
		uint32 start = GetWoWTickCount();
		while(guid.IsEmpty() && GetWoWTickCount() - start < 10000)
		{
			Sleep(1000);
			guid = pGame->GetObject(_T("瑞文戴尔男爵"), true);
		}
		if (!guid.IsEmpty())
			pGame->Attack(guid, 0.f);
		else
			pGame->Log(2, _T("Not found the desired monster!"));
		uint32 bag = pGame->GetBagSpace();
		bag = (bag >> 8) | (bag & 0x0000FF);
		pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
		pGame->TakeRest(resetwait);
		pGame->SaveItems();
		return !guid.IsEmpty() && pGame->GetUnitHealth(guid) == 0;
	}
	else
		pGame->Log(2, _T("Player doesn't have enough health/mana to start the module!"));
	return FALSE;
}


BOOL CBalnazzar::Run(IGamePlay *pGame)
{
	if (!_CSTSM::Run(pGame))
		return FALSE;
	if (KillBalnazzar(pGame, FALSE))
		pGame->PrintGameResult(IGameTunnel::RESULT_SUCCESS, 0);
	CheckGroup(pGame);
	if (pGame->Logout() != 0)
	{
		if (!GetOutOfSTSM(pGame))
			return FALSE;
		return pGame->Logout() == 0;
	}
	return TRUE;
}

BOOL CSTSM::Run(IGamePlay *pGame)
{
	if (!_CSTSM::Run(pGame))
		return FALSE;
	//back door
	uint32 map;
	float x, y, z, o;
	pGame->GetPosition(&x, &y, &z, &o, &map);
	BOOL result = FALSE;
	if (abs(x - 3590) <= 5)
		result = KillBaronRivendare(pGame, TRUE) && KillBalnazzar(pGame, FALSE);
	else//front door
		result = KillBalnazzar(pGame, TRUE) && KillBaronRivendare(pGame, FALSE);
	//result = KillBalnazzar(pGame, FALSE);
	//if (abs(x - 3590) > 5)
	//	pGame->RunTo(3590, -3643, 138.49, 4.0);
	//result = KillBaronRivendare(pGame, TRUE) && KillBalnazzar(pGame, FALSE);
	if (result)
		pGame->PrintGameResult(IGameTunnel::RESULT_SUCCESS, 0);
	CheckGroup(pGame);
	if (pGame->Logout() != 0)
	{
		if (!GetOutOfSTSM(pGame))
			return FALSE;
		return pGame->Logout() == 0;
	}
	return TRUE;
}

void CGroup::Load(IBotConfig *config)
{
	CGame::Load(config);

	TCHAR buf[512];
	if (GetPrivateProfileString(config->m_strSection, _T("GroupMember"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_strMember = buf;
}

void CGroup::CheckGroup(IGamePlay *pGame)
{
	while (1)
	{
		if (pGame->IsGameEnd() || pGame->IsDead() != 0)
			return;
		pGame->Lock();
		Group group = pGame->GetGroup();
		pGame->Unlock();
		if (group.members.size() > 0)
		{
			if (group.leader == pGame->GetID())
				pGame->SetLeader(m_strMember);
			else
				return;
		}
		else
		{
			pGame->Log(2, _T("Inviting %s now..."), m_strMember); 
			pGame->Invite(m_strMember, 0);
		}
	}
}

BOOL CResetInstance::Run(IGamePlay *pGame)
{
	pGame->Accept(m_strMember);
	//run to mailbox if we're not
	WoWGUID mailbox = pGame->GetObject(19);
	if (!mailbox)//not found
	{
		uint32 map;
		float x, y, z, o;
		pGame->GetPosition(&x, &y, &z, &o, &map);
		uint8 race = pGame->GetRace();
		// x=-8949.950195, y=-132.492996
		if (race == 1 && (x + 8949.950195) < 100 && (y + 132.492996) < 100)//human
			//x=-9454.763672, y=45.283672, z=56.595818
			pGame->RunTo(-9454.763672, 45.283672, 56.595818, 30, 0.f);
		//x=-6240.319824, y=331.032990
		if ((race == 3 || race == 7) && (x + 6240.319824) < 100 && (y - 331.032990) < 100)//Dwarves, Gnomes
			// x=-5603.943848, y=-511.751740, z=401.587341
			pGame->RunTo(-5603.943848, -511.751740, 401.587341, 30, 0.f);
		//x=10311.299805, y=831.463013
		if (race == 4 && (x - 10311.299805) < 100 && (y - 831.463013) < 100)//night elf
			//x=9849.100586, y=959.061157, z=1306.769165
			pGame->RunTo(9849.100586, 959.061157, 1306.769165, 30, 0.f);
		//x=-3961.639893, y=-13931.200195, z=100.613983
		if (race == 11 && (x + 3961.639893) < 100 && (y + 13931.200195) < 100)//Draenei
			// x=-4150.364746, y=-12492.696289, z=44.672039
			pGame->RunTo(-4150.364746, -12492.696289, 44.672039, 30, 0.f);
		
		//x=-618.518005, y=-4251.669922, z=38.717999
		if ((race == 2 || race == 8) && (x + 618.518005) < 100 && (y + 4251.669922) < 100)//Orcs, Trolls
			//x=324.370728, y=-4705.667480, z=15.423505
			pGame->RunTo(324.370728, -4705.667480, 15.423505, 30, 0.f);
		//x=-2917.580078, y=-257.980011, z=52.996799
		if (race == 6 && (x + 2917.580078) < 100 && (y + 257.980011) < 100)//Tauren
			//x=-2340.329346, y=-365.470459, z=-8.407935
			pGame->RunTo(-2340.329346, -365.470459, -8.407935, 30, 0.f);
		//x=1676.349976, y=1677.449951, z=121.669998
		if (race == 5 && (x - 1676.349976) < 100 && (y - 1677.449951) < 100)//Undead 
			// x=2239.385254, y=255.986191, z=34.116501
			pGame->RunTo(2239.385254, 255.986191, 34.116501, 30, 0.f);
		//x=10349.599609, y=-6357.290039, z=33.402599
		if (race == 10 && (x - 10349.599609) < 100 && (y + 6357.290039) < 100)//Blood Elves 
			//x=9499.495117, y=-6841.269531, z=16.494564
			pGame->RunTo(9499.495117, -6841.269531, 16.494564, 30, 0.f);


	}

	uint32 lastonline = 0;

	uint32 lastreadmail = 0;

	pGame->LeaveGroup();
	bool reset = true;
	bool ingroup = false;
	while (true)
	{
		if (pGame->IsGameEnd())
			break;
		Sleep(1000);
		if (GetWoWTickCount() - lastonline > 3600000)
			pGame->LeaveGroup();
		pGame->Lock();
		Group group = pGame->GetGroup();
		pGame->Unlock();
		if (group.members.size() > 0)
		{
			if (!ingroup)
			{
				lastonline = GetWoWTickCount();
				pGame->Log(2, _T("In group now!"));
			}
			ingroup = true;
			if (group.leader == pGame->GetID() && (group.grouptype & GROUPTYPE_RAID) == 0)
				pGame->ConvertGroupToRaid(true);
			list<Member>::iterator i = group.members.begin();
			for (; i != group.members.end(); i ++)
			{
				if ((i->status & MEMBER_STATUS_ONLINE) != 0) //online
				{
					lastonline = GetWoWTickCount();
					reset = false;
					break;
				}
			}
			if (i == group.members.end())
			{
				if (!reset)
				{
					if (group.leader != pGame->GetID())
					{
						//pGame->Say(_T("Pass leader please"));
					}
					else
					{
						//sleep 1 second more
						Sleep(1000);
						pGame->Log(2, _T("Reset instances now!"));
						if (pGame->ResetInstances())
						{
							pGame->Log(2, _T("Reset instances successfully!"));
						}
						reset = true;
					}
				}
			}
		}
		else
		{
			if (ingroup)
				pGame->Log(2, _T("Group is disbanded!"));
			ingroup = false;
			//pGame->Log(2, _T("Inviting %s now..."), m_strMember); 
			//pGame->Invite(m_strMember);
		}
		if (GetWoWTickCount() - lastreadmail > 1800000)
		{
			lastreadmail = GetWoWTickCount();
			pGame->ReadMail();
		}
	};
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////////
////
//////////////////////////////////////////////////////////////////////////////////
//奥术畸兽,法力残渣
BOOL CDireMaul::Run(IGamePlay *pGame)
{
	if (m_strMember.GetLength() > 0)
		pGame->Accept(m_strMember);

	CheckGroup(pGame);

	uint32 map;
	float x, y, z, o;
	if (!pGame->PickCorpse())
	{
		pGame->GetPosition(&x, &y, &z, &o, &map);
		if (pGame->IsDead() != 0 && (x + 3908) < 10 && (y - 1130) < 10)
		{
			pGame->RunTo(-3753.244141, 1249.821167, 160.263657, 1.f, 0.f);
			//pGame->PickCorpse();
			if (!pGame->ChangeArea())
				return FALSE;
			pGame->CheckAuras();
			pGame->TakeRest();
		}
	}
	pGame->GetPosition(&x, &y, &z, &o, &map);
	//if (map == 1 && abs(x - 2299.13) < 5)//
	//	ClearBag(pGame);
	if (map != 0x1AD)
	{
		if (map != 1)
		{
			if (!pGame->UseHomeStone())
			{
				pGame->Log(2, _T("Use home stone failed"));
				return FALSE;
			}
		}
		pGame->GetPosition(&x, &y, &z, &o, &map);
	}
	if (map != 0x1AD)
	{
		if (map != 1)
		{
			pGame->Log(2, _T("We're t wrong position, exit now!!!"));
			return FALSE;
		}
		else
		{
			pGame->RunTo(-3753.244141, 1249.821167, 160.263657, 1.f, 0.f);
			//pGame->PickCorpse();
			if (!pGame->ChangeArea())
				return FALSE;
		}
		pGame->GetPosition(&x, &y, &z, &o, &map);
	}
	if (map != 0x1AD)
	{
		pGame->Log(2, _T("We're t wrong position, exit now!!!"));
		return FALSE;
	}

	/*if ((abs(x - 33.108299) > 5 || abs(y - 158.977005) > 5))// && (abs(x - 3392) > 5 || abs(y + 3364) > 5) && (abs(x - 3392) > 5 || abs(y + 3395) > 5))
	{
		pGame->Log(2, _T("Not Reset Correctly?...logout now!"));
		if (pGame->IsDead() != 0)
			return FALSE;
		pGame->RunTo(33.108299, 158.977005, -3.47126, 1.0);
		if (!pGame->ChangeArea())
			return FALSE;
		pGame->LeaveGroup();
		CheckGroup(pGame);
		if (!pGame->ChangeArea())
			return FALSE;
	}*/
	if (pGame->NeedRest())
	{
		pGame->TakeRest();
		if (pGame->NeedRest())
		{
			pGame->Log(1, _T("no more food or bandage???...please cheak"));
			CheckGroup(pGame);
			return FALSE;
		}
	}
	//run to repair place first
	pGame->RunTo(122.308342, 460.118134, -48.461609, 1.0, 0.f);

	uint32 bag = pGame->GetBagSpace();
	uint32 minDur = pGame->GetMinDurability();
	if ((bag >> 16) < 5 || minDur < 3 || map == 0)
	{
		if ((bag >> 16) < 5 || minDur < 3)
		{
			if ((bag >> 16) < 5)
				pGame->Log(2, _T("Out of free bag space, clear them now..."));
			else
				pGame->Log(2, _T("Gear almost broken, repair them now..."));
			pGame->RunTo(122.308342, 460.118134, -48.461609, 1.0, 1.f, 0.f);

			if (!pGame->Repair())
			{
				pGame->Log(2, _T("Repair failed..."));
				return FALSE;
			}
			pGame->GetPosition(&x, &y, &z, &o, &map);
		}
	}

	pGame->Log(1, _T("Module:Immol'thar started"));
	if (!pGame->NeedRest())
	{
		pGame->RunTo(-15, 811.416809, -29.535837, 1.0, 0.f);
		pGame->CheckAuras();
		//if (pGame->GetRange() < 5)
		//	pGame->RunTo(-32, 811.416809, -29.535837, 1.0);
		//else
		//	pGame->RunTo(-20, 811.416809, -29.535837, 1.0);
		WoWGUID guid;
		uint32 start = GetWoWTickCount();
		while(!guid && GetWoWTickCount() - start < 5000)
		{
			Sleep(1000);
			guid = pGame->GetObject(_T("伊莫塔尔"), true);
		}
		if (!guid.IsEmpty())
		{
			pGame->Attack(guid, 0.f);
		}
		else
			pGame->Log(2, _T("Not found the desired monster:伊莫塔尔!"));
		uint32 bag = pGame->GetBagSpace();
		bag = (bag >> 8) | (bag & 0x0000FF);
		pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
		pGame->TakeRest(TRUE);
		pGame->SaveItems();
		if (!guid || pGame->GetUnitHealth(guid) > 0)
			return FALSE;
	}
	else
	{
		pGame->Log(2, _T("Player doesn't have enough health/mana to start the module!"));
		return FALSE;
	}

	pGame->Log(1, _T("Module:Prince Tortheldrin started"));
	if (!pGame->NeedRest())
	{
		pGame->RunTo(115.855339, 639.398071, -48.109039, 1.0, 1.f, 0.f);
		pGame->CheckAuras();
		//if (pGame->GetRange() < 5)
		//	pGame->RunTo(140, 621, -48, 1.0);
		//else
		//	pGame->RunTo(155, 621, -48, 1.0);
		WoWGUID guid;
		uint32 start = GetWoWTickCount();
		while(!guid && GetWoWTickCount() - start < 5000)
		{
			Sleep(1000);
			guid = pGame->GetObject(_T("托塞德林王子"), true);
		}
		if (!guid.IsEmpty())
		{
			pGame->Attack(guid, 0.f);
		}
		else
			pGame->Log(2, _T("Not found the desired monster:托塞德林王子!"));
		uint32 bag = pGame->GetBagSpace();
		bag = (bag >> 8) | (bag & 0x0000FF);
		pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
		pGame->TakeRest(TRUE);
		pGame->SaveItems();
		if (!guid || pGame->GetUnitHealth(guid) > 0)
			return FALSE;
	}
	else
	{
		pGame->Log(2, _T("Player doesn't have enough health/mana to start the module!"));
		return FALSE;
	}

	CheckGroup(pGame);
	pGame->PrintGameResult(IGameTunnel::RESULT_SUCCESS, 0);
	if (pGame->Logout() != 0)
	{
		pGame->RunTo(33.108299, 158.977005, -3.47126, 1.0, 1.f, 0.f);
		if (!pGame->ChangeArea())
			return FALSE;
		if (!pGame->ChangeArea())
			return FALSE;
		return pGame->Logout() == 0;
	}
	return TRUE;
}

/*
2010/07/17 17:05:20:023: [Game Received - SMSG_LOGIN_VERIFY_WORLD]: mapid=0, x=1270.293091, y=-2555.072510, z=92.979973, o=0.467916
2010/07/17 17:05:25:796: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=17dcb76, x=1270.293091, y=-2555.072510, z=92.979973, o=0.467916, unknow2=0
2010/07/17 17:05:32:685: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=17de68f, x=1270.293091, y=-2555.072510, z=92.979996, o=0.467916, unknow2=339
2010/07/17 17:05:35:433: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=17df151, x=1280.793945, y=-2549.419434, z=86.194611, o=0.491478, unknow2=456

//enter dungeon
2010/07/17 17:05:47:167: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=17e1eea, x=190.819000, y=126.329002, z=137.227005, o=0.000000, unknow2=0
2010/07/17 17:05:47:184: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=17e1f5f, x=190.819000, y=126.329002, z=137.227005, o=0.000000, unknow2=0


//詹迪斯·巴罗夫 
2010/07/17 17:10:24:584: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=1825b18, x=268.073883, y=67.637306, z=95.840752, o=1.567719, unknow2=339



//传令官
2010/07/17 17:21:54:374: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=18cf94a, x=322.134003, y=93.510193, z=101.638763, o=5.782157, unknow2=339


2010/07/17 17:21:43:895 [Game Sent - CMSG_USE_ITEM]:14 0F 4C 26 0D 00 00 94 52 02 C2 01 00 80 43 00 
2010/07/17 17:21:43:896 [Game Sent - CMSG_USE_ITEM]:00 00 00 00 00 08 00 00 FF 53 5F 19 CC AD 02 10 
2010/07/17 17:21:43:899 [Game Sent - CMSG_USE_ITEM]:F1 
2010/07/17 17:21:43:900 [Game Sent - CMSG_GAMEOBJ_REPORT_USE]:53 5F 19 CC AD 02 10 F1 

//莱斯霜语
//2010/07/19 13:39:51:811: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=a2ec07, x=-0.899833, y=141.827484, z=83.900665, o=3.176688, unknow2=0
2010/07/19 13:41:34:704 [Game Sent - MSG_MOVE_HEARTBEAT]:01 00 00 00 00 00 F2 7E A4 00 43 A1 17 C2 2C EA 
2010/07/19 13:41:34:705 [Game Sent - MSG_MOVE_HEARTBEAT]:1B 43 5A 17 A7 42 65 3C 28 3F 00 00 00 00 
2010/07/19 13:41:34:721 [Game Received - SMSG_MONSTER_MOVE]:DF E5 20 41 0C 29 30 F1 00 72 83 49 C2 F7 9A 04 
2010/07/19 13:41:34:722 [Game Received - SMSG_MONSTER_MOVE]:43 65 17 A7 42 D0 B2 69 05 03 EC 8C 21 00 00 00 
2010/07/19 13:41:34:723 [Game Received - SMSG_MONSTER_MOVE]:80 03 00 10 00 00 B7 0E 00 00 06 00 00 00 8A D6 
2010/07/19 13:41:34:723 [Game Received - SMSG_MONSTER_MOVE]:08 C2 55 EC 1D 43 59 17 A7 42 0C 98 80 FF 06 50 
2010/07/19 13:41:34:724 [Game Received - SMSG_MONSTER_MOVE]:40 FF FA BF 7F FF EE 2F 7F FF EC 0F BF FF 
2010/07/19 13:41:34:725 Monster c4120e5f1300029 moved from x=-50.378365,y=132.605331,z=83.545692 with time:3767
2010/07/19 13:41:34:725 to x=-34.209511,y=157.923172,z=83.545601
2010/07/19 13:41:34:726 to x=-31.209511,y=162.673172,z=83.045601
2010/07/19 13:41:34:727 to x=-32.709511,y=160.423172,z=82.795601
2010/07/19 13:41:34:729 to x=-35.709511,y=155.673172,z=82.795601
2010/07/19 13:41:34:730 to x=-38.709511,y=151.173172,z=82.795601
2010/07/19 13:41:34:730 to x=-39.209511,y=150.173172,z=83.045601
2010/07/19 13:41:34:732 [Game Received - SMSG_UPDATE_OBJECT]:01 00 00 00 00 DF E5 20 41 0C 29 30 F1 05 00 00 
2010/07/19 13:41:34:733 [Game Received - SMSG_UPDATE_OBJECT]:00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
2010/07/19 13:41:34:734 [Game Received - SMSG_UPDATE_OBJECT]:00 00 FD 30 00 00 
2010/07/19 13:41:34:735 [Game Received - SMSG_FORCE_MOVE_ROOT]:C7 EC 8C 21 80 03 13 00 00 00 
2010/07/19 13:41:34:735 [Game Received - SMSG_STANDSTATE_UPDATE]:00 
2010/07/19 13:41:34:736 [Game Received - SMSG_AURA_UPDATE]:C7 EC 8C 21 80 03 11 4B 49 00 00 A3 3E 00 DF E5 
2010/07/19 13:41:34:737 [Game Received - SMSG_AURA_UPDATE]:20 41 0C 29 30 F1 98 3A 00 00 98 3A 00 00 
2010/07/19 13:41:34:740 [Game Sent - CMSG_FORCE_MOVE_ROOT_ACK]:EC 8C 21 00 00 00 80 03 13 00 00 00 00 08 00 00 
2010/07/19 13:41:34:741 [Game Sent - CMSG_FORCE_MOVE_ROOT_ACK]:00 00 15 7F A4 00 A2 DA 16 C2 7D 10 1C 43 5A 17 
2010/07/19 13:41:34:741 [Game Sent - CMSG_FORCE_MOVE_ROOT_ACK]:A7 42 65 3C 28 3F 00 00 00 00 
2010/07/19 13:41:34:742 [Game Sent - MSG_MOVE_STOP]:00 08 00 00 00 00 15 7F A4 00 A2 DA 16 C2 7D 10 
2010/07/19 13:41:34:742 [Game Sent - MSG_MOVE_STOP]:1C 43 5A 17 A7 42 65 3C 28 3F 00 00 00 00 


/////////////////////////////////////////////////////////////////
//城墙
/////////////////////////////////////////////////////////////////
//out entrance
2010/07/20 11:21:10:023: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=45bccb, x=-363.516968, y=3078.294434, z=-15.005056, o=1.801737, unknow2=0
//entrance
2010/07/19 14:25:46:322: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=cd0545, x=-1355.280029, y=1641.569946, z=68.274002, o=1.047198, unknow2=0

//bos 1 pos
2010/07/19 14:08:09:301: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=bce2fd, x=-1409.728149, y=1742.579956, z=80.985512, o=5.664273, unknow2=0
//地域火斥候

2010/07/19 14:59:39:944 [Game Sent - CMSG_CAST_SPELL]:64    67 18 00 00   00   00 08 00 00    FF 5D 0F 2A 50 D3 
2010/07/19 14:59:39:945 [Game Sent - CMSG_CAST_SPELL]:02 10 F1 
2010/07/19 14:59:39:946 [Game Sent - CMSG_GAMEOBJ_REPORT_USE]:5D 0F 2A 50 D3 02 10 F1 


//无疤
2010/07/19 15:01:29:248: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=edba17, x=-1151.725098, y=1732.555786, z=89.990768, o=5.810678, unknow2=339

*/

bool CRamparts::ClearBag(IGamePlay *pGame)
{
	return true;
}

BOOL CRamparts::Run(IGamePlay *pGame)
{
	if (m_strMember.GetLength() > 0)
		pGame->Accept(m_strMember);

	CheckGroup(pGame);

	uint32 map;
	float x, y, z, o;
	if (pGame->PickCorpse())
	{
		pGame->CheckAuras();
		pGame->TakeRest();
	}
	pGame->GetPosition(&x, &y, &z, &o, &map);
	//if (map == 1 && abs(x - 2299.13) < 5)//
	//	ClearBag(pGame);
	if (map != 0x21F)//ramparts
	{
		if (map != 0x212)//outlands
		{
			if (!pGame->UseHomeStone())
			{
				pGame->Log(2, _T("Use home stone failed"));
				return FALSE;
			}
		}
		pGame->GetPosition(&x, &y, &z, &o, &map);
	}
	if (map != 0x21F)
	{
		if (map != 0x212)
		{
			pGame->Log(2, _T("We're t wrong position, exit now!!!"));
			return FALSE;
		}
		else
		{
			//2010/07/20 11:21:10:023: [Game Sent - MSG_MOVE_FALL_LAND]: optype=0, uint16=0, tick=45bccb, x=-363.516968, y=3078.294434, z=-15.005056, o=1.801737, unknow2=0
			pGame->RunTo(-363.516968, 3078.294434, -15.005056, 1.f, 0.f);
			if (!pGame->ChangeArea())
				return FALSE;
		}
		pGame->GetPosition(&x, &y, &z, &o, &map);
	}
	if (map != 0x21F)
	{
		pGame->Log(2, _T("We're t wrong position, exit now!!!"));
		return FALSE;
	}

	if ((abs(x + 1355.280029) > 5 || abs(y - 1641.569946) > 5))// && (abs(x - 3392) > 5 || abs(y + 3364) > 5) && (abs(x - 3392) > 5 || abs(y + 3395) > 5))
	{
		pGame->Log(2, _T("Not Reset Correctly?...logout now!"));
		if (pGame->IsDead() != 0)
			return FALSE;
		pGame->RunTo(- 1355.280029, 1641.569946, 68.274002, 10.0, 0.f);
		if (!pGame->ChangeArea())
			return FALSE;
		pGame->LeaveGroup();
		CheckGroup(pGame);
		if (!pGame->ChangeArea())
			return FALSE;
	}
	if (pGame->NeedRest())
	{
		pGame->TakeRest();
		if (pGame->NeedRest())
		{
			pGame->Log(1, _T("no more food or bandage???...please cheak"));
			CheckGroup(pGame);
			return FALSE;
		}
	}

	pGame->Log(1, _T("Module:Vazruden started"));
	if (!pGame->NeedRest())
	{
		pGame->RunTo(-1409.728149, 1742.579956, 80.985512, 10.0, 0.f);
		pGame->CheckAuras();
		//if (pGame->GetRange() < 5)
		//	pGame->RunTo(-32, 811.416809, -29.535837, 1.0);
		//else
		//	pGame->RunTo(-20, 811.416809, -29.535837, 1.0);
		WoWGUID guids[2];
		int nLen = 0;
		uint32 start = GetWoWTickCount();
		while((nLen = pGame->GetObjects(_T("地狱火斥候"), guids, 2)) < 2 && GetWoWTickCount() - start < 5000)
		{
			Sleep(1000);
		}
		if (nLen == 2)
		{
			for (int i = 0; i < 2; i ++)
			{
				if (pGame->GetUnitHealth(guids[i]) == 0)
					continue;
				pGame->Attack(guids[i], 0.f);
			}
		}
		else
			pGame->Log(2, _T("Not found the desired monster:地域火斥候!"));

		start = GetWoWTickCount();
		WoWGUID guidV;
		while(!guidV && GetWoWTickCount() - start < 10000)
		{
			Sleep(1000);
			guidV = pGame->GetObject(_T("瓦兹德"), true);
		}
		if (!guidV.IsEmpty())
		{
			pGame->Attack(guidV, 0.f);
		}
		else
			pGame->Log(2, _T("Not found the desired monster:瓦兹德!"));
		start = GetWoWTickCount();
		WoWGUID guidN;
		while(!guidN && GetWoWTickCount() - start < 5000)
		{
			Sleep(1000);
			guidN = pGame->GetObject(_T("纳杉"), true);
		}
		if (!!guidN)
		{
			pGame->Attack(guidN, 0.f);
		}
		else
			pGame->Log(2, _T("Not found the desired monster:纳杉!"));
		if (!!guidV && pGame->GetUnitHealth(guidV) == 0 && !!guidN && pGame->GetUnitHealth(guidN) == 0)
		{
			//open loot chest
			if (!pGame->OpenChest(_T("强化魔铁箱")))
			{
				pGame->Log(2, _T("Open chest failed"));
			}
		}

		uint32 bag = pGame->GetBagSpace();
		bag = (bag >> 8) | (bag & 0x0000FF);
		pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
		pGame->TakeRest(TRUE);
		pGame->SaveItems();
		if (!guidN || pGame->GetUnitHealth(guidN) > 0)
			return FALSE;
	}
	else
	{
		pGame->Log(2, _T("Player doesn't have enough health/mana to start the module!"));
		return FALSE;
	}

	pGame->Log(1, _T("Module:Omor the Unscarred started"));
	if (!pGame->NeedRest())
	{
		pGame->RunTo(-1151.725098, 1732.555786, 89.990768, 20.0, 0.f);
		pGame->CheckAuras();
		//if (pGame->GetRange() < 5)
		//	pGame->RunTo(140, 621, -48, 1.0);
		//else
		//	pGame->RunTo(155, 621, -48, 1.0);
		WoWGUID guid;
		uint32 start = GetWoWTickCount();
		while(!guid && GetWoWTickCount() - start < 5000)
		{
			Sleep(1000);
			guid = pGame->GetObject(_T("无疤者奥摩尔"), true);
		}
		if (!!guid)
		{
			pGame->Attack(guid, 0.f);
		}
		else
			pGame->Log(2, _T("Not found the desired monster:无疤者奥摩尔 !"));
		uint32 bag = pGame->GetBagSpace();
		bag = (bag >> 8) | (bag & 0x0000FF);
		pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
		pGame->TakeRest(TRUE);
		pGame->SaveItems();
		if (!guid || pGame->GetUnitHealth(guid) > 0)
			return FALSE;
	}
	else
	{
		pGame->Log(2, _T("Player doesn't have enough health/mana to start the module!"));
		return FALSE;
	}

	CheckGroup(pGame);
	pGame->PrintGameResult(IGameTunnel::RESULT_SUCCESS, 0);
	if (pGame->Logout() != 0)
	{
		pGame->RunTo(33.108299, 158.977005, -3.47126, 10.0, 0.f);
		if (!pGame->ChangeArea())
			return FALSE;
		if (!pGame->ChangeArea())
			return FALSE;
		return pGame->Logout() == 0;
	}
	return TRUE;
}

BOOL CRamparts::GetOutOfRamparts(IGamePlay *pGame)
{
	return true;
}

/////////////////////////////////////////////////////////////////////////////
//	list<LPCTSTR> msgs;
//	msgs.push_back(_T("没有糖果就捣乱！"));
//	pGame->GossipNpc(0x00010000, msgs, FALSE);
//	Sleep(3000);//wait item push
//2010/10/28 18:14:54:111 [Game Sent - CMSG_OPEN_ITEM]:FF 1A 
//2010/10/28 18:14:54:429 [Game Received - SMSG_LOOT_RESPONSE]:AA B3 25 F6 01 00 00 41 05 00 00 00 00 01 00 A5 
//2010/10/28 18:14:54:442 [Game Sent - CMSG_ITEM_QUERY_SINGLE]:A5 4F 00 00 
//2010/10/28 18:14:54:724 [Game Received - SMSG_ITEM_QUERY_SINGLE_RESPONSE]:A5 4F 00 00 00 00 00 00 00 00 00 00 FF FF FF FF 
//2010/10/28 18:14:57:079 [Game Sent - CMSG_LOOT_RELEASE]:AA B3 25 F6 01 00 00 41 
//2010/10/28 18:14:57:122 [Game Received - SMSG_LOOT_RELEASE_RESPONSE]:AA B3 25 F6 01 00 00 41 01 
//	pGame->OpenItem(_T("糖果包"));
//	pGame->Logout();
//	pGame->EndGame(0);
//	return TRUE;
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// dupe
/////////////////////////////////////////////////////////////////////////////
/*
BOOL CDupe:: Run(IGamePlay *pGame)
{
	Mail email;
	email.receiver = _T("梦糖塘");//_T("就是他了");
	email.subject = _T("g");
	//email.body = _T("Take the gold.\nThanks once more!");
	email.COD = 0;//GetWoWTickCount();
	email.money = 1760000;//
	MailItem mailitem1, mailitem2;
	uint8 bag, slot;
	mailitem1.guid = pGame->GetInventoryItem(_T("大块的硬面包"), bag, slot);//pGame->GetInventoryItem(_T("蜂蜜面包"), bag, slot);
	mailitem1.index = email.items.size();
	mailitem2.guid = mailitem1.guid;
	mailitem2.index = email.items.size();
	pGame->SendMail(email);
	Sleep(500);
	email.money = 0;
	email.items.push_back(mailitem1);
	//email.items.push_back(mailitem2);
	//uint64 mailbox = pGame->GetObject(19);//mailbox
	pGame->SendMail(email);
	pGame->SwapItem(0xFF, 0x17, 0xFF, 0x1A);
	//pGame->SplitItem(_T("蜂蜜面包"), 1);
	//pGame->Logout();
	return TRUE;
}
*/
void CDupe::OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg)
{
	if (_tcscmp(strMsg, _T("ServerSwitched")) == 0)
	{
		WoWGUID vendor = pGame->GetObject(_T("Godric Rothgar"), true);
		if (vendor.IsEmpty())
		{
			pGame->Log(2, _T("buyback failed:Not found vendor"));
			return;
		}		
		ByteBuffer packet;
		WoWGUID itemid4A = pGame->GetEquipment(0x4A);
		if (!!itemid4A)
		{
			packet << vendor << (uint32)0x004A;
			pGame->SendPacket(CMSG_BUY_BACK_ITEM, packet);
		}
	}
}

BOOL CDupe:: Run(IGamePlay *pGame)
{
	/*if (pGame->Logout() == 0)
	{
		pGame->CancelLogout();
		//pGame->SendMail(_T("随意取"), _T("测试"), _T("简单测试"), 1);
	}*/
	float x, y, z;
	pGame->GetPosition(&x, &y, &z);
	float height = pGame->GetHeight(x, y, z);

	pGame->Accept(_T("术桶天下"));
	WoWGUID vendor = pGame->GetObject(_T("Godric Rothgar"), false);
	if (vendor.IsEmpty())
	{
		pGame->Log(2, _T("Not found vendor"));
		return false;
	}
	pGame->ListInventory(vendor);
	pGame->Log(2, _T("client is ready"));

	Sleep(-1);
	//pGame->RunTo(-604, -4612, 41, 1);

	/*uint8 bag, slot;
	WoWGUID itemid = pGame->GetInventoryItem(m_item, bag, slot);
	uint32 itemspell = 0;

	Mail email;
	email.receiver = m_receiver;//_T("就是他了");
	email.subject = _T("g");
	//email.body = _T("Take the gold.\nThanks once more!");
	email.COD = 0;//GetWoWTickCount();
	uint32 coinage = pGame->GetCoinage();
	email.money = 0;
	if (coinage > 10000)
	{
		//email.money = coinage - 10000;
		//coinage = 10000 - 30;//postage cost
	}
	MailItem mailitem1, mailitem2;
	mailitem1.index = email.items.size();
	mailitem2.guid = mailitem1.guid;
	mailitem2.index = email.items.size();
	//email.items.push_back(mailitem1);
	//email.items.push_back(mailitem2);
	//uint64 mailbox = pGame->GetObject(19);//mailbox
	//pGame->SendMail(email, TRUE);
	//email.money = (coinage > 30 ? coinage - 30 : 0);//
	//email.items.push_back(mailitem1);
	/*pGame->SendMail(email, FALSE);
	//pGame->UseItem(bag, slot, itemid, 0x00C060, 0, 0, FALSE);
	pGame->SwapItem(0xFF, 0x17, 0xFF, 0x1A);
	ByteBuffer packet;
	pGame->SendPacket(CMSG_LOGOUT_REQUEST, packet);
	pGame->SendPacket(CMSG_LOGOUT_CANCEL, packet);*/
	/*WoWGUID seller = pGame->GetObject(m_seller);
	ByteBuffer packet(8);
	for (int i = 0; i < 100000; i ++)
	{
		Sleep(2000);

		uint32 itemcount = pGame->GetItemCount(m_crystallizedItem);
		pGame->Log(3, _T("now we have item count %d"), itemcount);

		WoWGUID itemid4A = pGame->GetEquipment(0x4A);
		if (!!itemid4A)
		{
			packet.clear();
			packet << seller << (uint32)0x004A;
			pGame->SendPacket(CMSG_BUY_BACK_ITEM, packet);
			Sleep(2000);
		}
		//while(!!(itemid = pGame->GetInventoryItem(m_item, bag, slot)))
		//{
		//	pGame->UseItem(bag, slot, itemid, WoWGUID(), 0);
		//	Sleep(2000);
		//}
		if (pGame->GetItemCount(m_crystallizedItem) < 15)
		{
			//pGame->Log(2, _T("the count of %s is less than 10!"), m_crystallizedItem);
			//return false;
		}
		bag = 0xFF; slot = 0x17;
		/*while (!!(itemid = pGame->GetNextInventoryItem(m_crystallizedItem, bag, slot)))
		{
			uint32 count = pGame->GetItemCount(itemid);
			uint8 nextbag = bag; 
			uint8 nextslot = slot + 1;
			while (count > 5)
			{
				WoWGUID nextitem = pGame->GetNextInventoryItem(m_crystallizedItem, nextbag, nextslot);
				if (!nextitem)
				{
					if (!pGame->GetFreeInventorySlot(nextbag, nextslot))
					{
						pGame->Log(2, _T("could not found free bag slot"));
						return false;
					}
					pGame->SplitItem(bag, slot, nextbag, nextslot, count - 5);
					count -= 5;
					Sleep(2000);
					break;
				}
				else
				{
					uint32 nextcount = pGame->GetItemCount(nextitem);
					if (nextcount < 5)
					{
						uint8 splitcount = ((count - 5 + nextcount) > 5 ? 5 - nextcount : count - 5);
						pGame->SplitItem(bag, slot, nextbag, nextslot, splitcount);
						Sleep(2000);
						count -= splitcount;
					}
				}
				nextslot ++;
			}
			slot ++;
		}*/
		/*while (pGame->GetItemCount(m_crystallizedItem) < 15)
		{
			if ((itemid = pGame->GetNextInventoryItem(m_crystallizedItem, bag, slot)).IsEmpty())
			{
				bag = 0xFF;
				slot = 0x17;
				if ((itemid = pGame->GetNextInventoryItem(m_item, bag, slot)).IsEmpty())
					return false;
				else
				{
					pGame->Log(2, _T("Use item :%s"), (LPCTSTR)m_item);
					pGame->UseItem(bag, slot, itemid, WoWGUID(), 0);
					Sleep(2000);
				}
			}
		}
		bag = 0xFF;
		slot = 0x17;
		if ((itemid = pGame->GetNextInventoryItem(m_crystallizedItem, bag, slot)).IsEmpty())
			return false;
		uint32 count = pGame->GetItemCount(itemid);
		uint8 nextbag = bag; 
		uint8 nextslot = slot + 1;
		uint32 count2 = 0;
		WoWGUID nextitem = pGame->GetNextInventoryItem(m_crystallizedItem, nextbag, nextslot);
		if (!nextitem.IsEmpty())
			count2 = pGame->GetItemCount(nextitem);
		else if (count > 9)
		{
			if (!pGame->GetFreeInventorySlot(nextbag, nextslot))
			{
				pGame->Log(2, _T("could not found free bag slot"));
				return false;
			}
			pGame->Log(2, _T("Split item :%s"), (LPCTSTR)m_crystallizedItem);
			pGame->SplitItem(bag, slot, nextbag, nextslot, 5);
			count -= 5;
			Sleep(2000);
			nextitem = pGame->GetNextInventoryItem(m_crystallizedItem, nextbag, nextslot);
			count2 = pGame->GetItemCount(nextitem);
		}
		uint8 nextslot2 = max(slot, nextslot) + 1;
		nextitem = pGame->GetNextInventoryItem(m_crystallizedItem, nextbag, nextslot2);
		if (!nextitem && count > 4)
		{
			if (!pGame->GetFreeInventorySlot(nextbag, nextslot2))
			{
				pGame->Log(2, _T("could not found free bag slot"));
				return false;
			}
			pGame->Log(2, _T("Split item :%s"), (LPCTSTR)m_crystallizedItem);
			pGame->SplitItem(bag, slot, nextbag, nextslot2, 2);
			count -= 2;
			Sleep(2000);
		}		
		bag = 0xFF;
		slot = 0x17;
		if ((itemid = pGame->GetNextInventoryItem(m_item, bag, slot)).IsEmpty())
			return false;
		count = pGame->GetItemCount(itemid);
		nextbag = bag; 
		nextslot = slot + 1;
		nextitem = pGame->GetNextInventoryItem(m_item, nextbag, nextslot);
		if (nextitem.IsEmpty() && count > 1)
		{
			if (!pGame->GetFreeInventorySlot(nextbag, nextslot))
			{
				pGame->Log(2, _T("could not found free bag slot"));
				return false;
			}
			pGame->Log(2, _T("Split item :%s"), (LPCTSTR)m_item);
			pGame->SplitItem(bag, slot, nextbag, nextslot, 1);
			Sleep(2000);
		}
		uint8 firstbag, firstslot;
		WoWGUID first = pGame->GetInventoryItem(m_crystallizedItem, firstbag, firstslot);
		if (!first)
			return false;
		uint8 secondbag = firstbag;
		uint8 secondslot = firstslot + 1;
		WoWGUID second = pGame->GetNextInventoryItem(m_crystallizedItem, secondbag, secondslot);
		if (!second)
			return false;
		uint8 thirdbag = secondbag;
		uint8 thirdslot = secondslot + 1;
		WoWGUID third = pGame->GetNextInventoryItem(m_crystallizedItem, thirdbag, thirdslot);
		if (!third)
			return false;
		if (itemspell == 0)
		{
			_SpellEntry *spell = pGame->GetItemSpell(second);
			if (!spell)
			{
				pGame->Log(2, _T("could not find the item spell"));
				return false;
			}
			itemspell = spell->spell_entry->ID;
		}
		packet.clear();
		//packet << seller << itemid << (uint32)0;
		//pGame->SendPacket(CMSG_SELL_ITEM, packet);
		mailitem1.guid = itemid;
		email.items.clear();
		email.items.push_back(mailitem1);
		//pGame->SendMail(email, FALSE);*/
	/*WoWGUID playerid = pGame->GetPlayer(_T("Marrah"));
	if (!playerid.IsEmpty())
	{
		list<uint32> tradeItems;
		//tradeItems.push_back(secondbag | (secondslot << 16));
		pGame->Trade(playerid, tradeItems, 100000);
		pGame->TradeAccept();
	}*/
	/*
	uint8 itemBag, itemSlot;
	WoWGUID itemid = pGame->GetInventoryItem(m_item, itemBag, itemSlot);
	itemSlot += 1;
	itemid = pGame->GetNextInventoryItem(m_item, itemBag, itemSlot);
	uint32 itemspell2 = 0;
	if (!itemid.IsEmpty())
	{
		_SpellEntry *spell = pGame->GetItemSpell(itemid);
		if (!spell)
		{
			pGame->Log(2, _T("could not find the item spell"));
			return false;
		}
		itemspell2 = spell->spell_entry->ID;
	}
	pGame->Log(2, _T("Start duping"));
	pGame->UseItem(secondbag, secondslot, second, itemspell, WoWGUID(), 0, FALSE);
	if (!itemid.IsEmpty())
		pGame->UseItem(itemBag, itemSlot, itemid, itemspell2, WoWGUID(), 0, FALSE);
	pGame->UseItem(thirdbag, thirdslot, third, itemspell, WoWGUID(), 0, FALSE);
	Sleep(2000);
		/*
		uint64 itemid17 = pGame->GetEquipment(0x17);
		uint32 count17 = pGame->GetItemCount(itemid17);
		uint64 itemid1A = pGame->GetEquipment(0x1A);
		uint32 count1A = pGame->GetItemCount(itemid1A);
		if (count17 > 1 && count1A == 0)
		{
			pGame->SplitItem(0xFF, 0x17, 0xFF, 0x1A, 1);
			Sleep(2000);
			itemid1A = pGame->GetEquipment(0x1A);
			count1A = pGame->GetItemCount(itemid1A);
			if (count1A == 0)
				return false;
		}
		if (count17 == 0 && count1A > 1)
		{
			pGame->SplitItem(0xFF, 0x1A, 0xFF, 0x17, count1A - 1);
			Sleep(2000);
			itemid17 = pGame->GetEquipment(0x17);
			count17 = pGame->GetItemCount(itemid17);
			if (count17 == 0)
				return false;
		}
		packet.clear();
		packet << seller << itemid17 << (uint32)0;
		pGame->SendPacket(CMSG_SELL_ITEM, packet);
		pGame->SwapItem(0xFF, 0x17, 0xFF, 0x1A);*/
	//}
	return true;
}

void CDupe::Load(IBotConfig *config)
{
	CGame::Load(config);
	TCHAR buf[512];
	if (GetPrivateProfileString(config->m_strSection, _T("MailReceiver"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_receiver = buf;
	if (GetPrivateProfileString(config->m_strSection, _T("Item"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_item = buf;
	if (GetPrivateProfileString(config->m_strSection, _T("Character"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_character = buf;
	if (GetPrivateProfileString(config->m_strSection, _T("Seller"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_seller = buf;
	if (GetPrivateProfileString(config->m_strSection, _T("CrystallizedItem"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_crystallizedItem = buf;
}

void __fastcall _UpdateExcludes(map<WoWGUID, uint32> *exclues, map<WoWGUID, uint32> *newDatas)
{
	*exclues = *newDatas;
}

BOOL CTest::Run(IGamePlay *pGame)
{
	//if (pGame->GetTryCount() == 1)
	//pGame->SetStopTime(90000);
	float x, y, z;
	pGame->GetPosition(&x, &y, &z);
	pGame->Log(2, _T("at position x:%f y:%f z:%f\n"), x, y, z);
	//pGame->RunTo(x+ 20, y+20, z, 10.f);

	WoWGUID guid;
	CMonsterExclude exclude;
	pGame->SetMonsterExclusive(&exclude);
	list<LPCTSTR> monsters;
	//monsters.push_back(_T("Shadowmoon Void"));
	//monsters.push_back(_T("Void-Torn S"));
	//pGame->SetVillagePos(202.955154, -484.324921, -4.277106);
	monsters.push_back(_T("Stone"));
	//monsters.push_back(_T("Training Dummy"));

	float patrol_size = 0;
	float patrol_points[][3] = 
	{
		93.131119, -745.432678, 16.632563,
		-60.114929, -812.968689, 12.041322,
		-259.041016, -700.167908, 15.455261,
		-51.492680, -630.308533, 19.392284,
	};
	srand(GetWoWTickCount());
	while(true)
	{
		SYSTEMTIME datetime;
		GetLocalTime(&datetime);
		DWORD current = datetime.wHour * 3600 + datetime.wMinute * 60 + datetime.wSecond;
		DWORD current2 = current + 24 * 3600;
		
		DWORD StartTime = rand() % (15 * 60);
		DWORD EndTime = 6 * 3600 + rand() % (15 * 60);
		if ((current >= StartTime && current < EndTime) || (current2 >= StartTime && current2 < EndTime))
		{
			DWORD nPeriod = EndTime - current;
			if (EndTime < current)
				nPeriod += 24 * 3600;
			pGame->Logout();
			pGame->EndGame(nPeriod);
			return TRUE;
		}
		if (pGame->IsTimerOut())
		{
			pGame->RefreshGameServer();
			return FALSE;
		}
		if (pGame->IsGameEnd())
			return FALSE;
		if (pGame->IsDead() != 0)
		{
			if (!pGame->PickCorpse())
			{
				pGame->Log(2, _T("pickup corpse failed!"));
				return FALSE;
			}
		}
		uint32 bag = pGame->GetBagSpace();
		uint32 minDur = pGame->GetMinDurability();
		if ((bag >> 16) < 2 || minDur < 3) 
		{
			if ((bag >> 16) < 2 || minDur < 3)
			{
				if ((bag >> 16) < 2)
					pGame->Log(2, _T("Out of free bag space, clear them now..."));
				else
					pGame->Log(2, _T("Gear almost broken, repair them now..."));
				if (!pGame->Repair())
					pGame->Sell();
			}
		}
		pGame->GetObject(_T("Stone"), false).IsEmpty();
		if (!pGame->KillClosestMonster(0, &monsters, 500, 1, 105))
		{
			pGame->Log(2, _T("not found monsters, start patrol!"));
			pGame->GetPosition(&x, &y, &z);
			float min = FLT_MAX;
			int index = -1;
			for (int i = 0; i < patrol_size; i ++)
			{
					float dx = x - patrol_points[i][0];
					float dy = y - patrol_points[i][1];
					float dist = sqrt((dx*dx) + (dy*dy));
					if (dist < min)
					{
						min = dist;
						index = i;
					}
			}
			if (index != -1)
			{
				index ++;
				if (index == patrol_size)
					index = 0;
				pGame->PatrolTo(0, &monsters, -1, -1, patrol_points[index][0], patrol_points[index][1], patrol_points[index][2]);
			}
			else
				Sleep(100);
		}
			//pGame->EndGame(-1);
	}
	pGame->SetMonsterExclusive(NULL);
	return TRUE;
}