// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "DropGames_TestGameMode.h"
#include "DropGames_TestCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADropGames_TestGameMode::ADropGames_TestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
