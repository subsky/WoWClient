#pragma once
#include "Game.h"
#include "Markup.h"
#include <vector>

using namespace std;

class GuCloudstrike : public FightModule
{
public:
	GuCloudstrike();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x4B27);
	};
};

class MasterSnowdrift : public FightModule
{
public:
	MasterSnowdrift();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x4B2C);
	};
};
class ShaOfViolence : public FightModule
{
public:
	ShaOfViolence();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x4B28);
	};
};
class TaranZhu : public FightModule
{
public:
	TaranZhu();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x4B2B);
	};
};
class ExitShadoPan : public FightModule
{
public:
	ExitShadoPan();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x00);
	};
};


class CShadoPanMonastery : public CDungeonGame
{
public:
	CShadoPanMonastery(CConfig *config) : CDungeonGame(config) 
	{
		m_modules.push_back(new GuCloudstrike());
		m_modules.push_back(new MasterSnowdrift());
		m_modules.push_back(new ShaOfViolence());
		m_modules.push_back(new TaranZhu());
	};
	~CShadoPanMonastery(void) {};

	virtual uint32 GetMapId() { return 0x3BF; };
	BOOL Prepare(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("ShadoPanMonastery");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);

	enum ROLE
	{
		TANK = 0,
		HEALER = 1,
		DPS = 2
	};
	uint32 m_role;
};

