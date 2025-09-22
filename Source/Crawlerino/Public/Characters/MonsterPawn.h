// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "UE/CrawlerDungeonSubsystem.h"

#include "MonsterPawn.generated.h"

UCLASS()
class CRAWLERINO_API AMonsterPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMonsterPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	void InitializeAtPos(const FDungeonPos& Pos)
	{
		_TerrainPos = Pos;
	}
	
	FDungeonPos GetPos() const { return _TerrainPos; }
private:
	FDungeonPos _TerrainPos;
};
