// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GenericPlatform/GenericPlatformMath.h"

struct CRAWLERINO_API FStatSheet
{
private:
	int16 _Health = -1;
	int16 _MaxHealth = -1;
	int16 _Damage = 0;
public:
	FStatSheet() = default;
	FStatSheet(int16 MaxHealth, int16 Damage)
	{
		_MaxHealth = MaxHealth;
		_Health = MaxHealth;
		_Damage = Damage;
	}
	~FStatSheet() = default;

	int GetHealth() const { return _Health; }
	int GetMaxHealth() const { return _MaxHealth; }
	float GetHealthPercent() const { return _Health / _MaxHealth; }

	int GetDamage() const { return _Damage; }

	void Heal(int16 Amount) { _Health = FGenericPlatformMath::Min(_Health + Amount, _MaxHealth); }
	void InflictDamage(int16 Damage) { _Health -= Damage; }

	bool IsAlive() const { return _Health > 0; }
};
