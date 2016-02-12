#pragma once
#include "Game.h"


class LookingForGroup : public CGame
{
public:
	LookingForGroup(CConfig *config) : CGame(config) {};
	~LookingForGroup(void) {};

	virtual LPCTSTR GetModuleName() {return _T("LookingForGroup");};
	void Load(IBotConfig *config);
	BOOL Run(IGamePlay *pGame);

	void OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg);

	uint32 m_mainMap;
	uint32 m_instanceMap;
	bool m_lastMoveChar;

};

