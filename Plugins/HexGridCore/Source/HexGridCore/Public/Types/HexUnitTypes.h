#pragma once

#include "CoreMinimal.h"
#include "HexUnitTypes.generated.h"

UENUM(BlueprintType)
enum class EHexUnitFaction : uint8
{
	PlayerControllable UMETA(DisplayName = "Player Controllable"),
	PlayerAlly UMETA(DisplayName = "Player Ally"),
	Neutral UMETA(DisplayName = "Neutral"),
	Enemy UMETA(DisplayName = "Enemy")
};
