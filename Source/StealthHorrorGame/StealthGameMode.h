// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "StealthGameMode.generated.h"

UCLASS()
class STEALTHHORRORGAME_API AStealthGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AStealthGameMode();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Game")
	void HandlePlayerCaught();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Game")
	void HandlePlayerEscaped();
};