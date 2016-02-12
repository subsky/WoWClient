#pragma once

#include "IWoWDKClient.h"
#include <list>
#include "Game.h"
#include "Config.h"
#include <afxmt.h>

#ifdef EXPORT
#undef EXPORT
#endif
#define	EXPORT	__declspec( dllexport ) __cdecl

using namespace std;

class CDefaultClient : public IWoWDKClient
{
public:
	CDefaultClient(IBotConfig *pConfig, IGamePlay *pGame);
	virtual ~CDefaultClient();
	virtual void OnGameCreate(IGamePlay *pGame);
	virtual void OnGameEnd();
	virtual void OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg);
	virtual void OnPlayerJoinGame(LPCTSTR user, int nClass, int nLevel);
	virtual void OnPlayerLeaveGame(LPCTSTR user);
	virtual void OnPlayerSlained(LPCTSTR user, LPCTSTR slainer);
	virtual void OnPlayerUpdate(LPCTSTR user, int life, int nMap);
	virtual void OnPortalOpened(DWORD id, LPCTSTR user, POINT position, int toMap);
	virtual DWORD Run();
	virtual void Reset(bool changingarea);
	virtual void OnInternalMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg, LPVOID data);

	virtual void CheckStatus();
	bool CheckBag();

private:
	void CheckGroup(Group &group, list<LPCTSTR> &kickoff, list<LPCTSTR> &offline, list<LPCTSTR> &death, list<ConfigMember> &invite);

private:
	IGamePlay *m_pGame;
	CConfig m_config;
	//list<CGame *> m_games;
	CGame *m_pClient;

	CEvent m_event;
	CString m_command;

	map<WoWGUID, CString> m_response;//used for leader
};


extern "C"
{
LPWoWDKClient EXPORT CreateClient(IBotConfig *pConfig, IGamePlay *pGame);
void EXPORT ReleaseClient(IWoWDKClient *pClient);
}
