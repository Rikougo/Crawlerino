// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"

#include "GameModes/CrawlerGameState.h"

#include "CrawlerTerrainGenerator.generated.h"

/**
 * Create geometry of the grid generated in CrawlerDungeonSubsystem
 */
UCLASS()
class CRAWLERINO_API ACrawlerTerrainGenerator : public AActor
{
	GENERATED_BODY()
public:	
	// Sets default values for this actor's properties
	ACrawlerTerrainGenerator();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:
	void GenerateTerrain() const;

	void TryGenerateWall(TArray<FVector>& Vertices, TArray<FVector>& Normals,
	                     TArray<int32>& Triangles, TArray<FVector2D>& UVs,
	                     int X, int Y, FDirection Direction) const;

private:
	ACrawlerGameState* _GameState;
	
	UProceduralMeshComponent* _Mesh;
};
