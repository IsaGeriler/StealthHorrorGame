// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Logging/LogMacros.h"
#include "StealthHorrorEntity.generated.h"

#define ENTITY_DEBUG false

UCLASS()
class STEALTHHORRORGAME_API AStealthHorrorEntity : public AActor
{
	GENERATED_BODY()

protected:

	/** Entity Mesh and MPC Declarations */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity")
	class UStaticMeshComponent* EntityMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity")
	class UAudioComponent* EntityAudio;

	UPROPERTY(EditAnywhere, Category = "Entity")
	class UMaterialParameterCollection* PlayerStateMPC;

	/** Result - Entity's World Position Vector */
	FVector EntityWorldPos;
	bool bPlayerCaught = false;

	UPROPERTY(EditAnywhere, Category = "Entity")
	bool bCanCatchPlayer = true;

	UPROPERTY(EditAnywhere, Category = "Entity")
	float GlideStillTreshold = 20.f;

	UPROPERTY(EditAnywhere, Category = "Entity")
	float GlideSpeed = 2.f;

	UPROPERTY(EditAnywhere, Category = "Entity")
	float CatchRadius = 100.f;

	/** MID */
	UPROPERTY(EditAnywhere, Category = "Entity")
	class UMaterialInterface* EntityMaterial;

	UPROPERTY()
	class UMaterialInstanceDynamic* EntityMID;

	/** Some of the variables originally from material node, carried here */
	UPROPERTY(EditAnywhere, Category = "Entity")
	float EntityRadius = 40.f;
	
	UPROPERTY(EditAnywhere, Category = "Entity")
	float MeshMeltAmount = 15.f;

	UPROPERTY(EditAnywhere, Category = "Entity")
	FVector EntityOffset = FVector(14.f, 8.f, 22.f);

	UPROPERTY(EditAnywhere, Category = "Entity")
	FVector EntityColour = FVector(0.9f, 0.1f, 0.1f);

	UPROPERTY(EditAnywhere, Category = "Entity")
	float EntityShapeSDF = 0.4f;

	/** Address the gap where player idle but entity can't see player */
	UPROPERTY(EditAnywhere, Category = "Entity")
	float ReturnDelay = 5.f;

	UPROPERTY(EditAnywhere, Category = "Entity")
	float ReturnSpeed = 1.f;

	// Grace Period
	UPROPERTY(EditAnywhere, Category = "Entity")
	float VisiblityGracePeriod = 0.5f;

	// Sense Radius
	UPROPERTY(EditAnywhere, Category = "Entity")
	float SenseRadius = 250;

	FVector LastKnownPlayerPos;
	FVector SpawnLocation;
	float TimeSinceLastSeenPlayer = 0.f;
	float VisibilityTimer = 0.f;
	float FloatTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Entity")
	float CreepSpeed = 0.5f;

	// Visual Jitter
	UPROPERTY(EditAnywhere, Category = "Movement")
	float HoverAmplitude = 10.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float GlitchAmplitude = 25.f;

	/** Scare */
	UPROPERTY(EditAnywhere, Category = "Scare")
	float BurstDuration = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Scare")
	float BurstMultiplier = 2.5f;

	UPROPERTY(EditAnywhere, Category = "Scare")
	USoundBase* ScareSound;

	UPROPERTY(EditAnywhere, Category = "Scare")
	float ScareMinDistance = 600.f;

	bool bHadSightLastFrame = false;
	float BurstTimer = 0.f;
	
public:	
	// Sets default values for this actor's properties
	AStealthHorrorEntity();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};