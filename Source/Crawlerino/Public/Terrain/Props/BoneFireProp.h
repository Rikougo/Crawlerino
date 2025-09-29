// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Terrain/Props/Interactable.h"
#include "Terrain/Props/TerrainProp.h"

#include "BoneFireProp.generated.h"

UCLASS(Blueprintable)
class CRAWLERINO_API ABoneFireProp : public ATerrainProp, public IInteractable
{
	GENERATED_BODY()
public:	
	ABoneFireProp();
protected:
	virtual void BeginPlay() override;
public:	
	virtual void Interact_Implementation() override;
};
