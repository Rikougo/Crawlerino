// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/DungeonGrid.h"
#include "UE/MonsterPawn.h"
#include "GameFramework/GameStateBase.h"
#include "CrawlerGameState.generated.h"

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
protected:
	ACrawlerGameState();
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	UFUNCTION(BlueprintPure)
	bool IsWalkable(const FDungeonPos& Pos) const;
	
	UFUNCTION(BlueprintPure)
	FDungeonPos GetRandomWalkablePos() const;
public:
	Crawlerino::DungeonGrid& GetDungeonGrid() const { return *_DungeonGrid; }

	UFUNCTION(BlueprintCallable, Category = "CrawlerGameMode")
	AMonsterPawn* SpawnMonster(TSubclassOf<AMonsterPawn> Pawn, const FDungeonPos& Pos);
private:
	std::unique_ptr<Crawlerino::DungeonGrid> _DungeonGrid;
	
	std::vector<AMonsterPawn*> _Monsters;
};
