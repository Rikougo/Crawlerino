// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "FDungeonPos.generated.h"

USTRUCT(BlueprintType)
struct FDungeonPos
{
	GENERATED_USTRUCT_BODY()

	FDungeonPos()
	{
		
	}
	
	FDungeonPos(int32 x, int32 y) : X(x), Y(y) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X{0};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y{0};


	bool operator==(const FDungeonPos& Other) const
	{
		return this->X == Other.X && this->Y == Other.Y;
	}

	FDungeonPos operator+(const FDungeonPos& Other) const
	{
		return FDungeonPos(X + Other.X, Y + Other.Y);
	}

	FDungeonPos operator-(const FDungeonPos& OtherPos) const
	{
		return FDungeonPos(X - OtherPos.X, Y - OtherPos.Y);
	}
};
