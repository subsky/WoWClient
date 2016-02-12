#include "stdafx.h"
#include "ShadoPanMonastery.h"
#include <vector>
#include "Opcodes.h"


GuCloudstrike::GuCloudstrike()
{
	m_BossRooms.push_back(BossRoom(3861.51, 2615.981, 754.5428, 30.f));
	m_PathToBoss.push_back(PathStep(3657.290039, 2551.919922, 766.968628, 1.f));//1.f
	m_PathToBoss.push_back(PathStep(3621.f, 2624.f, 766.966003, 0.f));//1.f
	m_PathToBoss.push_back(PathStep(3660.f, 2647.f, 770.041748, 0.f));//0.f
	m_PathToBoss.push_back(PathStep(3695.f, 2647.f, 770.041748, 0.f));//0.f change area
	m_PathToBoss.push_back(PathStep(3697.724854, 2661.628418, 770.041748, 0.f));//0.f
}
BOOL __fastcall StopAttack(IGamePlay *pGame, const WoWGUID &monster)
{
	if (pGame->HasAura(monster, _T("Charging Soul")))
		return TRUE;
	return FALSE;
}
BOOL GuCloudstrike::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->Log(2, _T("Boss Gu Cloudstrike Started!"));
	//pGame->RunTo
	if (step != 0)
	{
		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
		{
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
			if (i == 3)
			{
				ByteBuffer packet;
				packet << (uint32)0x1DE8;
				packet.WriteBit(1);//leave?
				packet.WriteBit(1);
				packet.FlushBits();
				pGame->SendPacket(CMSG_AREA_TRIGGER, packet);
			}
		}
	}
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
		return TRUE;
	WoWGUID GuCloudstrikeGuid;
	uint32 start = GetWoWTickCount();
	while(GetWoWTickCount() - start < 5000)
	{
		GuCloudstrikeGuid = pGame->GetObject(_T("Gu Cloudstrike"), true);
		if (!GuCloudstrikeGuid)
			Sleep(100);
		else
			break;
	}
	if (GuCloudstrikeGuid.IsEmpty())
	{
		pGame->Log(2, _T("not found boss Gu Cloudstrike"));
		return TRUE;
	}
	while(!(GuCloudstrikeGuid = pGame->GetObject(_T("Gu Cloudstrike"), true)).IsEmpty() && pGame->GetUnitHealth(GuCloudstrikeGuid) > 0)//unit disappeared
	{
		if (pGame->IsGameEnd())
			return FALSE;
		pGame->Attack(GuCloudstrikeGuid, 0.f, StopAttack);
		WoWGUID Azure;
		if (pGame->HasAura(GuCloudstrikeGuid, _T("Charging Soul")))
		{
			while(1)
			{
				Azure = pGame->GetObject(_T("Azure Serpent"), true);
				if (!Azure)
					Sleep(100);
				else
					break;
			}
			pGame->RunTo(3727.666, 2688.185, 768.0416, 0.f, 0.f);
			pGame->Attack(Azure, 45.f);
			pGame->Log(2, _T("Azure Serpent job finished!"));
		}
		pGame->Attack(GuCloudstrikeGuid, 0.f, StopAttack);
	}
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
	return TRUE;
}
BOOL __fastcall MasterSnowdriftCallback(IGamePlay *pGame, const WoWGUID &monster)
{
	float x, y, z;
	pGame->GetPosition(&x, &y, &z);
	if (z > 820.f)
	{
		pGame->Log(2, _T("Just after change transport, ok, let get the right height!"));
		const WoWPoint boss2pos(3717.1399, 3096.4500, 817.31897);
		const WoWPoint room2pos(3695.023, 3065.702, 816.201);
		float boss_dx = boss2pos.x - x;
		float boss_dy = boss2pos.y - y;
		float room_dx = room2pos.x - x;
		float room_dy = room2pos.y - y;
		if ((boss_dx * boss_dx + boss_dy * boss_dy) < (room_dx * room_dx + room_dy * room_dy))//we finished the encount
			pGame->RunTo(room2pos.x, room2pos.y, room2pos.z, 0.f, 0.f);
		else
			pGame->RunTo(boss2pos.x, boss2pos.y, boss2pos.z, 0.f, 0.f);
	}
	return FALSE;
}
MasterSnowdrift::MasterSnowdrift()
{
	m_BossRooms.push_back(BossRoom(3658.823, 3015.969, 804.6611, 40.f));
	m_BossRooms.push_back(BossRoom(3695.023, 3065.702, 816.201, 40.f));

	m_PathToBoss.push_back(PathStep(3721.981689, 2673.982422, 768.034668, 1.f));//first boss location
	m_PathToBoss.push_back(PathStep(3704.426025, 2834.264404, 798.167175, 0.f));//1.f
	m_PathToBoss.push_back(PathStep(3646.387451, 2955.756348, 804.133911, 1.f));//0.f
}
BOOL MasterSnowdrift::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->Log(2, _T("Boss Master Snowdrift Started!"));
	if (step != 0)
	{
		if (step == 1)
			pGame->GetOnMount(FALSE);
		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
		pGame->RunTo(m_BossRooms[0].x, m_BossRooms[0].y, m_BossRooms[0].z, 1.f, 0.f);
	}
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
		return TRUE;
	if (DistanceTo(pGame, m_BossRooms[0]) < 5.f)
	{
		ByteBuffer packet;
		packet << (uint32)0x1BB0;
		packet.WriteBit(1);
		packet.WriteBit(1);
		packet.FlushBits();
		pGame->SendPacket(CMSG_AREA_TRIGGER, packet);
		if (GetWoWTickCount() > (startTime + 20000))//we are from boss 1
		{
			return FALSE;
		}
	}
	WoWGUID Snowdrift;
	uint32 start = GetWoWTickCount();
	while(GetWoWTickCount() - start < 10000)
	{
		Snowdrift = pGame->GetObject(_T("Master Snowdrift"), true);
		if (!Snowdrift)
			Sleep(100);
		else
			break;
	}
	if (Snowdrift.IsEmpty())
	{
		pGame->Log(2, _T("not found boss Master Snowdrift"));
		return TRUE;
	}
	list<LPCTSTR> monsters;
	monsters.push_back(_T("Flying Snow"));
	monsters.push_back(_T("Fragrant Lotus"));
	monsters.push_back(_T("Shado-Pan Novice"));
	WoWPoint boss2pos(3717.1399, 3096.4500, 817.31897);
	while(!pGame->IsHostileTo(Snowdrift) && pGame->GetUnitHealth(Snowdrift) > 0)//unit disappeared
	{
		while (!pGame->KillClosestMonster(0, &monsters, 100))
			break;
		if (pGame->IsGameEnd())
			return FALSE;
		WoWPoint pos;
		if (pGame->GetUnitPosition(Snowdrift, pos))
		{
			float dx = boss2pos.x - pos.x;
			float dy = boss2pos.y - pos.y;
			//float dz = 817.31897 - pos.z;
			if ((dx * dx + dy * dy) < 10.f)//we finished the encount
				break;
		}
		WoWGUID FlyingSnow, FragrantLotus;
		if (!(FlyingSnow = pGame->GetObject(_T("Flying Snow"), true)).IsEmpty() && !pGame->IsHostileTo(FlyingSnow) && 
			!(FragrantLotus = pGame->GetObject(_T("Fragrant Lotus"), true)).IsEmpty() && !pGame->IsHostileTo(FragrantLotus))
		{
			if (GetWoWTickCount() > (startTime + 60000))
			{
				return FALSE;
			}
			break;
		}
		Sleep(200);
	}
	if (GetWoWTickCount() > (startTime + 60000))
	{
		return FALSE;
	}
	pGame->RunTo(m_BossRooms[1].x, m_BossRooms[1].y, m_BossRooms[1].z, 0.f, 0.f);

	WoWGUID chest;
	uint32 trycount = 0;
	pGame->AddAvoidCastingSpell("Fists of Fury");//FistsOfFuryId
	while ((chest = pGame->GetObject(_T("Snowdrift's Possessions"), true)).IsEmpty())
	{
		if (pGame->IsGameEnd())
			return FALSE;
		WoWGUID Snowdrift2 = pGame->GetObject(_T("Master Snowdrift"), true);
		if (!Snowdrift2.IsEmpty() && !pGame->IsHostileTo(Snowdrift2) && pGame->GetUnitHealth(Snowdrift2) > 0)
		{
			WoWPoint pos;
			if (pGame->GetUnitPosition(Snowdrift2, pos))
			{
				float dx = boss2pos.x - pos.x;
				float dy = boss2pos.y - pos.y;
				//float dz = 817.31897 - pos.z;
				if ((dx * dx + dy * dy) < 10.f)//we finished the encount
				{
					pGame->Log(2, _T("Snowdrift fight done!"));
					break;
				}
			}
		}
		if (Snowdrift2.IsEmpty())
		{
			pGame->Log(2, _T("Not found boss Master Snowdrift, try again:%d"), trycount);
			trycount ++;
			Sleep(200);
		}
		else
			trycount = 0;
		if (trycount > 200)
		{
			pGame->Log(2, _T("Not found boss Master Snowdrift after some tries, break this run!"));
			return TRUE;//let this run go end
		}
		//if (!pGame->KillClosestMonster(0, &monsters, 100))
		if (pGame->Attack(Snowdrift2, 0.f, MasterSnowdriftCallback) != 0)
			Sleep(200);
	}
	trycount = 0;
	while (!pGame->OpenChest(_T("Snowdrift's Possessions")) && trycount ++ < 3)
	{
		if (pGame->IsRooted())
		{
			pGame->Log(2, _T("Player is rooted, try later"));
			Sleep(3000);
		}
		else
		{
			pGame->Log(2, _T("Snowdrift's Possessions: no loot??? sleep some ticks and try again!"));
			Sleep(500);
		}
	}
	pGame->ResetAvoidCastingSpell();
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
	return TRUE;
}

ShaOfViolence::ShaOfViolence()
{
	m_BossRooms.push_back(BossRoom(3996.92, 2905.266, 770.3101, 40.f));

	m_PathToBoss.push_back(PathStep(3695.023, 3065.702, 816.201, 1.f));//chest position
	m_PathToBoss.push_back(PathStep(3728.590576, 3112.706543, 817.357178, 0.f));//1.f
	m_PathToBoss.push_back(PathStep(3927.100098, 2888.940918, 771.107788, 0.f));//0.f
}
BOOL ShaOfViolence::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->Log(2, _T("Boss Sha of Violence Started!"));
	if (step != 0)
	{
		if (GetWoWTickCount() > (startTime + 75000))
		{
			return FALSE;
		}
		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
		{
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
			if (i == 1)
			{
				ByteBuffer packet;
				packet << (uint32)0x1BD1;
				packet.WriteBit(1);
				packet.WriteBit(1);
				packet.FlushBits();
				pGame->SendPacket(CMSG_AREA_TRIGGER, packet);
				int trycount = 0;
				while (pGame->IsInCombat() && trycount ++ < 20)
					Sleep(200);
				pGame->GetOnMount(FALSE);
			}
		}
		if (GetWoWTickCount() > (startTime + 60000))
		{
			return FALSE;
		}
		pGame->RunTo(m_BossRooms[0].x, m_BossRooms[0].y, m_BossRooms[0].z, 1.f, 0.f);
	}
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
		return TRUE;
	WoWGUID sha;
	uint32 start = GetWoWTickCount();
	while(GetWoWTickCount() - start < 5000)
	{
		sha = pGame->GetObject(_T("Sha of Violence"), true);
		if (!sha)
			Sleep(100);
		else
			break;
	}
	if (sha.IsEmpty())
	{
		pGame->Log(2, _T("not found boss Sha of Violence"));
		return TRUE;
	}
	pGame->Attack(sha, 0.f);
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
	return TRUE;
}

BOOL __fastcall PreventAttackOther(IGamePlay *pGame, const WoWGUID &monster)
{
	return FALSE;
}
//Meditate
BOOL __fastcall AttackTaranZhuCallback(IGamePlay *pGame, const WoWGUID &monster)
{
	//if (_tcscmp(pGame->GetObjectName(monster), _T("Gripping Hatred")) == 0)
	//	return FALSE;
	if (pGame->GetPower(10) >= 45)//alternate power means hatred in this combat
	{
		//if (!pGame->CastSpell(_T("Meditate"), WoWGUID(), TRUE)) //i dont know where the extra button spell info
		pGame->Log(2, _T("Casting spell meditate"));
		if (!pGame->CastSpell(107200, WoWGUID(), /*TARGET_FLAG_SELF*/0))
			pGame->Log(2, _T("cast spell Meditate failed!!!!"));
	}
	//WoWGUID ResidualHatred = pGame->GetObject(_T("Gripping Hatred"), true, 11.f);
	//if (!ResidualHatred.IsEmpty())
	//	pGame->Attack(ResidualHatred, 0.f, PreventAttackOther);
	return FALSE;
}
TaranZhu::TaranZhu()
{
	m_BossRooms.push_back(BossRoom(3861.51, 2615.981, 754.5428, 40.f));

	m_PathToBoss.push_back(PathStep(3996.92, 2905.266, 770.3101, 1.f));//1.f
	m_PathToBoss.push_back(PathStep(3949.134521, 2885.140625, 772.013489, 1.f));//1.f
	m_PathToBoss.push_back(PathStep(3882.584473, 2683.577393, 759.053040, 0.f));//0.f
	m_PathToBoss.push_back(PathStep(3901.091064, 2614.472900, 756.204041, 1.f));//0.f
}
BOOL TaranZhu::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	pGame->Log(2, _T("Boss Taran Zhu Started!"));
	if (step != 0)
	{
		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
		{
			if (i == 2)
				pGame->GetOnMount(FALSE);
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
		}
	}
	if (IsCompleted(pGame) || pGame->IsScenarioComplete())
		return TRUE;
	WoWGUID Taran;
	uint32 start = GetWoWTickCount();
	while(GetWoWTickCount() - start < 5000)
	{
		Taran = pGame->GetObject(_T("Taran Zhu"), true);
		if (!Taran)
			Sleep(100);
		else
			break;
	}
	if (Taran.IsEmpty())
	{
		pGame->Log(2, _T("not found boss Taran Zhu"));
		return TRUE;
	}
	pGame->Dismount();
	if (pGame->IsHostileTo(Taran) && GetWoWTickCount() > (startTime + 30000))
	{
		return FALSE;
	}
	pGame->Attack(Taran, 0.f, AttackTaranZhuCallback);
	uint32 trycount = 0;
	//while(pGame->IsInCombat() && trycount ++ < 50)
	//	Sleep(200);
	trycount = 0;
	while (!pGame->OpenChest(_T("Taran Zhu's Personal Stash")) && trycount ++ < 3)
	{
		pGame->Log(2, _T("Taran Zhu:no loot??? sleep some ticks and try again!"));
		Sleep(500);
	}
	uint32 bag = pGame->GetBagSpace();
	bag = (bag >> 8) | (bag & 0x0000FF);
	pGame->PrintGameResult(IGameTunnel::FREE_BAG_SPACE, bag);
	return TRUE;
}

ExitShadoPan::ExitShadoPan()
{
	m_BossRooms.push_back(BossRoom(3649.855, 2548.755, 766.9684, 10.f));//exit point
	m_BossRooms.push_back(BossRoom(3631.290039, 2538.409912, 769.911987, 10.f));//enter point, from pandaria
	m_BossRooms.push_back(BossRoom(3529.825439, 2699.591064, 755.938599, 30.f));//tomb location, from pandaria

	m_PathToBoss.push_back(PathStep(3882.268799, 2582.124023, 757.201477, 0.f));//1.f
	m_PathToBoss.push_back(PathStep(3721.981689, 2673.982422, 768.034668, 0.f));//first boss location
	m_PathToBoss.push_back(PathStep(3621.f, 2624.f, 766.966003, 0.f));//0.f
	m_PathToBoss.push_back(PathStep(3657.290039, 2551.919922, 766.968628, 0.f));//0.f
	m_PathToBoss.push_back(PathStep(3649.855, 2548.755, 766.9684, 0.f));//exit point
}
BOOL ExitShadoPan::Fight(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (step != 0)
	{
		pGame->GetOnMount(FALSE);
		for (uint32 i = step; i < m_PathToBoss.size(); i ++)
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
	}
	if (pGame->GetMapID() == 0x03bf)
		pGame->ChangeArea(0x1E13);//the position of the areatrigger we got is not the with the game, use this function in fixed style
		//pGame->ChangeArea();
	if (pGame->GetMapID() == 0x0366)
	{
		pGame->ResetInstances();
		pGame->RunTo(3643.886, 2544.806, 769.9496, 1.f, 0.f);
		if (!pGame->ChangeArea() || pGame->GetMapID() != 0x03bf)
		{
			pGame->Log(2, _T("chaning area failed, from pandaria to sha-do pan monastery"));
			pGame->RunTo(3631.290039, 2538.409912, 769.911987, 1.f, 0.f);//back to enter position
			return FALSE;
		}
	}
	if (GetWoWTickCount() > (startTime + 45000))
	{
		return FALSE;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////
////
///////////////////////////////////////////////////////////////////////
void CShadoPanMonastery::Load(IBotConfig *config)
{
	CGame::Load(config);
	TCHAR buf[512];
	m_role = 2;
	if (GetPrivateProfileString(config->m_strSection, _T("Role"), NULL, buf, sizeof(buf), config->m_strIniFile))
		m_role = _tcstol(buf, 0, 10);
}

BOOL CShadoPanMonastery::Run(IGamePlay *pGame)
{
	return CDungeonGame::Run(pGame);
}

BOOL CShadoPanMonastery::Prepare(IGamePlay *pGame)
{
	return CGame::Prepare(pGame);
}
