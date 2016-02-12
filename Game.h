#pragma once

#include "IGamePlay.h"
#include "IWoWDKClient.h"
#include "FightModule.h"
#include <list>

using namespace std;

class CConfig;
class CGame  
{
public:
	void OnGameClose() {m_bClose = TRUE;};

	virtual uint32 GetMapId() { return 0; };
	virtual BOOL Prepare(IGamePlay *pGame);
	CGame(CConfig *config);
	virtual ~CGame();

	virtual void Load(IBotConfig *config);

	virtual LPCTSTR GetModuleName() {return _T("Default");};
	virtual void OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg)
	{
	};

	virtual BOOL Run(IGamePlay *pGame);
protected:
	BOOL m_bClose;
	CConfig *m_config;
	CString m_strVendor;
};
class CDungeonGame : public CGame
{
public:
	CDungeonGame(CConfig *config) : CGame(config) {};
	~CDungeonGame() 
	{
		for (list<FightModule *>::iterator i = m_modules.begin(); i != m_modules.end(); i++)
			delete *i;
	};
	virtual BOOL Run(IGamePlay *pGame);
	virtual void DisableAreaTrigger(IGamePlay *pGame) {};
protected:
	list<FightModule *> m_modules;

};

class CFishing : public CGame
{
public:
	CFishing(CConfig *config) : CGame(config) {};
	virtual ~CFishing() {};

	virtual LPCTSTR GetModuleName() {return _T("Fishing");};

	virtual BOOL Run(IGamePlay *pGame);
};

class CGroup : public CGame
{
public:
	CGroup(CConfig *config) : CGame(config) {};
	virtual LPCTSTR GetModuleName() { return _T("Group"); };

	void CheckGroup(IGamePlay *pGame);

	virtual void Load(IBotConfig *config);

	CString m_strMember;
};
class CResetInstance : public CGroup
{
public:
	CResetInstance(CConfig *config) : CGroup(config) {};

	virtual ~CResetInstance() {};

	virtual LPCTSTR GetModuleName() {return _T("Reset");};

	virtual BOOL Run(IGamePlay *pGame);
};
class _CSTSM : public CGroup
{
public:
	_CSTSM(CConfig *config) : CGroup(config) {};
	virtual ~_CSTSM() {};

	bool ClearBag(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("_STSM");};

	virtual BOOL Run(IGamePlay *pGame);

	BOOL GetOutOfSTSM(IGamePlay *pGame);

	BOOL KillBalnazzar(IGamePlay *pGame, BOOL resetwait);
	BOOL KillBaronRivendare(IGamePlay *pGame, BOOL resetwait);

private:
	//uint32 m_badreset;
};

class CBalnazzar : public _CSTSM
{
public:
	CBalnazzar(CConfig *config) : _CSTSM(config) {};
	virtual ~CBalnazzar() {};

	virtual LPCTSTR GetModuleName() {return _T("Balnazzar");};

	virtual BOOL Run(IGamePlay *pGame);
};

class CSTSM : public _CSTSM
{
public:
	CSTSM(CConfig *config) : _CSTSM(config) {};
	virtual ~CSTSM() {};

	virtual LPCTSTR GetModuleName() {return _T("STSM");};

	virtual BOOL Run(IGamePlay *pGame);

};

class CDireMaul : public CGroup
{
public:
	CDireMaul(CConfig *config) : CGroup(config) {};
	virtual ~CDireMaul() {};

	bool ClearBag(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("DireMaul");};

	virtual BOOL Run(IGamePlay *pGame);

	BOOL GetOutOfDireMaul(IGamePlay *pGame);
};

class CRamparts : public CGroup
{
public:
	CRamparts(CConfig *config) : CGroup(config) {};
	virtual ~CRamparts() {};

	bool ClearBag(IGamePlay *pGame);

	virtual LPCTSTR GetModuleName() {return _T("Ramparts");};

	virtual BOOL Run(IGamePlay *pGame);

	BOOL GetOutOfRamparts(IGamePlay *pGame);
};


class CDupe : public CGame
{
public:
	CDupe(CConfig *config) : CGame(config) {};
	virtual ~CDupe() {};

	virtual LPCTSTR GetModuleName() {return _T("Dupe");};
	virtual void OnMessage(IGamePlay *pGame, int nType, const WoWGUID &player, LPCTSTR strMsg);

	virtual BOOL Run(IGamePlay *pGame);

	virtual void Load(IBotConfig *config);

	CString m_receiver;
	CString m_item;
	CString m_character;
	CString m_seller;
	CString m_crystallizedItem;
};


class CTest : public CGame
{
public:
	CTest(CConfig *config) : CGame(config) {};
	virtual ~CTest() {};

	virtual LPCTSTR GetModuleName() {return _T("Test");};

	virtual BOOL Run(IGamePlay *pGame);
};

class CMonsterExclude : public MonsterExcludeCallback
{
public:
	virtual bool Callback(IGamePlay *pGame, const WoWGUID &monster, bool loot)
	{
		return !loot && m_excludes.find(monster) != m_excludes.end();
	}
	virtual void UpdateFailed(const WoWGUID &monster)
	{
		m_excludes[monster] = 0;
	}

	map<WoWGUID, uint32> m_excludes;
};