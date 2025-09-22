// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "MonsterController.generated.h"

/**
 * 
 */
UCLASS()
class AMonsterController : public AAIController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	void StartTurn();
};
