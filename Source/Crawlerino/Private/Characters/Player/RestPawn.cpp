// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Player/RestPawn.h"

ARestPawn::ARestPawn()
{
	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	check(Camera != nullptr);
	Camera->SetupAttachment(RootComponent);
}

void ARestPawn::BeginPlay()
{
	Super::BeginPlay();

	_GameState = GetWorld()->GetGameState<ACrawlerGameState>();

	if (_GameState == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not retrieve ACrawlerGameState, should not happen"))
	}
}

void ARestPawn::Init(const FDungeonPos& Pos, const FDirection& Direction)
{
	_Terrainpos = Pos;
	_FacingDirection = Direction;

	SetActorLocation(_GameState->GetWorldPos(_Terrainpos));
	SetActorRotation(FRotator(0, Crawlerino::GetYawFromDirection(_FacingDirection), 0));
}


