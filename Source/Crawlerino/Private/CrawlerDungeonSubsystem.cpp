// Fill out your copyright notice in the Description page of Project Settings.


#include "CrawlerDungeonSubsystem.h"

#include "IntVectorTypes.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"

void UCrawlerDungeonSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	
	_DungeonGrid = std::make_unique<Crawlerino::DungeonGrid>(32, 32);

	UE_LOG(LogTemp, Warning, TEXT("UCrawlerDungeonSubsystem : CREATING DUNGEON LAYOUT"));

	Crawlerino::DungeonGrid::GenerateTerrain(*_DungeonGrid, _RoomsToPlace);
	
	InitTexture();
}

void UCrawlerDungeonSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdateTexture();
}

void UCrawlerDungeonSubsystem::SetPlayerPosition(int X, int Y)
{
	check(X >= 0 && X < _DungeonGrid->Width() && Y >= 0 && Y < _DungeonGrid->Height());

	for (int yIdx = 0; yIdx < _DungeonGrid->Height(); yIdx++)
	{
		for (int xIdx = 0; xIdx < _DungeonGrid->Width(); xIdx++)
		{
			int i = yIdx * _DungeonGrid->Width() + xIdx;

			int value = _DungeonGrid->GetValue(xIdx, yIdx);

			// Set pixel to red
			if (X == xIdx && Y == yIdx)
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
	_TextureTotalPixels = _DungeonGrid->Width() * _DungeonGrid->Height();
	_TextureDataSize = _DungeonGrid->Width() * _DungeonGrid->Height() * 4; // Pixels times the data contained by each pixel
	_TextureDataSqrtSize = _DungeonGrid->Width()  * 4;
	
	_TextureData = new uint8[_TextureDataSize];

	// Init texture data
	for (int i = 0; i < (_DungeonGrid->Width() * _DungeonGrid->Height() * 4); i+=4)
	{
		int GridIdx = i / 4;
		int TileValue = _DungeonGrid->GetValue(GridIdx % _DungeonGrid->Width(), GridIdx / _DungeonGrid->Width());

		Color Color = RoomColors[TileValue-1];
		
		_TextureData[i + 0] = Color.B; // Blue
		_TextureData[i + 1] = Color.G; // Green
		_TextureData[i + 2] = Color.R; // Red
		_TextureData[i + 3] = 255; // Alpha
	}

	TConstArrayView64<uint8> View = TArrayView<uint8>(_TextureData, _TextureDataSize);
	
	_DungeonTexture = UTexture2D::CreateTransient(_DungeonGrid->Width(), _DungeonGrid->Height(), PF_B8G8R8A8, TEXT("Grid"), View);
	_DungeonTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	_DungeonTexture->SRGB = 0;
	_DungeonTexture->Filter = TextureFilter::TF_Nearest;
	_DungeonTexture->AddToRoot();
	_DungeonTexture->UpdateResource();

	_DungeonTextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, _DungeonGrid->Width(), _DungeonGrid->Height());
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
