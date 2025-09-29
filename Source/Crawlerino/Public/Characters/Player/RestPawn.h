// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"

#include "GameModes/CrawlerGameState.h"
#include "Terrain/DungeonGrid.h"
#include "Terrain/FDungeonPos.h"

#include "RestPawn.generated.h"

UCLASS()
class CRAWLERINO_API ARestPawn : public APawn
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* Camera;
public:
	// Sets default values for this pawn's properties
	ARestPawn();
protected:
	virtual void BeginPlay() override;
public:	
	void Init(const FDungeonPos& Pos, const FDirection& Direction);
private:
	ACrawlerGameState* _GameState;
	
	FDungeonPos _Terrainpos;
	FDirection _FacingDirection;
};
