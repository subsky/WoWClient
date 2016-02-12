#include "stdafx.h"
#include "LookingForGroup.h"
#include "ShadoPanMonastery.h"
#include "Mogu'shan Palace.h"
#include "Deadmines.h"
#include "ShadowfangKeep.h"


class CBuildDungeon : public CDungeonGame
{
public:
	CBuildDungeon(uint32 map, CConfig *config) : m_map(map), done(false), CDungeonGame(config)
	{
	};
	~CBuildDungeon(void) {};

	virtual uint32 GetMapId() { return m_map; };

	virtual LPCTSTR GetModuleName() { return _T("Build Dungeon"); };
	void Load(IBotConfig *config)
	{
	};
	virtual void OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg)
	{
		if (_tcscmp(strMsg, _T("done")) == 0)
		{
			done = true;
			pGame->StopAssist();
		}
	};
	BOOL Run(IGamePlay *pGame)
	{
		pGame->Lock();
		WoWGUID tank = pGame->GetWorkingGroup().GetTank();
		pGame->Unlock();
		pGame->AssistPlayer(tank);
		if (done)
			pGame->EndGame(-1);
		return TRUE;
	};
	uint32 m_map;
	bool done;
};


void LookingForGroup::OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg)
{
}
void LookingForGroup::Load(IBotConfig *config)
{
	TCHAR buf[512];
	m_lastMoveChar = false;
	if (GetPrivateProfileString(config->m_strSection, _T("LastMoveChar"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		if (_tcstol(buf, 0, 10) == 1)
			m_lastMoveChar = true;
	}
}
BOOL LookingForGroup::Run(IGamePlay *pGame)
{
	//if we are outside of instance, teleport to instace
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
	uint32 mapId = pGame->GetMapID();
	m_instanceMap = mapId;
	CDungeonGame *dungeon = NULL;
	if (mapId == 0x3BF)
	{
		m_mainMap = 0x366;
		dungeon = new CShadoPanMonastery(m_config);
	}
	else if (mapId == 0x3E2)
	{
		m_mainMap = 0x366;
		dungeon = new CMogushanPalace(m_config);
	}
	else if (mapId == 0x24)
	{
		//deadmines
		m_mainMap = 0x0;
		dungeon = new CDeadmines(m_config);
	}
	else if (mapId == 0x21)
	{
		//shadowfang keep
		m_mainMap = 0x0;
		dungeon = new CShadowfangKeep(m_config);
	}
	else
	{
		m_mainMap = 0x0;
		dungeon = new CBuildDungeon(mapId, m_config);
	}

	BOOL result = TRUE;
	if (pGame->GetMapID() == m_instanceMap)
	{
		if (dungeon)
			result = dungeon->Run(pGame);
	}
	if (dungeon)
		delete dungeon;
	/*if (pGame->IsScenarioComplete())
	{
		pGame->Lock();
		Group &lfg = pGame->GetGroup();
		uint32 count = lfg.members.size();
		pGame->Unlock();
		if (count == 5)
			pGame->
		/*Group &lfg = pGame->GetLFGroup();
		if (!lfg.leader.IsEmpty() && lfg.LfgSlot != -1)
			pGame->LeaveGroup();
	}*/
	//if (!result)
	//	pGame->RefreshGameServer();
	return (result);
}

