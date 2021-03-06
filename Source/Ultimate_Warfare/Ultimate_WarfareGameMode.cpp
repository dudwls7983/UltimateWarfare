// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "Ultimate_WarfareGameMode.h"
#include "Ultimate_WarfareHUD.h"
#include "Ultimate_WarfareCharacter.h"
#include "UObject/ConstructorHelpers.h"

AUltimate_WarfareGameMode::AUltimate_WarfareGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/UltimateWarfare/Blueprints/Enemy_BP"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}
