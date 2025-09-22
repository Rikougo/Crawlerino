// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Characters/CombatPawn.h"
#include "Characters/FirstPersonPawn.h"
#include "Characters/MonsterPawn.h"
#include "Combat/CombatManager.h"
#include "Combat/FStatSheet.h"
#include "Terrain/DungeonGrid.h"
#include "Terrain/FDungeonPos.h"

#include "CrawlerGameMode.generated.h"

UENUM(BlueprintType)
enum class CrawlerStatus : uint8
{
	Exploration,
	Combat
};

UENUM(BlueprintType)
enum class CombatResult : uint8
{
	Won,
	Lost
};

/**
 * 
 */
UCLASS()
class CRAWLERINO_API ACrawlerGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, NoClear, BlueprintReadOnly, Category=Classes)
	TSubclassOf<ACombatPawn> CombatPawnClass;
protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
public:
	/**
	 * Switch player to configured CombatPawnClass instance and switch to Combat status. CrawlerGameMode manage all combat
	 * rules such as turn and winning or losing condition
	 * @param Monster Combat target
	 */
	UFUNCTION(BlueprintCallable, Category="CrawlerGameMode")
	virtual void InitiateCombat(AMonsterPawn* Monster);

	UFUNCTION(BlueprintCallable, Category="CrawlerGameMode")
	virtual void CastAction(APawn* Pawn); 

	UFUNCTION(BlueprintCallable, Category="CrawlerGameMode")
	virtual void EndCombat(const CombatResult& Result);
private:
	ACombatPawn* SpawnOrGetCombatPawn();
private:
	CrawlerStatus _Status;

	AFirstPersonPawn* _ExplorationPawn;
	ACombatPawn* _CombatPawn;

	std::unique_ptr<CombatManager> _CombatManager;
};

namespace Crawlerino
{
	namespace Utils
	{
		Direction ComputeDirection(const FDungeonPos& PlayerPos, const FDungeonPos& MonsterPos);
	}
}
