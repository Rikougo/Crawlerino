// Fill out your copyright notice in the Description page of Project Settings.


#include "Terrain/CrawlerTerrainGenerator.h"


// Sets default values
ACrawlerTerrainGenerator::ACrawlerTerrainGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	_Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("FloorMesh"));
	_Mesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACrawlerTerrainGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	_GameState = GetWorld()->GetGameState<ACrawlerGameState>();

	GenerateTerrain();
	InitTexture();
}

void ACrawlerTerrainGenerator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	UpdateTexture();
}

void ACrawlerTerrainGenerator::SetPlayerPosition(const FDungeonPos& Pos)
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

void ACrawlerTerrainGenerator::GenerateTerrain() const
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<int32> Triangles;
	TArray<FVector2D> UVs;

	UE_LOG(LogTemp, Display, TEXT("Create terrain of size %d %d"), _GameState->GetDungeonGrid().Width(), _GameState->GetDungeonGrid().Height());

	auto CellSize = _GameState->CellSize;
	
	// Create ground mesh
	for (int yIdx = 0; yIdx < _GameState->GetDungeonGrid().Height(); yIdx++)
	{
		for (int xIdx = 0; xIdx < _GameState->GetDungeonGrid().Width(); xIdx++)
		{
			int IsActive = _GameState->GetDungeonGrid().GetValue(xIdx, yIdx);
			if (IsActive > 0)
			{
				UE_LOG(LogTemp, Display, TEXT("Create terrain %s %s"), *FString::FromInt(yIdx), *FString::FromInt(xIdx));

				int VertexOffset = Vertices.Num();
				float HalfSize = CellSize / 2;
				Vertices.Add(FVector(xIdx * CellSize - HalfSize, yIdx * CellSize + HalfSize, 0.0f)); // (-HalfSize, +HalfSize)
				Vertices.Add(FVector(xIdx * CellSize + HalfSize, yIdx * CellSize + HalfSize, 0.0f)); // (+HalfSize, +HalfSize)
				Vertices.Add(FVector(xIdx * CellSize + HalfSize, yIdx * CellSize - HalfSize, 0.0f)); // (+HalfSize, -HalfSize)
				Vertices.Add(FVector(xIdx * CellSize - HalfSize, yIdx * CellSize - HalfSize, 0.0f)); // (-HalfSize, -HalfSize)

				// DrawCircle(GetWorld(), FVector(xIdx * CellSize, yIdx * CellSize, 0.0f), FVector::ForwardVector, FVector::RightVector, FColor::Red, 10.0f, 32, true, 10000, 0, 1);

				for (int i = 0; i < 4; i++)
				{
					Normals.Add(FVector(0.0f, 0.0f, 1.0f));
				}

				// First triangle
				Triangles.Add(VertexOffset + 0);
				Triangles.Add(VertexOffset + 1);
				Triangles.Add(VertexOffset + 2);

				// Second triangle
				Triangles.Add(VertexOffset + 0);
				Triangles.Add(VertexOffset + 2);
				Triangles.Add(VertexOffset + 3);

				// UVs
				UVs.Add(FVector2D(0.0f, 1.0f));
				UVs.Add(FVector2D(1.0f, 1.0f));
				UVs.Add(FVector2D(1.0f, 0.0f));
				UVs.Add(FVector2D(0.0f, 0.0f));

				TryGenerateWall(Vertices, Normals, Triangles, UVs, xIdx, yIdx, North);
				TryGenerateWall(Vertices, Normals, Triangles, UVs, xIdx, yIdx, East);
				TryGenerateWall(Vertices, Normals, Triangles, UVs, xIdx, yIdx, South);
				TryGenerateWall(Vertices, Normals, Triangles, UVs, xIdx, yIdx, West);
			}
		}

	}
	
	_Mesh->ClearAllMeshSections();
	_Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, TArray<FLinearColor>(), TArray<FProcMeshTangent>(), true);
}

void ACrawlerTerrainGenerator::TryGenerateWall(TArray<FVector>& Vertices,
	TArray<FVector>& Normals,
	TArray<int32>& Triangles,
	TArray<FVector2D>& UVs,
	int X, int Y, FDirection Direction) const
{
	int nX = X, nY = Y;
	FVector Normal;
	switch (Direction)
	{
	case North:
		Normal = -FVector::RightVector;
		nY++;
		break;
	case East:
		Normal = -FVector::ForwardVector;
		nX++;
		break;
	case South:
		Normal = FVector::RightVector;
		nY--;
		break;
	case West:
		Normal = FVector::ForwardVector;
		nX--;
		break;
	}

	bool IsValidCoords = _GameState->GetDungeonGrid().IsValidPosition(FDungeonPos{nX, nY});
	float Value = IsValidCoords ? _GameState->GetDungeonGrid().GetValue(nX, nY) : 0.0f;

	auto CellSize = _GameState->CellSize;
	FVector OriginCenter = FVector(X * CellSize, Y * CellSize, 0.0f);

	// Create walls if empty case 
	if (!IsValidCoords || Value == 0.0f)
	{
		int VertexOffset = Vertices.Num();
		float HalfSize = CellSize / 2;
		
		if (Direction == North || Direction == South)
		{
			float CellY = nY * CellSize;
			FVector MiddlePosition = FVector(OriginCenter.X, OriginCenter.Y + (CellY - OriginCenter.Y) / 2.0f, 0.0f);

			Vertices.Add(FVector(MiddlePosition.X - HalfSize, MiddlePosition.Y, CellSize * 2.0f));
			Vertices.Add(FVector(MiddlePosition.X + HalfSize, MiddlePosition.Y, CellSize * 2.0f));
			Vertices.Add(FVector(MiddlePosition.X + HalfSize, MiddlePosition.Y, 0.0f));
			Vertices.Add(FVector(MiddlePosition.X - HalfSize, MiddlePosition.Y, 0.0f));

		} else
		{
			float CellX = nX * CellSize;
			FVector MiddlePosition = FVector(OriginCenter.X + (CellX- OriginCenter.X) / 2.0f, OriginCenter.Y, CellSize);

			Vertices.Add(FVector(MiddlePosition.X, MiddlePosition.Y - HalfSize, CellSize * 2.0f));
			Vertices.Add(FVector(MiddlePosition.X, MiddlePosition.Y + HalfSize, CellSize * 2.0f));
			Vertices.Add(FVector(MiddlePosition.X, MiddlePosition.Y + HalfSize, 0.0f));
			Vertices.Add(FVector(MiddlePosition.X, MiddlePosition.Y - HalfSize, 0.0f));
		}
		
		for (int i = 0; i < 4; i++)
		{
			Normals.Add(Normal);
		}

		if (Direction == North || Direction == West)
		{
			// First triangle
			Triangles.Add(VertexOffset + 0);
			Triangles.Add(VertexOffset + 1);
			Triangles.Add(VertexOffset + 2);
			
			// Second triangle
			Triangles.Add(VertexOffset + 0);
			Triangles.Add(VertexOffset + 2);
			Triangles.Add(VertexOffset + 3);
			
			// UVs
			UVs.Add(FVector2D(0.0f, 2.0f));
			UVs.Add(FVector2D(1.0f, 2.0f));
			UVs.Add(FVector2D(1.0f, 0.0f));
			UVs.Add(FVector2D(0.0f, 0.0f));
		} else
		{
			// First triangle
			Triangles.Add(VertexOffset + 1);
			Triangles.Add(VertexOffset + 0);
			Triangles.Add(VertexOffset + 3);
			
			// Second triangle
			Triangles.Add(VertexOffset + 1);
			Triangles.Add(VertexOffset + 3);
			Triangles.Add(VertexOffset + 2);

			// UVs
			UVs.Add(FVector2D(1.0f, 2.0f));
			UVs.Add(FVector2D(0.0f, 2.0f));
			UVs.Add(FVector2D(0.0f, 0.0f));
			UVs.Add(FVector2D(1.0f, 0.0f));
		}
	}
}

void ACrawlerTerrainGenerator::InitTexture()
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

void ACrawlerTerrainGenerator::UpdateTexture(bool bFreeData) const
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

