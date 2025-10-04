// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "IntVectorTypes.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"

#include "Common/CrawlerEvents.h"
#include "GameModes/CrawlerGameState.h"

#include "CrawlerTerrainGenerator.generated.h"

struct Color
{
	int R,G,B;
};

constexpr Color RoomColors[] = {
	Color{240, 248, 255 }, // Corridors
	Color{138,  43, 226 },
	Color{184, 134,  11 },
	Color{153,  50, 204 },
	Color{255,   0, 255 },
	Color{  0,   0, 128 },
	Color{139,  69,  19 }
};

/**
 * Create geometry of the grid generated in CrawlerDungeonSubsystem
 */
UCLASS()
class CRAWLERINO_API ACrawlerTerrainGenerator : public AActor
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FDungeonTextureReady OnTextureReady;
public:	
	// Sets default values for this actor's properties
	ACrawlerTerrainGenerator();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
public:
	UFUNCTION(BlueprintPure)
	UTexture2D* GetRoomTexture() const { return _DungeonTexture; }
	
	UFUNCTION(BlueprintCallable)
	void SetPlayerPosition(const FDungeonPos& Pos);
private:
	void GenerateTerrain() const;

	void TryGenerateWall(TArray<FVector>& Vertices, TArray<FVector>& Normals,
	                     TArray<int32>& Triangles, TArray<FVector2D>& UVs,
	                     int X, int Y, FDirection Direction) const;

	void InitTexture();
	//Update Texture Object from Texture Data
    void UpdateTexture(bool bFreeData = false) const;
private:
	ACrawlerGameState* _GameState;
	
	UProceduralMeshComponent* _Mesh;
	
	UTexture2D* _DungeonTexture;
    FUpdateTextureRegion2D* _DungeonTextureRegion; // Update Region Struct
	uint8* _TextureData; //Array that contains the Pixel Values for the Texture
    uint32 _TextureDataSize; // Total Bytes of Texture Data
    uint32 _TextureDataSqrtSize; // Texture Data Sqrt Size
    uint32 _TextureTotalPixels; // Total Count of Pixels in Texture
};
