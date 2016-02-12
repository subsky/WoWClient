// Config.cpp: implementation of the CConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Config.h"
#include <string>
#include "MonsterLevel.h"
#include "ShadoPanMonastery.h"
#include "Mogu'shan Palace.h"
#include "LookingForGroup.h"
#include "Guild.h"
#include "GuildMoney.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfig::CConfig()
{
}

CConfig::~CConfig()
{

}

ConfigMember GetConfigMember(LPCTSTR strMember)
{
	int index = 0;
	TCHAR c = 0;
	bool bname = true;
	ConfigMember cm;
	while (c = strMember[index ++])
	{
		if (c == _T('@'))
		{
			bname = false;
			continue;
		}
		if (bname)
			cm.name += c;
		else
			cm.realm += c;
	}
	return cm;
}

CGame *CConfig::Load(IBotConfig *config)
{
	CGame *result = NULL;
	TCHAR buf[512];
	if (GetPrivateProfileString(config->m_strSection, _T("Module"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		if (_tcscmp(_T("MonsterLevelup"), buf) == 0)
			result = new CMonsterLevelup(this);
		else if (_tcscmp(_T("Guild"), buf) == 0)
			result = new CGuild(this);
		else if (_tcscmp(_T("GuildMoney"), buf) == 0)
			result = new GuildMoney(this);
		else if (_tcscmp(_T("Fishing"), buf) == 0)
			result = new CFishing(this);
		else if (_tcscmp(_T("Test"), buf) == 0)
			result = new CTest(this);
		else if (_tcscmp(_T("ShadoPanMonastery"), buf) == 0)
			result = new CShadoPanMonastery(this);
		else if (_tcscmp(_T("MogushanPalace"), buf) == 0)
			result = new CMogushanPalace(this);
		else if (_tcscmp(_T("LookingForGroup"), buf) == 0)
			result = new LookingForGroup(this);
	}
	if (result)
	{
		result->Load(config);
	}
	m_role = 0;//damage
	if (GetPrivateProfileString(config->m_strSection, _T("Role"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_role = _tcstol(buf, 0, 10);
	m_lfgDifficulty = 1;//LFG_TYPE_DUNGEON
	if (GetPrivateProfileString(config->m_strSection, _T("LFGDifficulty"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_lfgDifficulty = _tcstol(buf, 0, 10);
	if (GetPrivateProfileString(config->m_strSection, _T("Param"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		LPTSTR dungeon = buf;
		LPCTSTR str_dungeon = dungeon;
		while (dungeon[0])
		{
			LPTSTR next = 0;
			if (dungeon[0] != _T(','))
				dungeon ++;
			else
			{
				dungeon[0] = 0;
				dungeon ++;			
				m_lfgDungeon.push_back(str_dungeon);
				str_dungeon = dungeon;
			}
		}
		if (str_dungeon && str_dungeon[0])
			m_lfgDungeon.push_back(str_dungeon);
	}
	if (GetPrivateProfileString(config->m_strSection, _T("GroupMember"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		LPTSTR member = buf;
		LPCTSTR strMember = member;
		while (member[0])
		{
			LPTSTR next = 0;
			if (member[0] != _T(','))
				member ++;
			else
			{
				member[0] = 0;
				member ++;			
				m_groupMembers.push_back(GetConfigMember(strMember));
				strMember = member;
			}
		}
		if (strMember[0])
			m_groupMembers.push_back(GetConfigMember(strMember));
	}
	if (GetPrivateProfileString(config->m_strSection, _T("GroupLeader"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_groupLeader = buf;
	if (GetPrivateProfileString(config->m_strSection, _T("GroupHelper"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_groupHelper = buf;
	m_needGuild = false;
	if (GetPrivateProfileString(config->m_strSection, _T("NeedGuild"), NULL, buf, sizeof(buf), config->m_strIniFile))
	{
		if (buf[0] == _T('1') || _tcsicmp(buf, _T("true")))
			m_needGuild = true;
	}
	return result;
}

/*
void CConfig::Load(list<CGame *> &games)
{
	TCHAR config[512];
	LPCTSTR strIniFile = _T("conf\\Game.ini");
	int nLen = GetPrivateProfileSection(_T("Game"), config, sizeof(config), strIniFile);
	TCHAR *p1 = config;
	while(p1 - config < nLen && *p1)
	{
		TCHAR *p2 = p1;

		while(*p2 != _T('=')) p2++;
		*p2 = 0;
		p2 ++;

		if(_tcslen(p2) == 0 || *p2 == _T(';'))
		{
		}
		if (_tcsicmp(_T("TRUE"), p2) == 0)// || _tcstol(p2, NULL, 10) == 1)
		{
			_tcslwr(p1);
			//games.push_back(m_instances[p1]);
		}		
		p1 = p2;
		while(p1 - config <nLen && *p1) p1++;
		p1++;
	}
}

void CConfig::Load(list<CGame *> &games, list<string> *pModules)
{
	for (list<string>::iterator i = pModules->begin(); i != pModules->end(); i ++)
	{
		map<string, CGame *>::iterator module = m_instances.find(*i);
		if (module != m_instances.end())
			games.push_back(module->second);
	}
}
*/