// Fill out your copyright notice in the Description page of Project Settings.

#include "StealthHorrorEntity.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "StealthGameMode.h"

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
	LastKnownPlayerPos = GetActorLocation();
	bPlayerCaught = false;
	UE_LOG(LogTemp, Warning, TEXT("Entity BeginPlay ran, bPlayerCaught reset to %d"), bPlayerCaught);

	// Execute MID if material and mesh exists
	if (EntityMesh && EntityMaterial)
	{
		// Create MID from material and apply to mesh
		EntityMID = EntityMesh->CreateDynamicMaterialInstance(0, EntityMaterial);
		UE_LOG(LogTemp, Warning, TEXT("MID Write: %s, MID Valid=%d"), *GetActorLocation().ToString(), EntityMID != nullptr);
		
		if (EntityMID)
		{
			EntityMID->SetScalarParameterValue(FName("Radius"), EntityRadius);
			EntityMID->SetScalarParameterValue(FName("Melt"), MeshMeltAmount);
			EntityMID->SetVectorParameterValue(FName("Offset"), FLinearColor(EntityOffset));
			EntityMID->SetVectorParameterValue(FName("Albedo"), FLinearColor(EntityColour));
			UE_LOG(LogTemp, Warning, TEXT("MID parameters are all set!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MID is null!"));
	}
}

// Called every frame
void AStealthHorrorEntity::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Green, FString::Printf(TEXT("Tick running, bPlayerCaught0%d"), bPlayerCaught));

	// Check if the Material Parameter Collection Exists in the First Place...
	//if (PlayerStateMPC)
	//{
	//	// Get Entity Position
	//	EntityWorldPos = GetActorLocation();
	//
	//	// Set the Vector
	//	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), PlayerStateMPC, FName("EntityCentre"), FLinearColor(EntityWorldPos));
	//}

	if (EntityMID)
	{
		// Update Entity's Position in MID instead of MPC
		EntityWorldPos = GetActorLocation();
		EntityMID->SetVectorParameterValue(FName("EntityCentre"), FLinearColor(EntityWorldPos));
	}

	// Stop if player is caught
	if (bPlayerCaught) 
	{
		GEngine->AddOnScreenDebugMessage(2, 0.f, FColor::Red, FString::Printf(TEXT("EXIT: bPlayerCaught0%d"), bPlayerCaught));
		return;
	}

	// Entity Logic (Gliding/Close Distance towards player)
	// Get Player Pawn
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn)
	{
		GEngine->AddOnScreenDebugMessage(3, 0.f, FColor::Red, FString::Printf(TEXT("EXIT: No Player")));
		return;
	}

	// Get Player/Enemy Position and Speed
	const FVector PlayerPos = PlayerPawn->GetActorLocation();
	const FVector EntityPos = GetActorLocation();
	const float DistanceToPlayer = FVector::Dist(EntityPos, PlayerPos);
	UE_LOG(LogTemp, Warning, TEXT("Dist=%f, CatchRadius=%f"), DistanceToPlayer, CatchRadius);

	// Player Speed Check for Glide
	float PlayerSpeed = 0.f;

	if (PlayerStateMPC)
	{
		PlayerSpeed = UKismetMaterialLibrary::GetScalarParameterValue(GetWorld(), PlayerStateMPC, FName("PlayerSpeed"));
	}

	if (DistanceToPlayer <= CatchRadius && !bPlayerCaught && bCanCatchPlayer)
	{
		bPlayerCaught = true;
		UE_LOG(LogTemp, Warning, TEXT("Caught Player!"));
		AStealthGameMode* GameMode = Cast<AStealthGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		
		if (GameMode)
		{
			GameMode->HandlePlayerCaught();
		}
		return;
	}

	// Line Tracing for Visibility Check
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(PlayerPawn);

	const bool bHasLineOfSight = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		EntityPos,
		PlayerPos,
		ECC_Visibility,
		QueryParams
	);

	const bool bPlayerVisible = !bHasLineOfSight;
	GEngine->AddOnScreenDebugMessage(5, 0.f, FColor::Yellow, FString::Printf(TEXT("Line Trace, hit=%d"), bHasLineOfSight));

	DrawDebugLine(
		GetWorld(),
		EntityPos,
		PlayerPos,
		bPlayerVisible ? FColor::Green : FColor::Red,
		false,  // Not persistent
		-1.f,   // One frame duration
		0,
		5.f     // Thickness adjusted by Aspect Ratio
	);

	if (bPlayerVisible)
	{
		// Reset TimeSinceLastSeenPlayer
		TimeSinceLastSeenPlayer = 0.f;
		const float EnemySpeed = (PlayerSpeed < GlideStillTreshold) ? GlideSpeed : CreepSpeed;
		const FVector NewPlayerPos = FMath::VInterpTo(EntityPos, PlayerPos, DeltaTime, EnemySpeed);
		SetActorLocation(NewPlayerPos);
		
		//if (PlayerSpeed < GlideStillTreshold)
		//{
		//	const FVector NewPlayerPos = FMath::VInterpTo(EntityPos, PlayerPos, DeltaTime, GlideSpeed);
		//	SetActorLocation(NewPlayerPos);
		//}
	}
	else
	{
		TimeSinceLastSeenPlayer += DeltaTime;
		if (TimeSinceLastSeenPlayer >= ReturnDelay)
		{
			SetActorLocation(FMath::VInterpTo(GetActorLocation(), LastKnownPlayerPos, DeltaTime, ReturnSpeed));
		}
	}
}