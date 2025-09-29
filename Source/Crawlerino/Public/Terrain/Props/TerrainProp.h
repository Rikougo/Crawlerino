// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameModes/CrawlerGameState.h"
#include "Terrain/DungeonGrid.h"

#include "Terrain/FDungeonPos.h"

#include "TerrainProp.generated.h"

UCLASS()
class CRAWLERINO_API ATerrainProp : public AActor
{
	GENERATED_BODY()

public:
	ATerrainProp() = default;

	void Initialize(ACrawlerGameState* GameState, const FDungeonPos& Pos, const FDirection& Dir)
	{
		_GameState = GameState;

		SetPosition(Pos);
		SetOrientation(Dir);
	}

	UFUNCTION(BlueprintPure)
	FDungeonPos GetPos() const { return _DungeonPos; }
	UFUNCTION(BlueprintPure)
	FDirection GetDirection() const { return _Direction; }
protected:
	UFUNCTION(BlueprintCallable)
	ACrawlerGameState* GetGameState() { return _GameState; };
private:
	void SetPosition(const FDungeonPos& Pos)
	{
		_DungeonPos = Pos;
		SetActorLocation(_GameState->GetWorldPos(Pos));
	}
	void SetOrientation(const FDirection Dir)
	{
		_Direction = Dir;
		SetActorRotation(FRotator(0, Crawlerino::GetYawFromDirection(Dir), 0));
	}
protected:
	ACrawlerGameState* _GameState;
	
	FDungeonPos _DungeonPos;
	FDirection _Direction;
};
