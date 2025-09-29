// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/CrawlerGameMode.h"
#include "Kismet/GameplayStatics.h"

#include "Characters/Enemies/MonsterController.h"
#include "Characters/Player/CrawlerPlayerController.h"


namespace
{
	FString GetStatusName(CrawlerStatus Status)
	{
		switch (Status)
		{
		case CrawlerStatus::Exploration:
			return "Exploration";
		case CrawlerStatus::Rest:
			return "Rest";
		case CrawlerStatus::Combat:
			return "Combat";
		default:
			return "Unknown";
		}
	}
}


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
	if (_Status != CrawlerStatus::Exploration)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot switch to combat while not in exploration, current status %s"), *GetStatusName(_Status))
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


	FDirection PlayerToMonsterDir = Crawlerino::Utils::ComputeDirection(PlayerPos, MonsterPos);
	UE_LOG(LogTemp, Log, TEXT("Player pos : %d %d; Monster pos : %d %d; Direction %d"), PlayerPos.X, PlayerPos.Y, MonsterPos.X, MonsterPos.Y, PlayerToMonsterDir);

	FRotator CombatPawnRotator = FRotator{0.0, Crawlerino::GetYawFromDirection(PlayerToMonsterDir), 0.0};
	FVector CombatPawnLocation = FVector{PlayerPos.X * 100.0f, PlayerPos.Y * 100.0f, 50.0f};
	CombatPawn->Init(CombatPawnLocation, CombatPawnRotator);

	PlayerController->Possess(CombatPawn);
	_CombatManager = std::make_unique<CombatManager>(FStatSheet{100, 50}, FStatSheet{100, 20});
	
	_CombatEntities.emplace_back(CombatEntity{CombatPawn, 0});
	_CombatEntities.emplace_back(CombatEntity{Monster, 1});
	
	_Status = CrawlerStatus::Combat;
}

void ACrawlerGameMode::CastAction(APawn* Pawn, int Target)
{
	CombatEntity Entity{nullptr, -1};
	for (CombatEntity Value : _CombatEntities)
	{
		if (Value.Pawn == Pawn)
		{
			Entity = Value;
		}
	}

	if (Entity.Pawn == nullptr)
	{
		// no entity associated to pawn, doing nothing
		return;
	}

	if (Entity.Index != _CombatManager->CurrentOwner())
	{
		// entity isn't turn owner, can't perform action
		return;
	}

	if (_CombatManager->InflictDamage(Entity.Index, Target))
	{
		auto CombatResult = _CombatManager->GetCombatResult();

		if (CombatResult != CombatManager::OnGoing)
		{
			auto Result = CombatResult == CombatManager::PlayerWin ? CombatResult::Won : CombatResult::Lost;
			EndCombat(Result);
		} else
		{
			CombatEntity NewOwner{};
			if (FetchEntity(_CombatManager->CurrentOwner(), NewOwner))
			{
				auto Controller = NewOwner.Pawn->GetController();

				if (auto MonsterController = static_cast<AMonsterController*>(Controller))
				{
					MonsterController->StartTurn();
				}

				if (auto PlayerController = static_cast<ACrawlerPlayerController*>(Controller))
				{
					PlayerController->StartTurn();
				}
				// TODO Call either Player or Monster controller controlling new owner's pawn
			}
		}
	}
}

void ACrawlerGameMode::EndCombat(const CombatResult& Result)
{
	if (_Status != CrawlerStatus::Combat)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot exit combat while not in combat, current status %s"), *GetStatusName(_Status))
		return;
	}

	auto PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve PlayerController, abort SwitchToCombat"))
		return;
	}

	if (Result == CombatResult::Won)
	{
		for (CombatEntity CombatEntity : _CombatEntities)
		{
			if (auto MonsterPawn = static_cast<AMonsterPawn*>(CombatEntity.Pawn))
			{
				GetGameState<ACrawlerGameState>()->KillMonster(MonsterPawn);
			}
		} 
	}

	_CombatEntities.clear();

	PlayerController->Possess(_ExplorationPawn);
	_CombatPawn->Clean();

	_CombatManager.release();
	
	_Status = CrawlerStatus::Exploration;
}

void ACrawlerGameMode::InitiateRest(ABoneFireProp* TargetFire)
{
	if (_Status != CrawlerStatus::Exploration)
	{
		UE_LOG(LogTemp, Error, TEXT("Cannot rest while not in exploration, current status %s"), *GetStatusName(_Status))
	}

	_RestFire = TargetFire;

	FDungeonPos ExplorationPos = _ExplorationPawn->GetPos();
	FDungeonPos FirePos = TargetFire->GetPos();
	FDirection Dir = Crawlerino::Utils::ComputeDirection(ExplorationPos, FirePos);
	
	auto RestPawn = SpawnOrGetRestPawn();
	RestPawn->Init(ExplorationPos, Dir);

	auto PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	PlayerController->Possess(RestPawn);
	
	_Status = CrawlerStatus::Rest;
}

void ACrawlerGameMode::EndRest()
{
	
}

ACombatPawn* ACrawlerGameMode::SpawnOrGetCombatPawn()
{
	if (_CombatPawn) return _CombatPawn;
	
	UWorld* World = GetWorld();
	if (!World) return nullptr;

	_CombatPawn = World->SpawnActor<ACombatPawn>(CombatPawnClass);
	return _CombatPawn;
}

ARestPawn* ACrawlerGameMode::SpawnOrGetRestPawn()
{
	if (_RestPawn) return _RestPawn;

	UWorld* World = GetWorld();
	if (!World) return nullptr;

	_RestPawn = World->SpawnActor<ARestPawn>(RestPawnClass);
	return _RestPawn;
	
}

bool ACrawlerGameMode::FetchEntity(const APawn* Pawn, CombatEntity& Result) const
{
	for (CombatEntity Value : _CombatEntities)
	{
		if (Value.Pawn == Pawn)
		{
			Result = {Value};
			return true;
		}
	}

	return false;
}

bool ACrawlerGameMode::FetchEntity(int Index, CombatEntity& Result) const
{
	for (CombatEntity Value : _CombatEntities)
	{
		if (Value.Index == Index)
		{
			Result = {Value};
			return true;
		}
	}

	return false;
}

FDirection Crawlerino::Utils::ComputeDirection(const FDungeonPos& PlayerPos, const FDungeonPos& MonsterPos)
{
	FDungeonPos Delta = MonsterPos - PlayerPos;

	if (abs(Delta.X) > abs(Delta.Y))
	{
		if (Delta.X > 0) return FDirection::North;
		else return FDirection::South;
	} else
	{
		if (Delta.Y > 0) return FDirection::West;
		else return FDirection::East;
	}
}


