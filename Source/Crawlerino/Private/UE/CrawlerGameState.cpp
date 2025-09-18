// Fill out your copyright notice in the Description page of Project Settings.


#include "UE/CrawlerGameState.h"

void ACrawlerGameState::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	_DungeonGrid = std::make_unique<Crawlerino::DungeonGrid>(DungeonSize.X, DungeonSize.Y);
	Crawlerino::DungeonGrid::GenerateTerrain(*_DungeonGrid, _RoomsToPlace);
}

void ACrawlerGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ACrawlerGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

bool ACrawlerGameState::IsWalkable(const FDungeonPos& Pos) const
{
	bool GridWalkable = _DungeonGrid->IsWalkable(Pos);

	bool CollideWithMonster = false;
	for (AMonsterPawn* Monster : _Monsters)
	{
		if (Monster->GetPos() == Pos)
		{
			CollideWithMonster = true;
			break;
		}
	}

	return GridWalkable && !CollideWithMonster;
}

FDungeonPos ACrawlerGameState::GetRandomWalkablePos() const
{
	FDungeonPos Result;
	do
	{
		Result.X = FMath::RandRange(0, _DungeonGrid->Width() - 1);
		Result.Y = FMath::RandRange(0, _DungeonGrid->Height() - 1);
	}
	while (!_DungeonGrid->IsWalkable(Result));

	return Result;
}

AMonsterPawn* ACrawlerGameState::SpawnMonster(TSubclassOf<AMonsterPawn> Pawn, const FDungeonPos& Pos)
{
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	
	UE_LOG(LogTemp, Log, TEXT("Spawn monster at : (%d;%d)"), Pos.X, Pos.Y);
	FVector SpawnLocation = FVector{Pos.X * 100.0f, Pos.Y * 100.0f, 0.1f};
	AMonsterPawn* MonsterInstance = World->SpawnActor<AMonsterPawn>(Pawn, FTransform(SpawnLocation));

	if (MonsterInstance)
	{
		MonsterInstance->InitializeAtPos(Pos);
		_Monsters.push_back(MonsterInstance);
	}
	
	return MonsterInstance;
}
