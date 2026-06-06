// Fill out your copyright notice in the Description page of Project Settings.

#include "StealthGameMode.h"

AStealthGameMode::AStealthGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClass(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControlClass(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonPlayerController"));
	if (PlayerPawnClass.Succeeded()) DefaultPawnClass = PlayerPawnClass.Class;
	if (PlayerControlClass.Succeeded()) PlayerControllerClass = PlayerControlClass.Class;
}