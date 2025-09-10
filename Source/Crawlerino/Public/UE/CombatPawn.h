// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CrawlerDungeonSubsystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "CombatPawn.generated.h"

UCLASS()
class CRAWLERINO_API ACombatPawn : public APawn
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* Camera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator CameraRot{-30.0f, 0.0f, 0.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Offset{100.0f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TransitionTime{1.0f};
	
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
	UFUNCTION(BlueprintCallable)
	virtual void InitAtRoom(int PosX, int PosY, Direction Direction, FVector CameraStartPos, FRotator CameraStartRot);
private:
	void StartTransition(const FVector& StartPos, const FRotator& StartRot, const FVector& TargetPos, const FRotator& TargetRot);
	void EndTransition();
private:
	UCrawlerDungeonSubsystem* _DungeonSubsystem;

	bool _IsAnimating{};
	double _AnimationTime{};
	FVector _CameraStartPos{};
	FRotator _CameraStartRot{};
	FVector _CameraTargetPos{};
	FRotator _CameraTargetRot{};

	Crawlerino::RoomInfo _RoomInfo{};
};
