#pragma once
#include "Game.h"
#include "Markup.h"

class HomeModule : public FightModule
{
public:
	HomeModule();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	CString m_strVendor;
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x00);
	};
};
class MonsterBattleModule : public FightModule
{
public:
	MonsterBattleModule();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x00);
	};
	list<uint32> factions;
	list<CString> strFactions;
	uint32 minLevel;
	uint32 maxLevel;
	map<WoWGUID, uint32> exclues;
};
class CMonsterLevelup : public CGame
{
public:
	CMonsterLevelup(CConfig *config) : CGame(config) 
	{
		m_selling = false;
	};
	~CMonsterLevelup(void) {};

	void OnMessage(IGamePlay *pGame, int type, const WoWGUID &player, LPCTSTR strMsg);
	BOOL Fight(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("MonsterLevelup");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);
	void Clear();

	CString m_xmlfile;
	CString m_friend;
	CString m_assist;
	CMarkup m_xml;
	uint32 m_capLevel;
	uint32 m_desiredSpec;
	bool m_selling;

	HomeModule m_home;
	MonsterBattleModule m_monster;
};

