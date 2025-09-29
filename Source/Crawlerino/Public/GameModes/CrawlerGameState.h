// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "Characters/Enemies/MonsterPawn.h"
#include "Terrain/DungeonGrid.h"

#include "CrawlerGameState.generated.h"

class ATerrainProp;

/**
 * 
 */
UCLASS()
class CRAWLERINO_API ACrawlerGameState : public AGameStateBase
{
	GENERATED_BODY()
private:
	const std::vector<Crawlerino::RoomDescriptor> _RoomsToPlace {
		Crawlerino::RoomDescriptor{2,2},
		Crawlerino::RoomDescriptor{2,2},
		Crawlerino::RoomDescriptor{2,2},
		Crawlerino::RoomDescriptor{4,4},
		Crawlerino::RoomDescriptor{4,4},
		Crawlerino::RoomDescriptor{4,4},
		Crawlerino::RoomDescriptor{10,10}
	};
public:
	UPROPERTY(EditAnywhere)
	FDungeonPos DungeonSize{32,32};

	UPROPERTY(EditAnywhere)
	float CellSize = 100.0f;
protected:
	ACrawlerGameState() = default;
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	UFUNCTION(BlueprintCallable)
	FVector GetWorldPos(const FDungeonPos& Pos) const
	{
		return FVector{Pos.X * CellSize, Pos.Y * CellSize, CellSize / 2.0f};
	};
	
	UFUNCTION(BlueprintPure)
	bool IsWalkable(const FDungeonPos& Pos) const;
	
	UFUNCTION(BlueprintPure)
	FDungeonPos GetRandomWalkablePos() const;
public:
	Crawlerino::DungeonGrid& GetDungeonGrid() const { return *_DungeonGrid; }

	UFUNCTION(BlueprintCallable, Category = "CrawlerGameState")
	AMonsterPawn* SpawnMonster(TSubclassOf<AMonsterPawn> Pawn, const FDungeonPos& Pos);

	UFUNCTION(BlueprintCallable, Category = "CrawlerGameState")
	bool KillMonster(AMonsterPawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "CrawlerGameState")
	ATerrainProp* SpawnTerrainProp(TSubclassOf<ATerrainProp> Prop, const FDungeonPos& Pos, const FDirection& Dir);
private:
	std::unique_ptr<Crawlerino::DungeonGrid> _DungeonGrid;
	
	std::vector<AMonsterPawn*> _Monsters;
	std::vector<ATerrainProp*> _TerrainProps;
};
