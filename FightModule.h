#pragma once
#include "IGamePlay.h"
#include "Markup.h"
#include <vector>

using namespace std;

struct PathStep : WoWPoint
{
	PathStep(float x, float y, float z, float s) : WoWPoint(x, y, z), speed(s)
	{
	};
	float speed;
};
struct BossRoom : public PathStep
{
	BossRoom(float x, float y, float z, float s, float zDiff = 10.f) : PathStep(x, y, z, s), z_diff(zDiff)
	{

	};
	float z_diff;
};

class FightModule
{
public:
	static const float ROOMSIZE;
	static const float MINPATHSIZE;

	FightModule() : m_startTime(0), stopAssistCB(0)
	{
	};
	virtual ~FightModule()
	{
	};
	virtual BOOL RunToBossRoom(IGamePlay *pGame, uint32 step, uint32 startTime);
	virtual BOOL BeforeStep(IGamePlay *pGame, uint32 step, uint32 startTim)
	{
		return TRUE;
	};
	//step = 0, at bos room
	virtual BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime) = 0;

	virtual BOOL IsCompleted(IGamePlay *pGame) = 0;

	virtual BOOL IsInBossRoom(WoWPoint &current);
	//-1 not found
	uint32 IsAtPathLine(WoWPoint &current);
	float BestDistanceInPath(WoWPoint &current, uint32 &index);

	vector<BossRoom> m_BossRooms;
	vector<PathStep> m_PathToBoss;

	float DistanceTo(IGamePlay *pGame, WoWPoint &p);
	bool IsCloseToPathPoint(IGamePlay *pGame, int index);

	uint32 m_startTime;
	StopAssistCallback *stopAssistCB;
};


class ExitModule : public FightModule
{
public:

	ExitModule(uint32 instanceId, uint32 outsideId) : m_instanceId(instanceId), m_outsideId(outsideId)
	{
	};
	virtual ~ExitModule()
	{
	};
	//step = 0, at bos room
	virtual BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime) = 0;
	virtual BOOL IsCompleted(IGamePlay *pGame)
	{
		return TRUE;
	};

	uint32 m_instanceId;
	uint32 m_outsideId;
};
class LFGExitModule : public ExitModule
{
public:

	LFGExitModule(uint32 instanceId, uint32 outsideId) : ExitModule(instanceId, outsideId)
	{
	};
	virtual ~LFGExitModule()
	{
	};
	//step = 0, at bos room
	virtual BOOL Fight(IGamePlay *pGame, uint32 step, uint32 startTime) = 0;
};
