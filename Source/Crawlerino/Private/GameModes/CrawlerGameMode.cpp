// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/CrawlerGameMode.h"


#include "Kismet/GameplayStatics.h"

void ACrawlerGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogTemp, Log, TEXT("Initializing game (loading level %s)"), *MapName);

	_Status = CrawlerStatus::Exploration;
}

void ACrawlerGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	auto ExplorationPawn = static_cast<AFirstPersonPawn*>(NewPlayer->GetPawn());

	if (!ExplorationPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve ExplorationPawn, this is not supposed to happen"))
		return;
	}

	_ExplorationPawn = ExplorationPawn;
}

void ACrawlerGameMode::InitiateCombat(AMonsterPawn* Monster)
{
	if (_Status == CrawlerStatus::Combat)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot switch to combat while already in combat"))
		return;
	}
	
	auto PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve PlayerController, abort SwitchToCombat"))
		return;
	}

	auto FPPawn = static_cast<AFirstPersonPawn*>(PlayerController->GetPawn());

	if (!FPPawn)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve FirstPersonPawn, abort SwitchToCombat"))
		return;
	}

	auto CombatPawn = SpawnOrGetCombatPawn();

	FDungeonPos MonsterPos = Monster->GetPos();
	FDungeonPos PlayerPos = FPPawn->GetPos();

	_CombatManager = std::make_unique<CombatManager>(FStatSheet{100, 50}, FStatSheet{100, 20});

	Direction PlayerToMonsterDir = Crawlerino::Utils::ComputeDirection(PlayerPos, MonsterPos);
	UE_LOG(LogTemp, Log, TEXT("Player pos : %d %d; Monster pos : %d %d; Direction %d"), PlayerPos.X, PlayerPos.Y, MonsterPos.X, MonsterPos.Y, PlayerToMonsterDir);

	FRotator CombatPawnRotator = FRotator{0.0, Crawlerino::GetYawFromDirection(PlayerToMonsterDir), 0.0};
	FVector CombatPawnLocation = FVector{PlayerPos.X * 100.0f, PlayerPos.Y * 100.0f, 50.0f};
	CombatPawn->Init(CombatPawnLocation, CombatPawnRotator);

	PlayerController->Possess(CombatPawn);
	_Status = CrawlerStatus::Combat;
}

void ACrawlerGameMode::CastAction(APawn* Pawn)
{
	if (Pawn == _CombatPawn)
	{
		// player attack monster, if return true then turn has switched to monster, perform monster action
		if (_CombatManager->InflictDamage(0, 1))
		{
			auto CombatResult = _CombatManager->GetCombatResult();
			if (CombatResult != CombatManager::OnGoing)
			{
				EndCombat(CombatResult::Won);
			}
			
			if (_CombatManager->InflictDamage(1, 0))
			{
				CombatResult = _CombatManager->GetCombatResult();
				if (CombatResult != CombatManager::OnGoing)
				{
					EndCombat(CombatResult::Won);
				}
			}

		}
	}
}


void ACrawlerGameMode::EndCombat(const CombatResult& Result)
{
	if (_Status == CrawlerStatus::Exploration)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot switch to exploration while already in exploration"))
		return;
	}

	auto PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve PlayerController, abort SwitchToCombat"))
		return;
	}

	PlayerController->Possess(_ExplorationPawn);
	_CombatPawn->Clean();
	_Status = CrawlerStatus::Exploration;
}

ACombatPawn* ACrawlerGameMode::SpawnOrGetCombatPawn()
{
	if (_CombatPawn) return _CombatPawn;
	
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	_CombatPawn = World->SpawnActor<ACombatPawn>(CombatPawnClass);
	return _CombatPawn;
}

Direction Crawlerino::Utils::ComputeDirection(const FDungeonPos& PlayerPos, const FDungeonPos& MonsterPos)
{
	FDungeonPos Delta = MonsterPos - PlayerPos;

	if (abs(Delta.X) > abs(Delta.Y))
	{
		if (Delta.X > 0) return Direction::North;
		else return Direction::South;
	} else
	{
		if (Delta.Y > 0) return Direction::West;
		else return Direction::East;
	}
}


