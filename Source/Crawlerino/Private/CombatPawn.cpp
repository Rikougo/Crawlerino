// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPawn.h"

FVector ComputeCameraPositionForBoundingBoxOriented(
    const FVector& BoxMin,
    const FVector& BoxMax,
    float FovDegrees,
    float AspectRatio,
    const FVector& CameraForward
)
{
    // 1. Compute the center of the bounding box
    FVector Center = (BoxMin + BoxMax) * 0.5f;

    // 2. Compute the size of the bounding box
    FVector BoxSize = BoxMax - BoxMin;
    float BoxHeight = BoxSize.Y;
    float BoxWidth = BoxSize.X;

    // 3. Compute the required distance to fit the box according to the vertical FOV
    float FovRadians = FMath::DegreesToRadians(FovDegrees);
    float DistanceY = (BoxHeight * 0.5f) / FMath::Tan(FovRadians * 0.5f);

    // 4. Take the aspect ratio into account for the width
    float HorizontalFov = 2.0f * FMath::Atan(FMath::Tan(FovRadians * 0.5f) * AspectRatio);
    float DistanceX = (BoxWidth * 0.5f) / FMath::Tan(HorizontalFov * 0.5f);

    // 5. Take the largest distance to ensure the whole box fits in the frame
    float Distance = FMath::Max(DistanceX, DistanceY);

    // 6. Place the camera in the direction of CameraForward, at the computed distance from the center
    FVector CameraPosition = Center - CameraForward * Distance;

    return CameraPosition;
}

#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathUtility.h"

// Calcule la position idéale de la caméra pour englober la bounding box définie par BoxMin et BoxMax
FVector CalculateCameraPositionToFitBoundingBox(
    const FVector& BoxMin,
    const FVector& BoxMax,
    const FRotator& CameraRotation,
    UWorld* World,
    float HorizontalFOVDegrees,
    float AspectRatio)
{
    FVector BoxCenter = (BoxMin + BoxMax) * 0.5f;
	
    FVector Vertices[8];
    Vertices[0] = FVector(BoxMin.X, BoxMin.Y, BoxMin.Z);
    Vertices[1] = FVector(BoxMin.X, BoxMin.Y, BoxMax.Z);
    Vertices[2] = FVector(BoxMin.X, BoxMax.Y, BoxMin.Z);
    Vertices[3] = FVector(BoxMin.X, BoxMax.Y, BoxMax.Z);
    Vertices[4] = FVector(BoxMax.X, BoxMin.Y, BoxMin.Z);
    Vertices[5] = FVector(BoxMax.X, BoxMin.Y, BoxMax.Z);
    Vertices[6] = FVector(BoxMax.X, BoxMax.Y, BoxMin.Z);
    Vertices[7] = FVector(BoxMax.X, BoxMax.Y, BoxMax.Z);
	
    FRotationMatrix CamRotMatrix(CameraRotation);
    FMatrix InvRotMatrix = CamRotMatrix.GetMatrixWithoutScale().GetTransposed(); // Inverse pour matrice orthonormale
	
    float MaxDX = 0.f;
    float MaxDY = 0.f;
    for (int i = 0; i < 8; ++i)
    {
        FVector LocalVertex = InvRotMatrix.TransformVector(Vertices[i] - BoxCenter);
    	DrawDebugCircle(World, LocalVertex, 5.0f, 8.0f, FColor::Yellow, true, -1, 0, 1.0f);
        MaxDX = FMath::Max(MaxDX, FMath::Abs(LocalVertex.X));
        MaxDY = FMath::Max(MaxDY, FMath::Abs(LocalVertex.Y));
    }
	
    float HorizontalFOVRad = FMath::DegreesToRadians(HorizontalFOVDegrees);
    float VerticalFOVRad = 2.f * FMath::Atan(FMath::Tan(HorizontalFOVRad / 2.f) / AspectRatio);
	
    float DistX = MaxDX / FMath::Tan(HorizontalFOVRad / 2.f);
    float DistY = MaxDY / FMath::Tan(VerticalFOVRad / 2.f);
	
    float Distance = FMath::Max(DistX, DistY) + 100.0f;
	
    FVector CameraForward = CameraRotation.Vector();
    FVector CameraPosition = BoxCenter - CameraForward * Distance;

    return CameraPosition;
}



// Sets default values
ACombatPawn::ACombatPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	check(Camera != nullptr);
	Camera->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACombatPawn::BeginPlay()
{
	Super::BeginPlay();
	
	_DungeonSubsystem = GetWorld()->GetSubsystem<UCrawlerDungeonSubsystem>();
}

// Called every frame
void ACombatPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (_IsAnimating)
	{
		float Elapsed = GetWorld()->GetTimeSeconds() - _AnimationTime;
		float Progress = Elapsed / TransitionTime;

		if (Progress >= 1.0f)
		{
			EndTransition();
		} else
		{
			float ModifiedProgress = EaseOutCirc(Progress);

			FVector Pos = _CameraStartPos + (_CameraTargetPos - _CameraStartPos) * Progress;
			FRotator Rotator = FQuat::Slerp(_CameraStartRot.Quaternion(), _CameraTargetRot.Quaternion(), ModifiedProgress).Rotator();

			Camera->SetWorldLocation(Pos);
			Camera->SetWorldRotation(Rotator);
		}
	}


}

// Called to bind functionality to input
void ACombatPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACombatPawn::InitAtRoom(int PosX, int PosY, Direction Direction, FVector CameraStartPos, FRotator CameraStartRot)
{
	Crawlerino::DungeonGrid& Grid = _DungeonSubsystem->GetDungeonGrid();

	Crawlerino::RoomInfo RoomInfo;
	std::vector<DungeonPos> TilePositions{};
	if (!Grid.GrabRoomInfo({PosX, PosY}, RoomInfo, TilePositions, 4)) return;

	/*for (auto TilePos : TilePositions)
	{
		FVector Pos{TilePos.X * 100.0f, TilePos.Y * 100.0f, 0.0f};
		DrawDebugCircle(GetWorld(), Pos, 25.0f, 8.0f, FColor::Green, true, -1, 0, 5.0f);
	}*/
	
	_RoomInfo.RoomIndex = RoomInfo.RoomIndex;
	_RoomInfo.MinRoomPos = RoomInfo.MinRoomPos;
	_RoomInfo.MaxRoomPos = RoomInfo.MaxRoomPos;
	
	FVector BoxMin{
		_RoomInfo.MinRoomPos.X * 100.0f - 50,
		_RoomInfo.MinRoomPos.Y * 100.0f - 50,
		0.0f
	};

	FVector BoxMax{
		_RoomInfo.MaxRoomPos.X * 100.0f + 50,
		_RoomInfo.MaxRoomPos.Y * 100.0f + 50,
		100.0f
	};

	FRotator CameraOrientation = FRotator{CameraRot.Pitch, Crawlerino::GetYawFromDirection(Direction), CameraRot.Roll};
	FVector CameraPosition = CalculateCameraPositionToFitBoundingBox(BoxMin, BoxMax, CameraOrientation, GetWorld(), Camera->FieldOfView, Camera->AspectRatio);

	StartTransition(CameraStartPos, CameraStartRot, CameraPosition, CameraOrientation);

	// Camera->SetWorldRotation(CameraOrientation);
	// Camera->SetWorldLocation(CameraPosition);

	/*FVector Center = (BoxMin + BoxMax) * 0.5f;
	FVector BoxExtent = (BoxMax - BoxMin) / 2.0f;

	DrawDebugBox(GetWorld(), Center, BoxExtent, FColor::Red, true, -1, 0, 1);

	for (auto TilePos : TilePositions)
	{
		UE_LOG(LogTemp, Display, TEXT("TilePos : %d %d"), TilePos.X, TilePos.Y);
	}
	
	UE_LOG(LogTemp, Display, TEXT("RoomInfo: %d; BoxExtent: %f, %f, %f; Center : %f, %f, %f; BoxMin : %f, %f, %f; BoxMax : %f, %f, %f;"),
		_RoomInfo.RoomIndex,	
		BoxExtent.X, BoxExtent.Y, BoxExtent.Z,
		Center.X, Center.Y, Center.Z,
		BoxMin.X, BoxMin.Y, BoxMin.Z,
		BoxMax.X, BoxMax.Y, BoxMax.Z);*/
}

void ACombatPawn::StartTransition(const FVector& StartPos, const FRotator& StartRot, const FVector& TargetPos, const FRotator& TargetRot)
{
	_IsAnimating = true;
	_AnimationTime = GetWorld()->GetTimeSeconds();
	_CameraStartPos = StartPos;
	_CameraStartRot = StartRot;
	_CameraTargetPos = TargetPos;
	_CameraTargetRot = TargetRot;
}

void ACombatPawn::EndTransition()
{
	_IsAnimating = false;
	_AnimationTime = std::numeric_limits<double>::min();
}


