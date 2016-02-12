#pragma once

#include <list>
#include <map>
#include <string>
#include "Game.h"

using namespace std;

struct ConfigMember
{
	CString name;
	CString realm;
};

class CConfig  
{
public:
	CConfig();
	virtual ~CConfig();

	CGame *Load(IBotConfig *config);

	list<ConfigMember> m_groupMembers;
	CString m_groupLeader;
	CString m_groupHelper;
	uint32 m_role;

	uint32 m_lfgDifficulty;
	list<CString> m_lfgDungeon;
	bool m_needGuild;

	map<string, CGame *> m_instances;

	map<DWORD, string> m_bosses;

};
