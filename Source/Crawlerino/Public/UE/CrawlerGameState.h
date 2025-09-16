// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/DungeonGrid.h"
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
	FDungeonPos GetRandomWalkablePos() const
	{
		FDungeonPos Result{};
		do
		{
			Result.X = FMath::RandRange(0, _DungeonGrid->Width() - 1);
			Result.Y = FMath::RandRange(0, _DungeonGrid->Height() - 1);
		} while (!_DungeonGrid->CanMoveTo(Result.X, Result.Y));

		return Result;
	}
public:
	Crawlerino::DungeonGrid& GetDungeonGrid() const { return *_DungeonGrid; }
	
private:
	std::unique_ptr<Crawlerino::DungeonGrid> _DungeonGrid;
};
