// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "PP_Capture_SandboxGameMode.h"
#include "PP_Capture_SandboxCharacter.h"
#include "UObject/ConstructorHelpers.h"

APP_Capture_SandboxGameMode::APP_Capture_SandboxGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
