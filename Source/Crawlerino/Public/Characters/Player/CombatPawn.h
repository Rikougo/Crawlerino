// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"

#include "GameModes/CrawlerGameState.h"
#include "UE/CrawlerDungeonSubsystem.h"
#include "CombatPawn.generated.h"

UCLASS()
class CRAWLERINO_API ACombatPawn : public APawn
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* Camera;
public:
	// Sets default values for this pawn's properties
	ACombatPawn();
private:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
public:
	void Init(const FVector& Location, const FRotator& Rotation);
	void Clean();
private:
	ACrawlerGameState* _GameState;

	Crawlerino::RoomInfo _RoomInfo{};
};
