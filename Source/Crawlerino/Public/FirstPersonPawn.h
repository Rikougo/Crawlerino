// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CrawlerTerrainGenerator.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "IntVectorTypes.h"
#include "FirstPersonPawn.generated.h"

 // Delegate signature

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotationChangedSignature, Direction, NewDirection);

class UInputMappingContext;

enum ActionType
{
	None  = 0,
	Look  = 1,
	Moving  = 2
};

UCLASS()
class CRAWLERINO_API AFirstPersonPawn : public APawn
{
	GENERATED_BODY()
private:
	const int PawnMovementDelta = 100.0f;
	
	const float AnimationTime = 0.35f;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* Camera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* InputMapping;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* LookLeftAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* LookRightAction;
public:
	// Function signature
	UPROPERTY(BlueprintAssignable)
	FOnRotationChangedSignature OnRotationChanged;
public:
	// Sets default values for this pawn's properties
	AFirstPersonPawn();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void AnimateMove(float Progress);
	void AnimateLook(float Progress);
	void FinalizeAnimation(ActionType Action);

	UFUNCTION(BlueprintPure)
	int GetDungeonPosX() const { return _TerrainPos.X; } 

	UFUNCTION(BlueprintPure)
	int GetDungeonPosY() const { return _TerrainPos.Y; } 

	UFUNCTION(BlueprintPure)
	int GetCurrentRoom() const { return _Dungeon->GetDungeonGrid().GetRoomIndex(_TerrainPos);};

	UFUNCTION(BlueprintPure)
	Direction GetFacingDirection() const { return _Facing; }
private:
	void StartActionAnimation(ActionType Type);
	
	void MoveCharacter(const FInputActionInstance& Instance);
	void LookLeftCharacter(const FInputActionInstance& Instance);
	void LookRightCharacter(const FInputActionInstance& Instance);
private:
	UCrawlerDungeonSubsystem* _Dungeon;
	Direction _Facing;
	DungeonPos _TerrainPos;
	
	
	bool _IsWalking;
	float _WalkingStart;
	FVector2D _TargetPosition;

	bool _IsRotating;
	float _RotatingStart;
	FRotator _TargetRotation;
};


