#include "stdafx.h"
#include "MonsterLevel.h"
#include <vector>


struct FightSpot
{
	FightSpot() : level(0), x(0), y(0), z(0), runned(false) {};
	FightSpot(uint8 nlevel, float fx, float fy, float fz) : level(nlevel), x(fx), y(fy), z(fz), runned(false) {};
	uint8 level;
	float x, y, z;
	bool runned;
};


void CMonsterLevelup::Load(IBotConfig *config)
{
	CGame::Load(config);
	TCHAR buf[512];
	if (GetPrivateProfileString(config->m_strSection, _T("Param"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		m_xmlfile = buf;
	}
	if (GetPrivateProfileString(config->m_strSection, _T("Vendor"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		m_home.m_strVendor = buf;
	}
	if (GetPrivateProfileString(config->m_strSection, _T("Friend"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		m_friend = buf;
	}
	if (GetPrivateProfileString(config->m_strSection, _T("Assist"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		m_assist = buf;
	}
	m_capLevel = 18;
	if (GetPrivateProfileString(config->m_strSection, _T("Cap Level"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_capLevel = _tcstol(buf, 0, 10);
	m_desiredSpec = 0;
	if (GetPrivateProfileString(config->m_strSection, _T("Class Spec"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		m_desiredSpec = _tcstol(buf, 0, 10);
	}
}
void CMonsterLevelup::OnMessage(IGamePlay *pGame, int type, const WoWGUID &player, LPCTSTR strMsg)
{
	//if (type != CHAT_MSG_PARTY && type != CHAT_MSG_PARTY_LEADER)//Party message or PartyLeader message
	//	return;
	if (_tcscmp(_T("MSelling"), strMsg) == 0)
	{
		m_selling = true;
		if (pGame)
			pGame->StopAssist();
	}
}
int GetLevel(IGamePlay *pGame)
{
	Group &group = pGame->GetGroup();
	if (group.id.IsEmpty())
		return pGame->GetLevel();
	list<WoWGUID> members;
	pGame->Lock();
	int level = 200;
	for (list<Member>::iterator i = group.members.begin(); i != group.members.end(); i++)
	{
		members.push_back(i->guid);
	}
	pGame->Unlock();
	for (list<WoWGUID>::iterator i = members.begin(); i != members.end(); i++)
	{
		int l = pGame->GetMemberLevel(*i);
		if (l != -1 && l != 0 && l < level)
			level = l;
	}
	return level;
}
void CMonsterLevelup::Clear()
{
	m_monster.factions.clear();
	m_monster.strFactions.clear();
	m_monster.exclues.clear();
	m_monster.m_PathToBoss.clear();
	m_home.m_BossRooms.clear();
	m_home.m_PathToBoss.clear();
}

BOOL CMonsterLevelup::Run(IGamePlay *pGame)
{
	Clear();
	m_xml.Load((LPCTSTR)m_xmlfile);
	if (m_desiredSpec)
		pGame->SetDesiredSpec(m_desiredSpec);
	if (m_xml.FindElem(_T("HBProfile")))
		m_xml.IntoElem();
	while (m_xml.FindElem(_T("SubProfile")) && m_xml.IntoElem())
	{
		int level = GetLevel(pGame);
		if (m_xml.FindElem(_T("MinLevel")) && _tcstol((LPCTSTR)m_xml.GetData(), 0, 10) > level ||
			m_xml.FindElem(_T("MaxLevel")) && _tcstol((LPCTSTR)m_xml.GetData(), 0, 10) < level)
		{
			m_xml.OutOfElem();
			continue;
		}
		if (m_xml.FindElem(_T("Vendors")) && m_xml.IntoElem())
		{
			if (m_xml.FindElem(_T("Vendor")))
			{
				float x = _tstof((LPCTSTR)m_xml.GetAttrib(_T("X")));
				float y = _tstof((LPCTSTR)m_xml.GetAttrib(_T("Y")));
				float z = _tstof((LPCTSTR)m_xml.GetAttrib(_T("Z")));
				//pGame->SetVillagePos(village[0], village[1], village[2]);
				m_home.m_BossRooms.push_back(BossRoom(x, y, z, 100.f));
			}
			m_xml.OutOfElem();
		}
		if (m_xml.FindElem(_T("PathToBattle")) && m_xml.IntoElem())
		{
			while (m_xml.FindElem(_T("Hotspot")))
			{
				WoWPoint spot(_tstof((LPCTSTR)m_xml.GetAttrib(_T("X"))), _tstof((LPCTSTR)m_xml.GetAttrib(_T("Y"))), _tstof((LPCTSTR)m_xml.GetAttrib(_T("Z"))));
				m_home.m_PathToBoss.push_back(PathStep(spot.x, spot.y, spot.z, 1.f));
			}
			m_xml.OutOfElem();
		}
		while (m_xml.FindElem(_T("GrindArea")) && m_xml.IntoElem())
		{
			if (m_xml.FindElem(_T("Factions")))
			{
				CString Factions = m_xml.GetData();
				LPCTSTR number = (LPCTSTR)Factions;
				while (number && number[0])
				{
					LPTSTR next = 0;
					long faction = _tcstol(number, &next, 10);
					if (faction != 0)
						m_monster.factions.push_back(faction);
					else
						break;
					if (next && next[0] == _T(' '))
						next ++;
					number = next;
				}
				TCHAR comma;
				while(number && number[0])
				{
					comma = number[0];
					if (comma == _T('"'))
						number ++;
					CString strFaction;
					while (number[0] && (comma == _T('"') ? number[0] != _T('"') : (number[0] != _T(' ') && number[0] != _T(','))))
					{
						strFaction += number[0];
						number ++;
					}
					if (strFaction.GetLength() > 0)
						m_monster.strFactions.push_back(strFaction);
					if (number[0] == '"')
						number++;
					while (number[0] == ' ' || number[0] == ',')//bypass ' ' or '"'
						number ++;
				}
			}
			if (m_xml.FindElem(_T("TargetMinLevel")))
				m_monster.minLevel = _tcstol((LPCTSTR)m_xml.GetData(), 0, 10);
			if (m_xml.FindElem(_T("TargetMaxLevel")))
				m_monster.maxLevel = _tcstol((LPCTSTR)m_xml.GetData(), 0, 10);
			if (m_xml.FindElem(_T("Hotspots")) && m_xml.IntoElem())
			{
				while (m_xml.FindElem(_T("Hotspot")))
				{
					WoWPoint spot(_tstof((LPCTSTR)m_xml.GetAttrib(_T("X"))), _tstof((LPCTSTR)m_xml.GetAttrib(_T("Y"))), _tstof((LPCTSTR)m_xml.GetAttrib(_T("Z"))));
					m_monster.m_PathToBoss.push_back(PathStep(spot.x, spot.y, spot.z, 1.f));
				}
				m_xml.OutOfElem();
			}
			if (!Fight(pGame))
				return FALSE;
			Clear();
			m_xml.OutOfElem();
		}
		m_xml.OutOfElem();
	}
	return TRUE;
}
class MonsterLevelCallback : public StopAssistCallback
{
protected:
public:
	MonsterLevelCallback(uint32 capLevel) : lastCheckBagTick(0), lastCheckLevelTick(0), m_capLevel(capLevel) {};
	bool Callback(IGamePlay *pGame)
	{
		uint32 current = GetWoWTickCount();
		if (current > (lastCheckBagTick + 30000))
		{
			lastCheckBagTick = current;
			uint32 bag = pGame->GetBagSpace();
			uint32 minDur = pGame->GetMinDurability();
			if ((bag >> 16) < 10 || minDur < 3)
				return true;
		}
		if (current >(lastCheckLevelTick + 60000))
		{
			lastCheckLevelTick = current;
			if (GetLevel(pGame) >= m_capLevel)
				return true;
		}
		return false;
	};
	uint32 lastCheckBagTick;
	uint32 lastCheckLevelTick;
	uint32 m_capLevel;
};
BOOL CMonsterLevelup::Fight(IGamePlay *pGame)
{
	//pGame->PrintGameResult(IGameTunnel::RESULT_SUCCESS, pGame->GetLevel());
	//pGame->SetStopTime(90000);
	WoWGUID guid;

	int level = 200;
	if ((level = GetLevel(pGame)) >= m_capLevel)
	{
		pGame->Log(2, _T("Cap level reached!"));
		pGame->UseHomeStone();
		pGame->EndGame(-1, _T("Standby"));
		return FALSE;
	}
	uint32 lastLevelTicks = GetWoWTickCount();
	srand(GetWoWTickCount());
	while (((lastLevelTicks + 60000) > GetWoWTickCount() || ((level = GetLevel(pGame)) && (lastLevelTicks = GetWoWTickCount()))) && level >= m_monster.minLevel && level <= m_monster.maxLevel && level < m_capLevel)
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
			//pGame->Logout();
			//pGame->EndGame(nPeriod);
			//return FALSE;
		}
		if (pGame->IsTimerOut())
		{
			WoWGUID players[10];
			float distance[10];
			list<LPCTSTR> excludes;
			if (m_friend.GetLength() > 0)
				excludes.push_back((LPCTSTR)m_friend);
			uint32 len = 10;
			if ((len = pGame->GetClosestPlayer(players, distance, len, excludes)) > 0)
			{
				for (uint32 i = 0; i < len; i ++)
				{
					if (distance[i] < 50.f)
					{
						//pGame->EndGame(10*60*1000);
						//return FALSE;
					}
				}
			}
			pGame->RefreshGameServer();
			return FALSE;
		}
		if (pGame->IsGameEnd())
		{
			pGame->RefreshGameServer();
			return FALSE;
		}
		if (pGame->IsDead() != 0)
		{
			if (!pGame->PickCorpse())
			{
				pGame->Log(2, _T("pickup corpse failed!"));
				pGame->RefreshGameServer();
				return FALSE;
			}
		}
		pGame->AutoEquip();
		WoWPoint pos;
		pGame->GetPosition(&pos.x, &pos.y, &pos.z);
		uint32 bag = pGame->GetBagSpace();
		uint32 minDur = pGame->GetMinDurability();
		if ((bag >> 16) < 10 || minDur < 3 || m_selling) 
		{
			if ((bag >> 16) < 10)
			{
				pGame->Log(2, _T("Out of free bag space, clear them now..."));
				//pGame->ChatMessageParty(_T("MSelling"));
				pGame->SendInternalMessage(WoWGUID(), _T("MSelling"), 0);
			}
			else if (minDur < 3)
			{
				pGame->Log(2, _T("Gear almost broken, repair them now..."));
				//pGame->ChatMessageParty(_T("MSelling"));
				pGame->SendInternalMessage(WoWGUID(), _T("MSelling"), 0);
			}
			if (m_home.Fight(pGame, -1, GetWoWTickCount()))
				m_selling = false;
		}
		bag = (bag >> 8) | (bag & 0x0000FF);
		pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);

		//if we are on the way to battle field(just back from home)
		uint32 index = -1;
		while ((index = m_home.IsAtPathLine(pos)) != -1)
		{	
			if (pGame->IsTimerOut() || pGame->IsGameEnd() || pGame->IsDead() != 0)
				break;
			if (index == m_home.m_PathToBoss.size())
				break;
			if (m_home.DistanceTo(pGame, m_home.m_PathToBoss[index]) < 3.f)
				index ++;
			if (index >= m_home.m_PathToBoss.size())
				break;
			pGame->RunTo(m_home.m_PathToBoss[index].x, m_home.m_PathToBoss[index].y, m_home.m_PathToBoss[index].z, m_home.m_PathToBoss[index].speed, 0.f);
			pGame->GetPosition(&pos.x, &pos.y, &pos.z);
		}

		pGame->SetRole(pGame->GetRole() | PLAYER_ROLE_TANK);
		WoWGUID assist = pGame->GetPlayer((LPCTSTR)m_assist);
		if (assist.IsEmpty())
		{
			WoWGUID leader = pGame->GetGroup().leader;
			if (!leader.IsEmpty() && leader != pGame->GetID())
				assist = leader;
		}
		if (!assist.IsEmpty())
		{
			MonsterLevelCallback cb(m_capLevel);
			pGame->AssistPlayer(assist, &cb);
			if (pGame->IsTimerOut())
			{
				pGame->RefreshGameServer();
				return FALSE;
			}
		}
		else
			m_monster.Fight(pGame, -1, GetWoWTickCount());
			//pGame->EndGame(-1);
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
////
///////////////////////////////////////////////////////////////////////////////
HomeModule::HomeModule()
{
}
BOOL HomeModule::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (m_strVendor.GetLength() > 0 && pGame->GetSpell((LPCTSTR)m_strVendor))
	{
		pGame->Dismount();
		uint32 trycount = 0;
		if (pGame->CastSpell((LPCTSTR)m_strVendor, WoWGUID()))
		{
			trycount = 0;
			pGame->AutoEquip();
			while (!pGame->Repair() && trycount ++ < 3)
				Sleep(1000);//waiting for the npc come(packet delay???)
			pGame->Dismount();
			return TRUE;
		}
		else
		{
			pGame->Log(2, _T("cast summon vendor spell failed!"));
			return FALSE;
		}
	}
	else 
	{
		WoWPoint pos;
		uint32 map, area;
		uint32 index = -1;
		pGame->GetHomeInfo(pos, map, area);
		if (IsInBossRoom(pos))//the home position is correct, use it
		{
			pGame->GetPosition(&pos.x, &pos.y, &pos.z);
			if (IsInBossRoom(pos) || pGame->UseHomeStone())
			{
				if (!pGame->Repair() || pGame->Sell())
				{
					pGame->Log(2, _T("at home, cant sell/repair"));;
					return FALSE;
				}
				return TRUE;
			}
			else
			{
				pGame->Log(2, _T("Take home failed!"));
				return FALSE;
			}
		}
		else 
		{	
			pGame->GetPosition(&pos.x, &pos.y, &pos.z);
			uint32 index = IsAtPathLine(pos);
			if (index == -1)
			{
				float min = FLT_MAX;
				for (int i = 0; i < m_PathToBoss.size(); i++)
				{
					float dx = pos.x - m_PathToBoss[i].x;
					float dy = pos.y - m_PathToBoss[i].y;
					float dist = sqrt((dx*dx) + (dy*dy));
					if (dist < min)
					{
						min = dist;
						index = i;
					}
				}
			}
			if (index != -1)
			{
				if (index == m_PathToBoss.size())
					index = 0;
				for (uint32 i = index; i < m_PathToBoss.size(); index ++)
				{
					if (pGame->IsTimerOut() || pGame->IsGameEnd() || pGame->IsDead() != 0)
						return FALSE;
					WoWGUID attackme = pGame->RunTo(m_PathToBoss[index].x, m_PathToBoss[index].y, m_PathToBoss[index].z, m_PathToBoss[index].speed, 0.f);
					pGame->GetPosition(&pos.x, &pos.y, &pos.z);
					if (!attackme.IsEmpty())
						pGame->Attack(attackme);
				}
			}
			else if (m_BossRooms.size() > 0)
			{
				do 
				{
					if (pGame->IsTimerOut() || pGame->IsGameEnd() || pGame->IsDead() != 0)
						return FALSE;
					pGame->RunTo(m_BossRooms[0].x, m_BossRooms[0].y, m_BossRooms[0].z, 1.f, 0.f);
					pGame->GetPosition(&pos.x, &pos.y, &pos.z);
				}
				while (!IsInBossRoom(pos));

			}
			pGame->AutoEquip();
			if (!pGame->Repair() && !pGame->Sell())
			{
				pGame->Log(2, _T("at home, cant sell/repair"));
				return FALSE;
			}
			return TRUE;
		}
	}
	return FALSE;
}

MonsterBattleModule::MonsterBattleModule()
{
}
BOOL MonsterBattleModule::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	CMonsterExclude excludes;
	pGame->SetMonsterExclusive(&excludes);
	list<LPCTSTR> pFactions;
	for (list<CString>::iterator i = strFactions.begin(); i != strFactions.end(); i ++)
	{
		pFactions.push_back((LPCTSTR)(*i));
	}

	if (!pGame->KillClosestMonster(factions.size() == 0 ? NULL : &factions, pFactions.size() == 0 ? NULL : &pFactions, 500, minLevel, maxLevel))
	{
		pGame->Log(2, _T("not found monsters, start patrol!"));
		WoWPoint pos;
		pGame->GetPosition(&pos.x, &pos.y, &pos.z);
		uint32 index = IsAtPathLine(pos);
		if (index == -1)
		{	
			float min = FLT_MAX;
			for (int i = 0; i < m_PathToBoss.size(); i ++)
			{
				float dx = pos.x - m_PathToBoss[i].x;
				float dy = pos.y - m_PathToBoss[i].y;
				float dist = sqrt((dx*dx) + (dy*dy));
				if (dist < min)
				{
					min = dist;
					index = i;
				}
			}
		}
		if (index != -1)
		{
			if (index == m_PathToBoss.size())
				index = 0;
			WoWGUID attackme;
			for (uint32 i = index; i < m_PathToBoss.size();)
			{
				if ((attackme = pGame->PatrolTo(factions.size() == 0 ? NULL : &factions,
					pFactions.size() == 0 ? NULL : &pFactions,
					minLevel,
					maxLevel,
					m_PathToBoss[i].x,
					m_PathToBoss[i].y,
					m_PathToBoss[i].z)) != WoWGUID(1))
				{
					if (attackme != WoWGUID(0))
						pGame->Attack(attackme, 0.f);
					break;
				}
				if (++i == m_PathToBoss.size())
					i = 0;
			}
		}
		else
			Sleep(100);
	}
	pGame->SetMonsterExclusive(NULL);
	return TRUE;
}