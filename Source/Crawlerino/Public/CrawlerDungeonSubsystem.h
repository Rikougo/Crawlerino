// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "DungeonGrid.h"
#include "IntVectorTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrawlerDungeonSubsystem.generated.h"

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
 * 
 */
UCLASS()
class CRAWLERINO_API UCrawlerDungeonSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

private:
	std::unique_ptr<Crawlerino::DungeonGrid> _DungeonGrid;

	const std::vector<Crawlerino::RoomDescriptor> _RoomsToPlace {
		Crawlerino::RoomDescriptor{2,2},
		Crawlerino::RoomDescriptor{2,2},
		Crawlerino::RoomDescriptor{2,2},
		Crawlerino::RoomDescriptor{4,4},
		Crawlerino::RoomDescriptor{4,4},
		Crawlerino::RoomDescriptor{4,4},
		Crawlerino::RoomDescriptor{10,10}
	};
	
private:
	UTexture2D* _DungeonTexture;
    FUpdateTextureRegion2D* _DungeonTextureRegion; // Update Region Struct
	

	uint8* _TextureData; //Array that contains the Pixel Values for the Texture
    uint32 _TextureDataSize; // Total Bytes of Texture Data
    uint32 _TextureDataSqrtSize; // Texture Data Sqrt Size
    uint32 _TextureTotalPixels; // Total Count of Pixels in Texture
private:

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// FTickableGameObject implementation Begin
	virtual UWorld* GetTickableGameObjectWorld() const override { return GetWorld(); }
	virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Always; }
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return TStatId(); }
	// FTickableGameObject implementation End

public: // Grid management 
	DungeonPos GetStartPos() const { return _DungeonGrid->StartPos(); }

	Crawlerino::DungeonGrid& GetDungeonGrid() const { return *_DungeonGrid; }
public: // Texture management 
	UFUNCTION(BlueprintCallable)
	UTexture2D* GetRoomTexture() const { return _DungeonTexture; }

	UFUNCTION(BlueprintCallable)
	void SetPlayerPosition(int X, int Y);
private:
	void InitTexture();
	//Update Texture Object from Texture Data
    void UpdateTexture(bool bFreeData = false) const;
};
