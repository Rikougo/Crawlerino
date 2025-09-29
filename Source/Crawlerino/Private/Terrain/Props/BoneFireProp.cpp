// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrain/Props/BoneFireProp.h"

// Sets default values
ABoneFireProp::ABoneFireProp()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoneFireProp::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABoneFireProp::Interact_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Received interact signal"));
}

