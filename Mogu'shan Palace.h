#pragma once
#include "Game.h"
#include "Markup.h"
#include <vector>

using namespace std;

class TrialoftheKing : public FightModule
{
public:
	TrialoftheKing();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x60D0);
	};
};

class Gekkan : public FightModule
{
public:
	Gekkan();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x4B38);
	};
};
class Xin : public FightModule
{
public:
	Xin();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x4B39);
	};
};

class ExitMogushan : public FightModule
{
public:
	ExitMogushan();
	BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return pGame->IsScenarioEventComplete(0x00);
	};
};


class CMogushanPalace : public CDungeonGame
{
public:
	CMogushanPalace(CConfig *config) : CDungeonGame(config) 
	{
		m_modules.push_back(new TrialoftheKing());
		m_modules.push_back(new Gekkan());
		m_modules.push_back(new Xin());
	};
	~CMogushanPalace(void) {};

	virtual uint32 GetMapId() { return 0x3E2; };
	BOOL Prepare(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("MogushanPalace");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);

};

