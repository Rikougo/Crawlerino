// Fill out your copyright notice in the Description page of Project Settings.


#include "UE/CrawlerTerrainGenerator.h"

// Sets default values
ACrawlerTerrainGenerator::ACrawlerTerrainGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	_Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("FloorMesh"));
	_Mesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACrawlerTerrainGenerator::BeginPlay()
{
	Super::BeginPlay();
	
	_Dungeon = GetWorld()->GetSubsystem<UCrawlerDungeonSubsystem>();

	GenerateTerrain();
}

void ACrawlerTerrainGenerator::GenerateTerrain() const
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<int32> Triangles;
	TArray<FVector2D> UVs;
	
	// Create ground mesh
	for (int yIdx = 0; yIdx < _Dungeon->GetDungeonGrid().Height(); yIdx++)
	{
		for (int xIdx = 0; xIdx < _Dungeon->GetDungeonGrid().Width(); xIdx++)
		{
			int IsActive = _Dungeon->GetDungeonGrid().GetValue(xIdx, yIdx);
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
	int X, int Y, Direction Direction) const
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

	bool IsValidCoords = _Dungeon->GetDungeonGrid().IsValidPosition(DungeonPos{nX, nY});
	float Value = IsValidCoords ? _Dungeon->GetDungeonGrid().GetValue(nX, nY) : 0.0f;

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

