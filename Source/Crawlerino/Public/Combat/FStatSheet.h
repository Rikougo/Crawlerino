// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GenericPlatform/GenericPlatformMath.h"

#include "FStatSheet.generated.h"

USTRUCT(BlueprintType)
struct CRAWLERINO_API FStatSheet
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Statistics")
	int32 Health = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Statistics")
	int32 MaxHealth = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Statistics")
	int32 Damage = 0;
public:
	FStatSheet() = default;
	FStatSheet(int32 MHealth, int32 Dmg)
	{
		MaxHealth = MHealth;
		Health = MHealth;
		Damage = Dmg;
	}
	~FStatSheet() = default;
};

class CRAWLERINO_API FStatManipulator
{
public:
	FStatManipulator() = delete;
	FStatManipulator(const FStatSheet& FromSheet, bool IsPlayer)
	{
		_Sheet = FromSheet;
		_IsPlayer = IsPlayer;
	}

	int GetHealth() const { return _Sheet.Health; }
	int GetMaxHealth() const { return _Sheet.MaxHealth; }
	float GetHealthPercent() const { return _Sheet.Health / _Sheet.MaxHealth; }

	int GetDamage() const { return _Sheet.Damage; }

	void Heal(int16 Amount) { _Sheet.Health = FGenericPlatformMath::Min(_Sheet.Health + Amount, _Sheet.MaxHealth); }
	void InflictDamage(int16 Damage) { _Sheet.Health -= Damage; }

	bool IsAlive() const { return _Sheet.Health > 0; }
	bool IsPlayer() const { return _IsPlayer; }
private:
	bool _IsPlayer;
	FStatSheet _Sheet;
};
