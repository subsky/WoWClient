#include "stdafx.h"
#include "Guild.h"
#include <vector>


void CGuild::Load(IBotConfig *config)
{
	m_role = 0;
	TCHAR buf[512];
	if (GetPrivateProfileString(config->m_strSection, _T("Role"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_role = _tcstol(buf, 0, 10);
	if (GetPrivateProfileString(config->m_strSection, _T("GoldHolder"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_goldHolder = buf;
}
void CGuild::OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg)
{
	if (_tcscmp(_T("GuildCreateMoney"), strMsg) == 0)
	{
		m_guildCreator = player;
		return;
	}
}

BOOL CGuild::Run(IGamePlay *pGame)
{
	if (m_role == 1)
		CreateGuild(pGame);
	else if (m_role == 2)
		SignGuild(pGame);
	else if (m_role == 3)
		GuildCreateGoldHold(pGame);
	else if (m_role == 4)
		InvitePlayers(pGame);
	//pGame->GuildInviteFinish();
	return FALSE;
}
int CGuild::GenerateRandomName(CHAR *name)
{
	srand(GetWoWTickCount());
	int len = rand() % 9;
	if (len < 3)
		len = 3;
	int i = 0;
	for (; i < len; i++)
	{
		name[i] = (i == 0 ? 'A' : 'a') + (rand() % 26);
	}
	name[i] = 0;
	return i;
}

void CGuild::CreateGuild(IGamePlay *pGame)
{
	pGame->SetStopTime(75000);
	RunToVisitorRoom(pGame);
	if (pGame->IsTimerOut())
	{
		pGame->RefreshGameServer();
		return;
	}
	bool startHold = false;
	while (pGame->GetCoinage() < 1000)//10 silver
	{
		pGame->TradeAcceptGold(1000);
		WoWGUID holder = pGame->FindPlayer(m_goldHolder, _T(""));
		if (!startHold)
		{
			startHold = true;
			pGame->StartBot(m_goldHolder, _T(""));
		}
		while (holder.IsEmpty())
		{
			Sleep(500);
			holder = pGame->FindPlayer(m_goldHolder, _T(""));
			if (pGame->IsTimerOut())
			{
				pGame->RefreshGameServer();
				return;
			}
		}
		WoWPoint pos(-8889.239258, 605.871826, 95.257996);
		if (!pGame->WaitUnitToPosition(holder, pos, 120000, 5.f))
		{
			pGame->Log(2, _T("gold holder is not ready"));
			return;
		}
		pGame->SendInternalMessage(holder, _T("GuildCreateMoney"), NULL);
		//if (pGame->Trade())
		Sleep(500);
	}
	uint8 bag, slot;
	if (pGame->GetInventoryItem(_T("Guild Charter"), bag, slot).IsEmpty())
	{
		list<LPCTSTR> msgs;
		msgs.push_back(_T("How do I form a guild?"));
		if (!pGame->GossipNpc(0x00040000/*UNIT_NPC_FLAG_PETITIONER*/, msgs, false))
		{
			pGame->Log(2, _T("gossip to npc failed"));
			return;
		}
		CHAR guildName[64] = { 0 };
		int len = GenerateRandomName(guildName);
		if (len == 0)
		{
			pGame->Log(2, _T("Guild creation:generating random name failed"));
			return;
		}
		pGame->BuyPetition(guildName);
	}
	WoWGUID signatures[4];
	int count = 0;
	while ((count = pGame->GetPetitionSignatures(signatures, 4)) < 4)
	{
		MemberInfo players[5];
		int c = pGame->GetGuildRequestList(players, 5);
		int i = 0;
		for (; i < c; i++)
		{
			int j = 0;
			for (; j < count; j++)
			{
				if (players[i].guid == signatures[j])
					break;
			}
			if (j == count)
				pGame->OfferPetition(players[i].guid);
		}
		Sleep(100);
	}
	if (pGame->TurnInPetition())
	{
		pGame->SetGuildRankPermission(4, "Initiate", 0x080043, 0x43, 100000);//max
		pGame->SetGuildRankPermission(3, "Member", 0x080043, 0x43, 100000);//max
	}
	pGame->GuildCreateFinish();
}
void CGuild::InvitePlayers(IGamePlay *pGame)
{
	pGame->SetStopTime(75000);
	MemberInfo players[5];
	int c = pGame->GetGuildRequestList(players, 5);
	/*while (c < 5)
	{
		Sleep(500);
		if (pGame->IsTimerOut())
		{
		pGame->RefreshGameServer();
		return;
		}
		c = pGame->GetGuildRequestList(players, 5);
	}*/
	CString inviter = CString(_T("GuildInviter:")) + pGame->GetPlayerName();
	int left = c;
	while (left)
	{
		for (int i = 0; i < c; i++)
		{
			if (!players[i].guid.IsEmpty())
			{
				pGame->SendInternalMessage(players[i].guid, inviter, NULL);
				if (pGame->GuildInvite(players[i].character));
				{
					left -= 1;
					players[i].guid.Clear();
				}
			}
		}
		if (pGame->IsTimerOut())
		{
			pGame->Log(2, _T("Guild inviter player error:time out!"));
			pGame->RefreshGameServer();
			return;
		}
	}
	//for two fast close this game, may some player didnt accept the message
	pGame->GuildInviteFinish();
}

void CGuild::SignGuild(IGamePlay *pGame)
{
	if (!pGame->HasGuild())
	{
		pGame->SetStopTime(75000);
		RunToVisitorRoom(pGame);
		if (pGame->IsTimerOut())
		{
			pGame->RefreshGameServer();
			return;
		}
		WoWGUID guid;
		while ((guid = pGame->RequestCreateGuild()).IsEmpty())
		{
			Sleep(500);
			if (pGame->IsTimerOut())
			{
				pGame->RefreshGameServer();
				return;
			}
		}
		pGame->SetPetitionProvider(guid);
		while (!pGame->HasGuild())
		{
			if (pGame->IsGameEnd())
				return;
			Sleep(500);
			if (pGame->IsTimerOut())
			{
				pGame->RefreshGameServer();
				return;
			}
		}
	}
	pGame->GuildCreateFinish();
}
void CGuild::GuildCreateGoldHold(IGamePlay *pGame)
{
	WoWPoint pos;
	pGame->GetPosition(&pos.x, &pos.y, &pos.z);
	if (fabs(pos.x + 8889.239258) > 1.f || fabs(pos.y - 605.871826) > 1.f)
		RunToVisitorRoom(pGame);
	while (m_guildCreator.IsEmpty())
	{
		Sleep(500);
	}
	if (!m_guildCreator.IsEmpty())
	{
		list<uint32> items;
		pGame->Trade(m_guildCreator, items, 1000);
	}
	pGame->EndGame(-1);
}
class GuildRunningCallback : public RunningCallback
{
	virtual bool Callback(float &x, float &y, float &z)
	{
		return false;
	}
};
void CGuild::RunToVisitorRoom(IGamePlay *pGame)
{
	WoWPoint pos;
	pGame->GetPosition(&pos.x, &pos.y, &pos.z);
	if (fabs(pos.x + 8889.239258) < 5.f && fabs(pos.y - 605.871826) < 5.f)
	{
		pGame->RunTo(-8889.239258, 605.871826, 95.257996, 0.f, 0.f);//human,stormwind
		return;
	}
	GuildRunningCallback gcb;
	while ((fabs(pos.x + 8884.777344) > 3.f && fabs(pos.x + 8886.202148) > 3.f) || (fabs(pos.y - 597.388855) > 3.f) && fabs(pos.y - 599.597656) > 3.f)
	{
		WoWGUID attackme = pGame->RunTo(-8884.777344, 597.388855, 94.174294, 1.f, false, &gcb);//human,stormwind
		if (pGame->IsDead())
			pGame->PickCorpse();
		if (pGame->IsTimerOut())
			return;
		if (pGame->IsGameEnd())
			return;
		pGame->GetPosition(&pos.x, &pos.y, &pos.z);
	}
	pGame->RunTo(-8889.239258, 605.871826, 95.257996, 0.f, 0.f);//human,stormwind
}
