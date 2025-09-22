// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/FirstPersonPawn.h"

// Sets default values
AFirstPersonPawn::AFirstPersonPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	check(Camera != nullptr);
	Camera->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFirstPersonPawn::BeginPlay()
{
	Super::BeginPlay();

	_GameState = GetWorld()->GetGameState<ACrawlerGameState>();

	_TerrainPos = _GameState->GetDungeonGrid().StartPos();
	SetActorLocation(FVector{
		(double)_TerrainPos.X * _PawnMovementDelta, (double)_TerrainPos.Y * _PawnMovementDelta, GetActorLocation().Z
	});
	_Facing = Direction::North;
	
	_PlayerController = Cast<APlayerController>(GetController());
	_PlayerController->SetShowMouseCursor(true);
}

void AFirstPersonPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AFirstPersonPawn::UnPossessed()
{
	Super::UnPossessed();

	if (_IsWalking) this->FinalizeAnimation(ActionType::Moving);
	if (_IsRotating) this->FinalizeAnimation(ActionType::Look);
}

void AFirstPersonPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);

	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFirstPersonPawn::MoveCharacter);
	// Rigid look left/right input
	Input->BindAction(LookLeftAction, ETriggerEvent::Started, this, &AFirstPersonPawn::LookLeftCharacter);
	Input->BindAction(LookRightAction, ETriggerEvent::Started, this, &AFirstPersonPawn::LookRightCharacter);

	// Mouse free look input
	Input->BindAction(FreeLookButtonAction, ETriggerEvent::Started, this, &AFirstPersonPawn::FreeLookButtonInput);
	Input->BindAction(FreeLookButtonAction, ETriggerEvent::Completed, this, &AFirstPersonPawn::FreeLookButtonInput);

	_PlayerController = Cast<APlayerController>(GetController());
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
		_PlayerController->GetLocalPlayer());
	Subsystem->ClearAllMappings();
	Subsystem->AddMappingContext(InputMapping, 0);
}

// Called every frame
void AFirstPersonPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float Time = GetWorld()->GetTimeSeconds();

	if (_IsWalking)
	{
		float AnimTime = Time - _WalkingStart;
		float AnimProgress = AnimTime / _Config.AnimTime;
		float EasedProgress = Crawlerino::Utils::EaseInOutCirc(AnimProgress);

		AnimateMove(EasedProgress);

		if (AnimTime > _Config.AnimTime)
		{
			FinalizeAnimation(Moving);
		}
	}

	if (_IsRotating)
	{
		float AnimTime = Time - _RotatingStart;
		float AnimProgress = AnimTime / _Config.AnimTime;
		float EasedProgress = Crawlerino::Utils::EaseInOutCirc(AnimProgress);

		AnimateLook(EasedProgress);

		if (AnimTime > _Config.AnimTime)
		{
			FinalizeAnimation(Look);
		}
	}

	if (_IsLookingAround && !_IsRotating)
	{
		FVector2D Delta = FVector2D::ZeroVector;
		UE::Geometry::FVector2i ViewportSize = UE::Geometry::FVector2i::Zero();
		_PlayerController->GetInputMouseDelta(Delta.X, Delta.Y);
		_PlayerController->GetViewportSize(ViewportSize.X, ViewportSize.Y);

		FVector2D NormalizedDelta = FVector2D{Delta.X / ViewportSize.X, Delta.Y / ViewportSize.Y};
		UE_LOG(LogTemp, Display, TEXT("Normalized delta : X [%f]; Y[%f]"), NormalizedDelta.X, NormalizedDelta.Y);

		auto Rotator = GetActorRotation();
		Rotator.Pitch = FMath::Clamp(Rotator.Pitch + NormalizedDelta.Y * 360.0f * _Config.SensibilityY, -75.0f, 75.0f);
		Rotator.Yaw += NormalizedDelta.X * 360 * _Config.SensibilityX;
		if (Rotator.Yaw < 0.0f) Rotator.Yaw += 360.0f;
		SetActorRotation(Rotator);

		float DirAngle = Crawlerino::Utils::ClampAngle(Rotator.Yaw);
		_Facing = Crawlerino::Utils::DirFromAngle(DirAngle);
	}
}

void AFirstPersonPawn::StartActionAnimation(ActionType Type)
{
	switch (Type)
	{
	case Moving:
		_IsWalking = true;
		_WalkingStart = GetWorld()->GetTimeSeconds();
		break;
	case Look:
		_IsRotating = true;
		_RotatingStart = GetWorld()->GetTimeSeconds();
		break;
	default:
		break;
	}
}

void AFirstPersonPawn::AnimateMove(float Progress)
{
	FVector ActorLoc = GetActorLocation();

	FVector NewLocation = FVector{
		ActorLoc.X + ((_TargetPosition.X - ActorLoc.X) * Progress),
		ActorLoc.Y + ((_TargetPosition.Y - ActorLoc.Y) * Progress),
		ActorLoc.Z
	};
	SetActorLocation(NewLocation);
}

void AFirstPersonPawn::AnimateLook(float Progress)
{
	FQuat ActorRot = FQuat::Slerp(GetActorRotation().Quaternion(), _TargetRotation.Quaternion(), Progress);
	SetActorRotation(ActorRot);
}

void AFirstPersonPawn::FinalizeAnimation(ActionType Action)
{
	switch (Action)
	{
	case Moving:
		FVector FinalLoc = FVector{_TargetPosition.X, _TargetPosition.Y, GetActorLocation().Z};
		SetActorLocation(FinalLoc);

		_IsWalking = false;
		break;
	case Look:
		SetActorRotation(_TargetRotation);

		_IsRotating = false;

		OnRotationChanged.Broadcast(_Facing);
		break;
	default:
		break;
	}
}

FFirstPersonPawnConfig AFirstPersonPawn::GetConfiguration() const
{
	return _Config;
}

void AFirstPersonPawn::SetConfiguration(const FFirstPersonPawnConfig& NewConfig)
{
	_Config = NewConfig;
	UE_LOG(LogTemp, Display, TEXT("AnimTime[%f];SensX[%f];SensY[%f]"), _Config.AnimTime, _Config.SensibilityX, _Config.SensibilityY);
}

void AFirstPersonPawn::MoveCharacter(const FInputActionInstance& Instance)
{
	if (_IsWalking) return;

	FVector2D InputDir = Instance.GetValue().Get<FVector2D>();

	// Modify input depending on current direction of the player
	switch (_Facing)
	{
	float tmp;
	case Direction::North:
		break;
	case Direction::West:
		tmp = InputDir.Y;
		InputDir.Y = InputDir.X;
		InputDir.X = -tmp;
		break;
	case Direction::South:
		InputDir.X *= -1;
		InputDir.Y *= -1;
		break;
	case Direction::East:
		tmp = InputDir.Y;
		InputDir.Y = -InputDir.X;
		InputDir.X = tmp;
	}

	bool IsXInput = abs(InputDir.X) > 0.0f;
	int Sign = IsXInput ? InputDir.X > 0.0f ? 1 : -1 : InputDir.Y > 0.0f ? 1 : -1;
	FVector3d Pos = GetActorLocation();
	FDungeonPos TargetPos = IsXInput
		                       ? FDungeonPos(_TerrainPos.X + Sign, _TerrainPos.Y)
		                       : FDungeonPos(_TerrainPos.X, _TerrainPos.Y + Sign);

	if (_GameState->IsWalkable(TargetPos))
	{
		if (IsXInput)
		{
			_TerrainPos.X += Sign;
			Pos.X += Sign * _PawnMovementDelta;
		}
		else
		{
			_TerrainPos.Y += Sign;
			Pos.Y += Sign * _PawnMovementDelta;
		}

		_TargetPosition = FVector2D{Pos};
		StartActionAnimation(ActionType::Moving);
	}
}

void AFirstPersonPawn::LookLeftCharacter(const FInputActionInstance& Instance)
{
	if (_IsRotating || _IsLookingAround) return;

	_Facing = (Direction)((int)_Facing > 0 ? ((int)_Facing - 1) : 3);

	FRotator Rotator = GetActorRotation();
	Rotator.Yaw = Crawlerino::GetYawFromDirection(_Facing);

	UE_LOG(LogTemp, Display, TEXT("Look Left, Result Yaw = %f"), Rotator.Yaw);

	_TargetRotation = Rotator;
	StartActionAnimation(ActionType::Look);
}

void AFirstPersonPawn::LookRightCharacter(const FInputActionInstance& Instance)
{
	if (_IsRotating || _IsLookingAround) return;

	_Facing = (Direction)(((int)_Facing + 1) % 4);

	FRotator Rotator = GetActorRotation();
	Rotator.Yaw = Crawlerino::GetYawFromDirection(_Facing);

	UE_LOG(LogTemp, Display, TEXT("Look Right, Result Yaw = %f"), Rotator.Yaw);

	_TargetRotation = Rotator;
	StartActionAnimation(ActionType::Look);
}

void AFirstPersonPawn::FreeLookButtonInput(const FInputActionInstance& Instance)
{
	ETriggerEvent Event = Instance.GetTriggerEvent();

	if (Event == ETriggerEvent::Started)
	{
		_IsLookingAround = true;

		// Désactiver l'affichage du curseur
	    _PlayerController->bShowMouseCursor = false;

	    // Passer en mode "Game Only"
	    FInputModeGameOnly InputMode;
		InputMode.SetConsumeCaptureMouseDown(true);
	    _PlayerController->SetInputMode(InputMode);
		// _PlayerController->Cursor
	}
	else if (Event == ETriggerEvent::Completed)
	{
		_IsLookingAround = false;

		_TargetRotation = GetActorRotation();
		_TargetRotation.Pitch = 0.0f;
		_TargetRotation.Yaw = Crawlerino::GetYawFromDirection(_Facing);

		StartActionAnimation(ActionType::Look);

		// Activer l'affichage du curseur
	    _PlayerController->bShowMouseCursor = true;

	    // Préparer le mode d’input avec confinement
	    FInputModeGameAndUI InputMode;
	    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // Confine le curseur
	    InputMode.SetHideCursorDuringCapture(false);

		FVector2D ViewportSize;
	    GEngine->GameViewport->GetViewportSize(ViewportSize);
	    int32 CenterX = ViewportSize.X / 2;
	    int32 CenterY = ViewportSize.Y / 2;
	    _PlayerController->SetMouseLocation(CenterX, CenterY); // Repositionne le curseur

	    _PlayerController->SetInputMode(InputMode);
	}
}
