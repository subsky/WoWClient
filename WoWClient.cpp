#include "stdafx.h"
#include "WoWClient.h"
#include "Game.h"
#include "Config.h"
#include "LookingForGroup.h"
#include "PlayerInfo.h"


LPWoWDKClient EXPORT CreateClient(IBotConfig *pConfig, IGamePlay *pGame)
{
	return new CDefaultClient(pConfig, pGame);
}
void EXPORT ReleaseClient(IWoWDKClient *pClient)
{
	delete pClient;
}


CDefaultClient::CDefaultClient(IBotConfig *pConfig, IGamePlay *pGame) : IWoWDKClient(pConfig), m_pGame(pGame)
{
	m_pClient = m_config.Load(pConfig);
}
CDefaultClient::~CDefaultClient()
{
	if (m_pClient)
	{
		delete m_pClient;
		m_pClient = NULL;
	}
}

void CDefaultClient::OnGameCreate(IGamePlay *pGame)
{
}
void CDefaultClient::OnGameEnd()
{
	//m_games.clear();
}

void CDefaultClient::OnPlayerJoinGame(LPCTSTR user, int nClass, int nLevel)
{
}
void CDefaultClient::OnPlayerLeaveGame(LPCTSTR user)
{
}
void CDefaultClient::OnPlayerSlained(LPCTSTR user, LPCTSTR slainer)
{
	//if (stricmp(user, m_pBotConfig->m_strCharacter) == 0)
	//	m_pGame->EndGame();
}

void CDefaultClient::OnPlayerUpdate(LPCTSTR user, int life, int nMap)
{
}

void CDefaultClient::OnPortalOpened(DWORD id, LPCTSTR user, POINT position, int toMap)
{
}

bool CDefaultClient::CheckBag()
{
	uint32 bag = m_pGame->GetBagSpace();
	uint32 minDur = m_pGame->GetMinDurability();
	if ((bag >> 16) < 10 || minDur < 3)
		return true;
	return false;
}

void CDefaultClient::OnMessage(IGamePlay *pGame, int type, const WoWGUID &player, LPCTSTR strMsg)
{
	if(m_pClient)
		m_pClient->OnMessage(pGame, type, player, strMsg);

	if (type == INTERNAL_MESSAGE_WHISPER && _tcsncmp(strMsg, _T("GuildInviter:"), _tcslen(_T("GuildInviter:"))) == 0)
		m_pGame->SetGuildInviter(strMsg + _tcslen(_T("GuildInviter:")));

	if (type == CHAT_MSG_WHISPER)
	{
		return;
	}
	if (type != CHAT_MSG_PARTY && type != CHAT_MSG_PARTY_LEADER && type != INTERNAL_MESSAGE_LEADER && type != INTERNAL_MESSAGE_GROUP && type != INTERNAL_MESSAGE_WHISPER)//Party message or PartyLeader message
		return;
	m_pGame->Lock();
	Group &group = m_pGame->GetGroup();
	m_pGame->Unlock();
	if (type == CHAT_MSG_PARTY_LEADER || type == INTERNAL_MESSAGE_LEADER)//from leader
	{
		m_command = strMsg;
		//L leader, M member
		if (m_command.Compare(_T("LStatus")) == 0 || m_command.Compare(_T("LRepair")) == 0 || m_command.Compare(_T("LReady")) == 0)
			m_event.SetEvent();
	}
	if (m_pGame->GetID() == group.leader)//i am the leader, and the message is from other players
	{
		if (strMsg && strMsg[0] != _T('M'))
			return;
		if (_tcscmp(_T("MReady"), strMsg) != 0 && _tcscmp(_T("MRepair"), strMsg) != 0)
			return;
		m_pGame->Lock();
		m_response[player] = CString(strMsg);
		uint32 ready = 0;
		uint32 repair = 0;
		if (m_response.size() == group.members.size())
		{
			for (map<WoWGUID, CString>::iterator i = m_response.begin(); i != m_response.end(); i ++)
			{
				if (i->second.Compare(_T("MReady")) == 0)
					ready ++;
				else if (i->second.Compare(_T("MRepair")) == 0)
					repair ++;
			}
			if (repair > 0 || ready == group.members.size())
			{
				//for (list<Member>::iterator i = group.members.begin(); i != group.members.end(); i ++)
					//m_pGame->ChatMessageParty(repair > 0 ? _T("LRepair") : _T("LReady"));
				m_pGame->SendInternalMessage(WoWGUID(), repair > 0 ? _T("LRepair") : _T("LReady"), NULL);
			}
		}
		m_pGame->Unlock();
	}
}
void CDefaultClient::OnInternalMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg, LPVOID data)
{
	OnMessage(pGame, nType, player, strMsg);
}

void CDefaultClient::CheckStatus()
{
	bool ot = false;
	m_response.clear();
	m_command.Empty();
	Group &group = m_pGame->GetGroup();
	while (1)
	{
		if (m_pGame->IsGameEnd())
			return;
		if (group.leader == m_pGame->GetID() && !ot)
		{
			m_pGame->Lock();
			m_response.clear();
			//for (list<Member>::iterator i = group.members.begin(); i != group.members.end(); i ++)
			{
				//m_pGame->ChatMessageParty(_T("LStatus"));
				m_pGame->SendInternalMessage(WoWGUID(), _T("LStatus"), NULL);
			}
			m_pGame->Unlock();
		}
		if (!m_pGame->GetLFGroup().id.IsEmpty() && m_pGame->GetLFGroup().LfgFlag != 2 && m_pGame->GetLFGState() != 25)//flag should be 0, it's not completed
			return;
		//if (!m_pGame->IsScenarioComplete())
			//m_pGame->ChatMessageParty(_T("MAmInLFG"));
			//m_pGame->SendInternalMessage(WoWGUID(), _T("MAmInLFG"), NULL);
		if (ot = m_event.Lock(5000))
		{
			if (m_command.Compare(_T("LStatus")) == 0 && m_pGame->IsGameReady())
			{
				if (CheckBag())
					//m_pGame->ChatMessageParty(_T("MRepair"));
					m_pGame->SendInternalMessage(group.leader, _T("MRepair"), NULL);
				else if (m_pGame->IsDead() != 2)//in ghost
					//m_pGame->ChatMessageParty(_T("MReady"));
					m_pGame->SendInternalMessage(group.leader, _T("MReady"), NULL);
			}
			else if (m_command.Compare(_T("LRepair")) == 0)
			{
				m_pGame->Log(2, _T("going to repair item now"));
				if (m_pGame->IsInDungeon())
				{
					if (m_pGame->GetLFGroup().LfgSlot != -1)
					{
						if (m_pGame->LFGTeleport(true) != 0)
							m_pGame->Log(2, _T("lfg teleport out failed"));
					}
					else
					{
						if (m_pGame->ChangeArea())
							m_pGame->Log(2, _T("changing area failed"));
					}
				}
				if (!m_pGame->Repair())
					m_pGame->Log(2, _T("repair failed"));
			}
			else if (m_command.Compare(_T("LReady")) == 0)
			{
				return;
			}
		}
	}

}

void CDefaultClient::CheckGroup(Group &group, list<LPCTSTR> &kickoff, list<LPCTSTR> &offline, list<LPCTSTR> &death, list<ConfigMember> &invite)
{
	invite = m_config.m_groupMembers;
	
	for (list<Member>::iterator i = group.members.begin(); i != group.members.end(); i ++)
	{
		bool found = i->guid == m_pGame->GetID();
		for (list<ConfigMember>::iterator j = invite.begin(); j != invite.end(); j ++)
		{
			if (_tcscmp(i->name, j->name) == 0)
			{
				invite.erase(j);
				found = true;
				break;
			}
		}
		if (!found)
		{
			kickoff.push_back(i->name);
			continue;
		}
		if ((i->status & MEMBER_STATUS_ONLINE))//mean offline
			offline.push_back(i->name);
		else if ((i->status & MEMBER_STATUS_DEAD) || (i->status & MEMBER_STATUS_GHOST))//mean offline
			death.push_back(i->name);
	}
}

DWORD CDefaultClient::Run()
{
	while (1)
	{
		if (m_pGame->IsGameEnd())
		{
			//m_pGame->RefreshGameServer();
			return 0;
		}
		if (m_config.m_needGuild && (!m_pGame->HasGuild() || m_pGame->HasCompleteChallenge()))
		{
			if (m_pGame->HasGuild())
				m_pGame->QuitGuild();
			int newGuild = 0;
			while ((newGuild = m_pGame->RequestNewGuild()) != 0)
			{
				if (newGuild == 2)
				{
					m_pGame->Log(1, _T("Error: not found guild creator/not available!"));
					break;
				}
				m_pGame->Log(2, _T("info: may other realm visiting the guild creator? wait some seconds!"));
				Sleep(500);
			}
			while (newGuild == 0 && !m_pGame->HasGuild())
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

		Group &group = m_pGame->GetGroup();
		Group &lfg = m_pGame->GetLFGroup();
		if (m_config.m_role)
			m_pGame->SetRole(m_config.m_role);
		else
		{
			uint32 spec = m_pGame->GetCurrentSpec();
			uint32 role = ClassSpec::GetRoleBySpec(spec);
			if (m_config.m_groupLeader.Compare(m_pGame->GetPlayerName()) == 0)
				role += 1;
			m_pGame->SetRole(role);

		}
		m_pGame->Accept(m_config.m_groupLeader);
		if (m_config.m_groupHelper.GetLength() > 0)
			m_pGame->Accept(m_config.m_groupHelper);

		if (m_config.m_groupLeader.GetLength() > 0 || m_config.m_groupHelper.GetLength() > 0)
		{
			if (!lfg.id.IsEmpty())
			{
				m_pGame->Lock();
				WoWGUID leader1 = lfg.GetMember(m_config.m_groupLeader);
				WoWGUID leader2 = lfg.GetMember(m_config.m_groupHelper);
				m_pGame->Unlock();
				if (leader1 != lfg.leader && leader2 != lfg.leader)
				{
					m_pGame->Log(2, _T("LFG leader is not same with defined in config.ini"));
					m_pGame->LeaveGroup();
					m_pGame->LeaveGroup();
				}
			}
			if (!group.id.IsEmpty())
			{
				m_pGame->Lock();
				WoWGUID leader1 = group.GetMember(m_config.m_groupLeader);
				WoWGUID leader2 = group.GetMember(m_config.m_groupHelper);
				m_pGame->Unlock();
				if (leader1 != group.leader && leader2 != group.leader)
				{
					m_pGame->Log(2, _T("group leader is not same with defined in config.ini"));
					m_pGame->LeaveGroup();
				}
			}
		}
		if (m_pGame->IsDead() != 0 && !m_pGame->PickCorpse())
		{
			m_pGame->Log(2, _T("pickup corpse failed!"));
			m_pGame->RefreshGameServer();
			return FALSE;
		}
		if (m_config.m_groupLeader.Compare(m_pGame->GetPlayerName()) == 0 && m_config.m_groupHelper.GetLength() > 0)
		{
			if (group.leader.IsEmpty() && lfg.leader.IsEmpty())
				//request the helper online to build the group
				while (m_pGame->RequestGroupHelper(m_config.m_groupHelper) != 0)
					Sleep(100);
		}

		if (m_config.m_groupLeader.Compare(m_pGame->GetPlayerName()) == 0 || m_config.m_groupHelper.Compare(m_pGame->GetPlayerName()) == 0)//me is group leader, check and invite
		{
			list<LPCTSTR> kickoffplayers;
			list<ConfigMember> toInvite;
			list<LPCTSTR> offline;
			list<LPCTSTR> death;
			while (1)
			{
				if (m_pGame->IsGameEnd())
					return 0;
				if (!group.leader.IsEmpty() && m_pGame->GetID() != group.leader && m_config.m_groupHelper.GetLength() > 0)
				{
					//in this case i'm need group helper to build the group, then wait him pass the leader to me
					Sleep(200);
					continue;
				}
				kickoffplayers.clear();
				toInvite.clear();
				offline.clear();
				death.clear();
				if (group.grouptype & GROUPTYPE_RAID && group.leader == m_pGame->GetID() && m_config.m_groupHelper.Compare(m_pGame->GetPlayerName()) != 0)
					m_pGame->ConvertGroupToRaid(false);
				m_pGame->Lock();
				CheckGroup(!lfg.id.IsEmpty() ? lfg : group, kickoffplayers, offline, death, toInvite);
				int membersize = group.members.size();
				m_pGame->Unlock();
				for (list<LPCTSTR>::iterator i = kickoffplayers.begin(); i != kickoffplayers.end(); i++)
					m_pGame->Uninvite(*i);
				if (toInvite.size() > 0)
				{
					m_pGame->Log(2, _T("Waiting some player to join the group: %s"), (LPCTSTR)toInvite.begin()->name);
					if ((membersize + toInvite.size() > 5) && group.grouptype == GROUPTYPE_NORMAL && !group.id.IsEmpty())
						m_pGame->ConvertGroupToRaid(true);
				}
				for (list<ConfigMember>::iterator i = toInvite.begin(); i != toInvite.end(); i++)
					m_pGame->Invite(i->name, i->realm);
				if (kickoffplayers.size() == 0 && toInvite.size() == 0)
				{
					if ((lfg.leader.IsEmpty() && lfg.LfgSlot == -1) || m_pGame->IsScenarioComplete())
					{
						if (offline.size() > 0)
						{
							m_pGame->Log(2, _T("Waiting for player online %s"), *(offline.begin()));
							Sleep(200);
							continue;
						}
						else if (death.size() > 0)
						{
							m_pGame->Log(2, _T("Waiting for player pickup corpse %s"), *(death.begin()));
							Sleep(200);
							continue;
						}
					}
					break;
				}
				Sleep(200);
			}
		}
		if (m_config.m_groupLeader.GetLength() > 0)
		{
			uint32 lastMsgTicks = 0;
			while (group.id.IsEmpty() && lfg.id.IsEmpty())
			{
				if (m_pGame->IsGameEnd())
					return FALSE;
				Sleep(200);
				m_pGame->Log(2, _T("Waiting for %s invite to group"), (LPCTSTR)m_config.m_groupLeader);
			}
			//waiting till the help quit and pass leadship to our real leader
			while (1)
			{
				WoWGUID leader = group.GetMember(m_config.m_groupLeader);
				if (leader == group.leader)
					break;
				if (m_pGame->IsGameEnd())
					return FALSE;
				Sleep(200);
			}
		}
		//
		if (m_config.m_groupHelper.Compare(m_pGame->GetPlayerName()) == 0)
		{
			//pass leader to the right one, the first one in the list
			if (m_config.m_groupMembers.size() > 0)
			{
				m_pGame->SetLeader(m_config.m_groupMembers.begin()->name);
			}
			m_pGame->LeaveGroup();
		}



		//if we want lfg, just do it
		LookingForGroup *lfgClient = dynamic_cast<LookingForGroup *>(m_pClient);
		if (lfgClient && m_config.m_lfgDungeon.size() > 0)
		{
			list<LPCTSTR> slots;
			for (list<CString>::iterator i = m_config.m_lfgDungeon.begin(); i != m_config.m_lfgDungeon.end(); i++)
				slots.push_back((LPCTSTR)(*i));
			if ((lfg.id.IsEmpty() && lfg.LfgSlot == -1) || (lfg.LfgFlag == 2))
			{
				CheckStatus();
				if (m_config.m_groupLeader.Compare(m_pGame->GetPlayerName()) == 0 || group.id.IsEmpty())
					m_pGame->LookingForGroup(false, slots, m_config.m_lfgDifficulty);
				else
					m_pGame->LookingForGroup(true, slots, m_config.m_lfgDifficulty);
			}
		}

		if (!m_pClient) //group helper didnt have any module instance
		{
			m_pGame->EndGame(-1);
			return -1;
		}
		if (m_pClient->Run(m_pGame))
		{
			/*uint32 logout = m_pGame->Logout();
			if (logout != 0)
			{
				m_pGame->Log(2, _T("Logout with result %x"), logout);
				m_pGame->EndGame(10);
			}*/
			//m_pGame->RefreshGameServer();
		}
	}
	return 0;
}

void CDefaultClient::Reset(bool changingarea)
{
	if (!changingarea)
	{
	}
}
