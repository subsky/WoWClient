#pragma once
#include "Game.h"
#include "Markup.h"
#include <vector>

using namespace std;

class Glubtok : public FightModule
{
public:
	Glubtok();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x616D);
	};
};
class HelixGearbreaker : public FightModule
{
public:
	HelixGearbreaker();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x616E);
	};
};
class FoeReaper5000 : public FightModule
{
public:
	FoeReaper5000();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x6170);
	};
};
class AdmiralRipsnarlVanish : public FightModule
{
public:
	AdmiralRipsnarlVanish();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x616F);
	};
};
class CaptainCookie : public FightModule
{
public:
	CaptainCookie();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x485A);
	};
};

class CDeadmines : public CDungeonGame
{
public:
	CDeadmines(CConfig *config) : CDungeonGame(config) 
	{
		m_modules.push_back(new Glubtok());
		m_modules.push_back(new HelixGearbreaker());
		m_modules.push_back(new FoeReaper5000());
		m_modules.push_back(new AdmiralRipsnarlVanish());
		m_modules.push_back(new CaptainCookie());
	};
	~CDeadmines(void) {};

	virtual uint32 GetMapId() { return 0x24; };
	BOOL Prepare(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("Deadmines");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);
};

