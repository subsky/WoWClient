#include "stdafx.h"
#include "FightModule.h"
#include <vector>
#include "Opcodes.h"


const float FightModule::ROOMSIZE = 40.f;
const float FightModule::MINPATHSIZE = 30.f;

class DefaultStopAssistCallback : public StopAssistCallback
{
protected:
	FightModule *m_pFight;
public:
	DefaultStopAssistCallback(float x, float y, float z, StopAssistCallback *pre) : m_x(x), m_y(y), m_z(z), m_pPreCallback(pre) {};
	bool Callback(IGamePlay *pGame)
	{
		WoWPoint pos;
		pGame->GetPosition(&pos.x, &pos.y, &pos.z);
		float dx = m_x - pos.x;
		float dy = m_y - pos.y;
		//float dz = 817.31897 - pos.z;
		if ((dx * dx + dy * dy) < 625.f && fabs(m_z - pos.z) < 8.f)
		{
			return !m_pPreCallback || m_pPreCallback->Callback(pGame);
		}
		return false;
	};
	StopAssistCallback *m_pPreCallback;
	float m_x, m_y, m_z;
};
BOOL FightModule::RunToBossRoom(IGamePlay *pGame, uint32 step, uint32 startTime)
{
	if (step == 0)//already in boos room
		return TRUE;
	if (step < 0 || step >= m_PathToBoss.size())
	{
		pGame->Log(2, _T("RunToBoSS error: we are not at the right start step:%d"), step);
		return FALSE;
	}
	for (uint32 i = step; i < m_PathToBoss.size(); i++)
	{
		bool completed = IsCompleted(pGame);
		BeforeStep(pGame, i, startTime);
		WoWGUID tank;
		if (!pGame->GetWorkingGroup().id.IsEmpty() && !(pGame->GetRole() & 2))
		{
			pGame->Lock();
			tank = pGame->GetWorkingGroup().GetTank();
			pGame->Unlock();
		}
		if (completed || tank.IsEmpty() || m_PathToBoss[i].speed == 0.f)
		{
			if (m_PathToBoss[i].speed == 0.f)
			{
				if (!pGame->HasGuardianPet().IsEmpty())
				{
					if (!pGame->CastSpell(_T("Dismiss Pet"), WoWGUID(0)))
					{
						//flag == 7 delete, flag == C1 castspell
						pGame->PetAction(0x03, 0x07, WoWGUID(0), NULL);
					}
				}
			}
			while (!IsCloseToPathPoint(pGame, i))
			{
				if (pGame->IsGameEnd() || pGame->IsDead())
					return FALSE;
				pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
			}
		}
		else
		{
			DefaultStopAssistCallback default_cb(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, stopAssistCB);
			//first follow tank
			pGame->AssistPlayer(tank, &default_cb);
			//second, if there's some steps between the dest, run to it
			pGame->RunTo(m_PathToBoss[i].x, m_PathToBoss[i].y, m_PathToBoss[i].z, m_PathToBoss[i].speed, 0.f);
		}
	}
	return TRUE;
}

BOOL FightModule::IsInBossRoom(WoWPoint &current)
{
	for(uint32 i = 0; i < m_BossRooms.size(); i ++)
	{
	    float dx = current.x - m_BossRooms[i].x;
		float dy = current.y - m_BossRooms[i].y;
		float dist = /*sqrt*/((dx*dx) + (dy*dy));
		if (dist < m_BossRooms[i].speed * m_BossRooms[i].speed)
		{
			if (fabs(current.z - m_BossRooms[i].z) < m_BossRooms[i].z_diff)
				return TRUE;
		}
	}
	return FALSE;
}
//-1 not found
uint32 FightModule::IsAtPathLine(WoWPoint &current)
{
	for (uint32 i = 1; i < m_PathToBoss.size(); i ++)
	{
		float x1 = current.x - m_PathToBoss[i - 1].x;
		float x2 = current.x - m_PathToBoss[i].x;
		float y1 = current.y - m_PathToBoss[i - 1].y;
		float y2 = current.y - m_PathToBoss[i].y;
		if (fabs(x1) < 3.f && fabs(y1) < 3.f)
			return i;
		if (fabs(x2) < 3.f && fabs(y2) < 3.f)
		{
			if (i + 1 == m_PathToBoss.size())
				return 0;//we are at last step, take it as we are at room
			return i + 1;
		}
		if ((x1 > 0.f && x2 > 0.f) || (x1 < 0.f && x2 < 0.f) || (y1 > 0.f && y2 > 0.f) || (y1 < 0.f && y2 < 0.f))
			continue;
		float dx = m_PathToBoss[i].x - m_PathToBoss[i - 1].x;
		float dy = m_PathToBoss[i].y - m_PathToBoss[i - 1].y;
		float value = 0.f;
		if (fabs(dx) > fabs(dy))
		{
			if (fabs(current.y - ((current.x - m_PathToBoss[i - 1].x) * dy / dx + m_PathToBoss[i - 1].y)) < MINPATHSIZE)
				return i;
		}
		else
		{
			if (fabs(current.x - ((current.y - m_PathToBoss[i - 1].y) * dx / dy + m_PathToBoss[i - 1].x)) < MINPATHSIZE)
				return i;
		}
	}
	return -1;
}
float FightModule::BestDistanceInPath(WoWPoint &current, uint32 &index)
{
	float result = FLT_MAX;
	for (int i = 1; i < m_PathToBoss.size(); i++)
	{
		if (m_PathToBoss[i].speed != 1.f)
			continue;
		float dx = m_PathToBoss[i].x - current.x;
		float dy = m_PathToBoss[i].y - current.y;
		float dz = m_PathToBoss[i].z - current.z;
		float d = sqrt((dx * dx + dy * dy + dz * dz));
		if (d < result)
		{
			result = d;
			index = i;
		}
	}
	return result;
}

float FightModule::DistanceTo(IGamePlay *pGame, WoWPoint &p)
{
	WoWPoint pos;
	pGame->GetPosition(&pos.x, &pos.y, &pos.z);
	float dx = p.x - pos.x;
	float dy = p.y - pos.y;
	//float dz = 817.31897 - pos.z;
	return sqrt((dx * dx + dy * dy));
}
bool FightModule::IsCloseToPathPoint(IGamePlay *pGame, int index)
{
	WoWPoint pos;
	pGame->GetPosition(&pos.x, &pos.y, &pos.z);
	float dx = m_PathToBoss[index].x - pos.x;
	float dy = m_PathToBoss[index].y - pos.y;
	//float dz = 817.31897 - pos.z;
	return ((dx * dx + dy * dy)) < 25.f && fabs(m_PathToBoss[index].z - pos.z) < 5.f;
}
