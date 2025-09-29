// Fill out your copyright notice in the Description page of Project Settings.


#include "UE/CrawlerDungeonSubsystem.h"

#include "IntVectorTypes.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"

#include "GameModes/CrawlerGameState.h"

void UCrawlerDungeonSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCrawlerDungeonSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UCrawlerDungeonSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	InitTexture();
}

void UCrawlerDungeonSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateTexture();
}

void UCrawlerDungeonSubsystem::SetPlayerPosition(const FDungeonPos& Pos)
{
	Crawlerino::DungeonGrid& Grid = GetWorld()->GetGameState<ACrawlerGameState>()->GetDungeonGrid(); 
	
	check(Pos.X >= 0 && Pos.X < Grid.Width() && Pos.Y >= 0 && Pos.Y < Grid.Height());

	for (int yIdx = 0; yIdx < Grid.Height(); yIdx++)
	{
		for (int xIdx = 0; xIdx < Grid.Width(); xIdx++)
		{
			int i = yIdx * Grid.Width() + xIdx;

			int value = Grid.GetValue(xIdx, yIdx);

			// Set pixel to red
			if (Pos.X == xIdx && Pos.Y == yIdx)
			{
				_TextureData[i * 4 + 0] = 0;
				_TextureData[i * 4 + 1] = 0;
				_TextureData[i * 4 + 2] = 255;
				_TextureData[i * 4 + 3] = 255;
			} else
			{
				Color Color = RoomColors[value-1];
				
				_TextureData[i * 4 + 0] = Color.B;
				_TextureData[i * 4 + 1] = Color.G;
				_TextureData[i * 4 + 2] = Color.R;
				_TextureData[i * 4 + 3] = 255;
			}
		}
	}
}

void UCrawlerDungeonSubsystem::InitTexture()
{
	Crawlerino::DungeonGrid& Grid = GetWorld()->GetGameState<ACrawlerGameState>()->GetDungeonGrid(); 
	
	_TextureTotalPixels = Grid.Width() * Grid.Height();
	_TextureDataSize = Grid.Width() * Grid.Height() * 4; // Pixels times the data contained by each pixel
	_TextureDataSqrtSize = Grid.Width()  * 4;
	
	_TextureData = new uint8[_TextureDataSize];

	// Init texture data
	for (int i = 0; i < (Grid.Width() * Grid.Height() * 4); i+=4)
	{
		int GridIdx = i / 4;
		int TileValue = Grid.GetValue(GridIdx % Grid.Width(), GridIdx / Grid.Width());

		Color Color = RoomColors[TileValue-1];
		
		_TextureData[i + 0] = Color.B; // Blue
		_TextureData[i + 1] = Color.G; // Green
		_TextureData[i + 2] = Color.R; // Red
		_TextureData[i + 3] = 255; // Alpha
	}

	TConstArrayView64<uint8> View = TArrayView<uint8>(_TextureData, _TextureDataSize);
	
	_DungeonTexture = UTexture2D::CreateTransient(Grid.Width(), Grid.Height(), PF_B8G8R8A8, TEXT("Grid"), View);
	_DungeonTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	_DungeonTexture->SRGB = 0;
	_DungeonTexture->Filter = TextureFilter::TF_Nearest;
	_DungeonTexture->AddToRoot();
	_DungeonTexture->UpdateResource();

	_DungeonTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, Grid.Width(), Grid.Height());

	OnTextureReady.Broadcast();
}

void UCrawlerDungeonSubsystem::UpdateTexture(bool bFreeData) const
{
	if (_DungeonTexture == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Dynamic Texture tried to Update before being initialized!"));
        return;
    }

    struct FUpdateTextureRegionsData
    {
        FTexture2DResource* Texture2DResource;
        FRHITexture* TextureRHI;
        int32 MipIndex;
        uint32 NumRegions;
        FUpdateTextureRegion2D* Regions;
        uint32 SrcPitch;
        uint32 SrcBpp;
        uint8* SrcData;
    };

    FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

    UTexture2D* Texture = _DungeonTexture;

    RegionData->Texture2DResource = (FTexture2DResource*)Texture->GetResource();
    RegionData->TextureRHI = RegionData->Texture2DResource->GetTexture2DRHI();
    RegionData->MipIndex = 0;
    RegionData->NumRegions = 1;
    RegionData->Regions = _DungeonTextureRegion;
    RegionData->SrcPitch = _TextureDataSqrtSize;
    RegionData->SrcBpp = 4;
    RegionData->SrcData = _TextureData;

    ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
        [RegionData, bFreeData, Texture](FRHICommandListImmediate& RHICmdList)
        {
            for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
            {
                int32 CurrentFirstMip = Texture->FirstResourceMemMip;
                if (RegionData->TextureRHI && RegionData->MipIndex >= CurrentFirstMip)
                {
                    RHIUpdateTexture2D(
                        RegionData->TextureRHI,
                        RegionData->MipIndex - CurrentFirstMip,
                        RegionData->Regions[RegionIndex],
                        RegionData->SrcPitch,
                        RegionData->SrcData
                        + RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
                        + RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
                    );
                }
            }
            if (bFreeData) {
                FMemory::Free(RegionData->Regions);
                FMemory::Free(RegionData->SrcData);
            }
            delete RegionData;
        });
}
