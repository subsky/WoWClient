#pragma once
#include "Game.h"

class CGuild : public CGame
{
public:
	CGuild(CConfig *config) : CGame(config) {};
	~CGuild(void) {};

	virtual LPCTSTR GetModuleName() {return _T("Guild");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);
	void CreateGuild(IGamePlay *pGame);
	void SignGuild(IGamePlay *pGame);
	void GuildCreateGoldHold(IGamePlay *pGame);

	void InvitePlayers(IGamePlay *pGame);
	int GenerateRandomName(CHAR *name);

	void OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg);
	void RunToVisitorRoom(IGamePlay *pGame);


	CString m_type;
	uint32 m_role;
	CString m_goldHolder;
	WoWGUID m_guildCreator;
};

