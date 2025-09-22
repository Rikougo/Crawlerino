#pragma once

#include "Combat/FStatSheet.h"

class CombatManager
{
private:
	typedef std::unique_ptr<FStatManipulator> FStatManipulatorPtr;
	typedef std::vector<FStatManipulator> FStatManipulatorList;
public:
	enum CombatResult
	{
		OnGoing,
		PlayerWin,
		MonsterWin,
	};

public:
	CombatManager() = delete;
	CombatManager(const FStatSheet& PlayerStats, const FStatSheet& MonsterStats)
	{
		_EntitiesStats = {};
		_EntitiesStats.push_back(FStatManipulator(PlayerStats, true));
		_EntitiesStats.push_back(FStatManipulator(MonsterStats, false));

		_TurnCounter = 0;
		_TurnOwner = 0; // PLAYER START
		_IsEnded = false;
	}

	int CurrentOwner() const { return _TurnOwner;}
	CombatResult GetCombatResult() const { return _EndResult;}
	
	bool InflictDamage(int Source, int Target)
	{
		if (_TurnOwner != Source)
		{
			return false; // only turn owner can perform action
		}

		auto& OwnerStats = GetOwnerStats();
		auto& TargetStats = GetStats(Target);
		
		TargetStats.InflictDamage(OwnerStats.GetDamage());

		CheckEndCondition();

		ConsumeTurn();

		return true;
	}

private:
	FStatManipulator& GetOwnerStats()
	{
		return GetStats(_TurnOwner);
	}
	FStatManipulator& GetStats(int Index)
	{
		return _EntitiesStats[Index];
	}

	void CheckEndCondition()
	{
		if (_IsEnded) { return;}

		auto PlayerCheck = [](auto& Stats)
		{
			bool IsPlayer = Stats.IsPlayer();
			bool IsAlive = Stats.IsAlive();

			return (!IsPlayer) || (IsPlayer && IsAlive);
		};
		if (!std::ranges::all_of(_EntitiesStats,PlayerCheck))
		{
			_IsEnded = true;
			_EndResult = MonsterWin;
			return;
		}

		auto MonsterCheck = [](auto& Stats)
		{
			bool IsMonter = !Stats.IsPlayer();
			bool IsAlive = Stats.IsAlive();

			return (!IsMonter) || (IsMonter && IsAlive);
		};
		if (!std::ranges::all_of(_EntitiesStats,MonsterCheck))
		{
			_IsEnded = true;
			_EndResult = PlayerWin;
			return;
		}
	}
	
	void ConsumeTurn()
	{
		_TurnOwner = _TurnOwner == 0 ? 1 : 0;
		_TurnCounter++;
	}
private:
	FStatManipulatorList _EntitiesStats;

	int _TurnCounter = 0;
	int _TurnOwner = 0; // index of the one currently playing
	bool _IsEnded = false;
	CombatResult _EndResult = CombatResult::OnGoing;

};
