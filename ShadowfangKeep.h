#pragma once
#include "Game.h"
#include "Markup.h"
#include <vector>

using namespace std;

class BaronAshbury : public FightModule
{
public:
	BaronAshbury();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x60C6);
	};
};
class BaronSilverlaine : public FightModule
{
public:
	BaronSilverlaine();
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x60CB);
	};
};
class CommanderSpringvale : public FightModule
{
public:
	CommanderSpringvale();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x60CC);
	};
};
class LordWalden : public FightModule
{
public:
	LordWalden();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x60C7);
	};
};
class LordGodfrey : public FightModule
{
public:
	LordGodfrey();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x485D);
	};
};

class CShadowfangKeep : public CDungeonGame
{
public:
	CShadowfangKeep(CConfig *config) : CDungeonGame(config) 
	{
		m_modules.push_back(new BaronAshbury());
		//m_modules.push_back(new BaronSilverlaine());
		//m_modules.push_back(new CommanderSpringvale());
		//m_modules.push_back(new LordWalden());
		//m_modules.push_back(new LordGodfrey());
	};
	~CShadowfangKeep(void) {};

	virtual uint32 GetMapId() { return 0x21; };

	virtual void DisableAreaTrigger(IGamePlay *pGame)
	{
		pGame->DisableAreaTrigger(0x966);
	};
	BOOL Prepare(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("Deadmines");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);

};

