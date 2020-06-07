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
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AUltimate_WarfareHUD::StaticClass();
}
