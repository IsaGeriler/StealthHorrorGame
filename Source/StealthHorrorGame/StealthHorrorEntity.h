// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialParameterCollection.h"
#include "Logging/LogMacros.h"
#include "StealthHorrorEntity.generated.h"

UCLASS()
class STEALTHHORRORGAME_API AStealthHorrorEntity : public AActor
{
	GENERATED_BODY()

protected:

	/** Entity Mesh and MPC Declarations */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity")
	class UStaticMeshComponent* EntityMesh;

	UPROPERTY(EditAnywhere, Category = "Entity")
	class UMaterialParameterCollection* PlayerStateMPC;

	/** Result - Entity's World Position Vector */
	FVector EntityWorldPos;
	bool bPlayerCaught = false;

	UPROPERTY(EditAnywhere, Category = "Entity")
	float GlideStillTreshold = 20.f;

	UPROPERTY(EditAnywhere, Category = "Entity")
	float GlideSpeed = 1.f;

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