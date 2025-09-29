// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "IntVectorTypes.h"

#include "GameModes/CrawlerGameState.h"
#include "Terrain/Props/Interactable.h"

#include "FirstPersonPawn.generated.h"

 // Delegate signature

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRotationChangedSignature, FDirection, NewDirection);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USphereComponent* SphereCollider;
	
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input")
	UInputAction* InteractAction;
public:
	UPROPERTY(BlueprintAssignable)
	FOnRotationChangedSignature OnRotationChanged;
public:
	// Sets default values for this pawn's properties
	AFirstPersonPawn();

protected:
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintPure)
	FFirstPersonPawnConfig GetConfiguration() const;

	UFUNCTION(BlueprintCallable)
	void SetConfiguration(const FFirstPersonPawnConfig& NewConfig);
public:
	UFUNCTION(BlueprintPure)
	FDungeonPos GetPos() const { return _TerrainPos; }
	UFUNCTION(BlueprintPure)
	int GetCurrentRoom() const { return _GameState->GetDungeonGrid().GetRoomIndex(_TerrainPos);};
	UFUNCTION(BlueprintPure)
	bool IsMoving() const { return _IsWalking; }
	UFUNCTION(BlueprintPure)
	bool IsRotating() const { return _IsRotating; }
	UFUNCTION(BlueprintPure)
	bool IsLooking() const { return _IsLookingAround; }
	UFUNCTION(BlueprintPure)
	FDirection GetFacingDirection() const { return _Facing; }
private:
	void StartActionAnimation(ActionType Type);
	void AnimateMove(float Progress);
	void AnimateLook(float Progress);
	void FinalizeAnimation(ActionType Action);

	UFUNCTION()
	void OnBeginOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnEndOverlap(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	void MoveCharacter(const FInputActionInstance& Instance);
	void LookLeftCharacter(const FInputActionInstance& Instance);
	void LookRightCharacter(const FInputActionInstance& Instance);
	void FreeLookButtonInput(const FInputActionInstance& Instance);
	void OnInteract(const FInputActionInstance& Instance);
private:
	// Config
	FFirstPersonPawnConfig _Config {
		0.35f,
		1.0f,
		1.0f
	};
	
	// References
	ACrawlerGameState* _GameState;
	APlayerController* _PlayerController;

	// Positional data
	FDirection _Facing;
	FDungeonPos _TerrainPos;

	std::vector<TScriptInterface<IInteractable>> _Interactables;

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


