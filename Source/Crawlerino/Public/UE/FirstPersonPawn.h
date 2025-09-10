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

USTRUCT(BlueprintType)
struct FFirstPersonPawnConfig
{
	GENERATED_USTRUCT_BODY()

public:
	// Control animations
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AnimTime;
	
	// Look settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SensibilityX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SensibilityY;
};

UCLASS()
class CRAWLERINO_API AFirstPersonPawn : public APawn
{
	GENERATED_BODY()
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* FreeLookButtonAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* FreeLookAction;
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

	UFUNCTION(BlueprintPure)
	FFirstPersonPawnConfig GetConfiguration() const;

	UFUNCTION(BlueprintCallable)
	void SetConfiguration(const FFirstPersonPawnConfig& NewConfig);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void AnimateMove(float Progress);
	void AnimateLook(float Progress);
	void FinalizeAnimation(ActionType Action);
	
	UFUNCTION(BlueprintPure, meta=(DeprecatedFunction, DeprecationMessage="Use GetPos() instead"))
	int GetDungeonPosX() const { return _TerrainPos.X; } 

	UFUNCTION(BlueprintPure, meta=(DeprecatedFunction, DeprecationMessage="Use GetPos() instead"))
	int GetDungeonPosY() const { return _TerrainPos.Y; }

	UFUNCTION(BlueprintPure)
	FDungeonPos GetPos() const { return _TerrainPos; }

	UFUNCTION(BlueprintPure)
	int GetCurrentRoom() const { return _Dungeon->GetDungeonGrid().GetRoomIndex(_TerrainPos);};

	UFUNCTION(BlueprintPure)
	bool IsMoving() const { return _IsWalking; }

	UFUNCTION(BlueprintPure)
	bool IsRotating() const { return _IsRotating; }

	UFUNCTION(BlueprintPure)
	bool IsLooking() const { return _IsLookingAround; }

	UFUNCTION(BlueprintPure)
	Direction GetFacingDirection() const { return _Facing; }
private:
	void StartActionAnimation(ActionType Type);
	
	void MoveCharacter(const FInputActionInstance& Instance);
	void LookLeftCharacter(const FInputActionInstance& Instance);
	void LookRightCharacter(const FInputActionInstance& Instance);
	void FreeLookButtonInput(const FInputActionInstance& Instance);
private:
	// Config
	float _PawnMovementDelta = 100.0f;
	FFirstPersonPawnConfig _Config {
		0.35f,
		1.0f,
		1.0f
	};
	
	// References
	UCrawlerDungeonSubsystem* _Dungeon;
	APlayerController* _PlayerController;

	// Positional data
	Direction _Facing;
	FDungeonPos _TerrainPos;

	// Walk animation
	bool _IsWalking;
	float _WalkingStart;
	FVector2D _TargetPosition;

	// Rotate input animation
	bool _IsRotating;
	float _RotatingStart;
	FRotator _TargetRotation;

	// Free look handling
	bool _IsLookingAround;
};


