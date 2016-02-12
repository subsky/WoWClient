#pragma once
#include "Game.h"


class GuildMoney : public CGame
{
public:
	GuildMoney(CConfig *config) : CGame(config) { debug = false; };
	~GuildMoney(void) {};

	virtual LPCTSTR GetModuleName() {return _T("GuildMoney");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);

	void OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg);

	bool debug;

};

