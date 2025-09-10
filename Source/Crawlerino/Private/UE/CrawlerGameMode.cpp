// Fill out your copyright notice in the Description page of Project Settings.


#include "UE/CrawlerGameMode.h"

void ACrawlerGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UE_LOG(LogTemp, Log, TEXT("Initializing game (loading level %s)"), *MapName);
}
