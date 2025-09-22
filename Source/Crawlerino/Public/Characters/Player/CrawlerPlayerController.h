// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "CrawlerPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CRAWLERINO_API ACrawlerPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent)
	void StartTurn();
};
