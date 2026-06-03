// Fill out your copyright notice in the Description page of Project Settings.

#include "StealthHorrorEntity.h"

// Sets default values
AStealthHorrorEntity::AStealthHorrorEntity()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Static Mesh Component & Set as Root
	EntityMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EntityMesh"));
	SetRootComponent(EntityMesh);

}

// Called when the game starts or when spawned
void AStealthHorrorEntity::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStealthHorrorEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if the Material Parameter Collection Exists in the First Place...
	if (PlayerStateMPC)
	{
		// Get Entity Position
		EntityWorldPos = GetActorLocation();

		// Set the Vector
		UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), PlayerStateMPC, FName("EntityCentre"), FLinearColor(EntityWorldPos));
	}

	// Entity Logic (Gliding/Close Distance towards player)
	// Get Player Pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (PlayerPawn)
	{
		// Get Player Position and Speed
		const FVector PlayerPos = PlayerPawn->GetActorLocation();
		float PlayerSpeed = 0.f;

		if (PlayerStateMPC)
		{
			PlayerSpeed = UKismetMaterialLibrary::GetScalarParameterValue(GetWorld(), PlayerStateMPC, FName("PlayerSpeed"));

			if (PlayerSpeed > GlideStillTreshold)
			{
				const FVector NewPlayerPos = FMath::VInterpTo(GetActorLocation(), PlayerPos, DeltaTime, GlideSpeed);
				SetActorLocation(NewPlayerPos);
			}
		}
	}
}