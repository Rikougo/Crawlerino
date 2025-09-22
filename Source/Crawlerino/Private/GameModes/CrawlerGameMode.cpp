// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/CrawlerGameMode.h"
#include "Kismet/GameplayStatics.h"

#include "Characters/Enemies/MonsterController.h"
#include "Characters/Player/CrawlerPlayerController.h"


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


	Direction PlayerToMonsterDir = Crawlerino::Utils::ComputeDirection(PlayerPos, MonsterPos);
	UE_LOG(LogTemp, Log, TEXT("Player pos : %d %d; Monster pos : %d %d; Direction %d"), PlayerPos.X, PlayerPos.Y, MonsterPos.X, MonsterPos.Y, PlayerToMonsterDir);

	FRotator CombatPawnRotator = FRotator{0.0, Crawlerino::GetYawFromDirection(PlayerToMonsterDir), 0.0};
	FVector CombatPawnLocation = FVector{PlayerPos.X * 100.0f, PlayerPos.Y * 100.0f, 50.0f};
	CombatPawn->Init(CombatPawnLocation, CombatPawnRotator);

	PlayerController->Possess(CombatPawn);
	_EnemyPawn = Monster;
	_CombatManager = std::make_unique<CombatManager>(FStatSheet{100, 50}, FStatSheet{100, 20});
	
	_CombatEntities.emplace_back(CombatEntity{CombatPawn, 0});
	_CombatEntities.emplace_back(CombatEntity{_EnemyPawn, 1});
	
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
	_EnemyPawn->Destroy();
	_EnemyPawn = nullptr;
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


