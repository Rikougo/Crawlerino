// Fill out your copyright notice in the Description page of Project Settings.


#include "UE/CrawlerGameState.h"

ACrawlerGameState::ACrawlerGameState()
{
	_DungeonGrid = std::make_unique<Crawlerino::DungeonGrid>(DungeonSize.X, DungeonSize.Y);
	
	Crawlerino::DungeonGrid::GenerateTerrain(*_DungeonGrid, _RoomsToPlace);
}

void ACrawlerGameState::BeginPlay()
{
	Super::BeginPlay();
}

void ACrawlerGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
