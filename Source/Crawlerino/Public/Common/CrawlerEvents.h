#pragma once

#include "CoreMinimal.h"
#include "CrawlerEvents.generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartTurn);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDungeonTextureReady);
