#pragma once

#include "Core/FStatSheet.h"

class CombatManager
{
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
		_PlayerStats = PlayerStats;
		_MonsterStats = MonsterStats;

		_TurnCounter = 0;
		_TurnOwner = 0; // PLAYER START
		_IsEnded = false;
	}

	bool InflictDamage(int Source, int Target)
	{
		if (_TurnOwner != Source)
		{
			return false; // only turn owner can perform action
		}

		FStatSheet* OwnerStats = GetOwnerStats();

		FStatSheet* TargetStats = GetStats(Target);
		if (TargetStats == nullptr)
		{
			return false; // error wrong target
		}

		TargetStats->InflictDamage(OwnerStats->GetDamage());

		CheckEndCondition();

		ConsumeTurn();

		return true;
	}

	CombatResult GetCombatResult() const { return _EndResult;}
private:
	FStatSheet* GetOwnerStats()
	{
		return GetStats(_TurnOwner);
	}
	FStatSheet* GetStats(int Index)
	{
		switch (_TurnOwner)
		{
			case 0:
				return &_PlayerStats;
			case 1:
				return &_MonsterStats;
			default:
				return nullptr;
		}
	}

	void CheckEndCondition()
	{
		if (_IsEnded) { return;}
		
		if (!_PlayerStats.IsAlive())
		{
			_IsEnded = true;
			_EndResult = MonsterWin;
			return;
		}
		
		if (!_MonsterStats.IsAlive())
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
	FStatSheet _PlayerStats{0,-1};
	FStatSheet _MonsterStats{0, -1};

	int _TurnCounter = 0;
	int _TurnOwner = 0; // index of the one currently playing
	bool _IsEnded = false;
	CombatResult _EndResult = CombatResult::OnGoing;

};
