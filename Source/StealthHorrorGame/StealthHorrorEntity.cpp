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

	// Create Audio Component for the Entity
	EntityAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EntityAudio"));
	EntityAudio->SetupAttachment(RootComponent);
	EntityAudio->bAutoActivate = true;
}

// Called when the game starts or when spawned
void AStealthHorrorEntity::BeginPlay()
{
	Super::BeginPlay();

	SpawnLocation = GetActorLocation();
	LastKnownPlayerPos = SpawnLocation;
	bPlayerCaught = false;

	#if ENTITY_DEBUG
	UE_LOG(LogTemp, Warning, TEXT("Entity BeginPlay ran, bPlayerCaught reset to %d"), bPlayerCaught);
	#endif

	// Execute MID if material and mesh exists
	if (EntityMesh && EntityMaterial)
	{
		// Create MID from material and apply to mesh
		EntityMID = EntityMesh->CreateDynamicMaterialInstance(0, EntityMaterial);

		#if ENTITY_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("MID Write: %s, MID Valid=%d"), *GetActorLocation().ToString(), EntityMID != nullptr);
		#endif

		if (EntityMID)
		{
			EntityMID->SetScalarParameterValue(FName("Radius"), EntityRadius);
			EntityMID->SetScalarParameterValue(FName("Melt"), MeshMeltAmount);
			EntityMID->SetVectorParameterValue(FName("Offset"), FLinearColor(EntityOffset));
			EntityMID->SetVectorParameterValue(FName("Albedo"), FLinearColor(EntityColour));
			EntityMID->SetScalarParameterValue(FName("Shape"), EntityShapeSDF);
			
			#if ENTITY_DEBUG
			UE_LOG(LogTemp, Warning, TEXT("MID parameters are all set!"));
			#endif
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
	FloatTime += DeltaTime;

    #if ENTITY_DEBUG
	GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Green, FString::Printf(TEXT("Tick running, bPlayerCaught0%d"), bPlayerCaught));
	#endif

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
		#if ENTITY_DEBUG
		GEngine->AddOnScreenDebugMessage(3, 0.f, FColor::Red, FString::Printf(TEXT("EXIT: No Player")));
		#endif
		return;
	}

	// Get Player/Enemy Position and Speed
	const FVector PlayerPos = PlayerPawn->GetActorLocation();
	const FVector EntityPos = GetActorLocation();
	const float DistanceToPlayer = FVector::Dist(EntityPos, PlayerPos);

	#if ENTITY_DEBUG
	UE_LOG(LogTemp, Warning, TEXT("Dist=%f, CatchRadius=%f"), DistanceToPlayer, CatchRadius);
	#endif

	// Player Speed Check for Glide
	float PlayerSpeed = 0.f;

	if (PlayerStateMPC)
	{
		PlayerSpeed = UKismetMaterialLibrary::GetScalarParameterValue(GetWorld(), PlayerStateMPC, FName("PlayerSpeed"));
	}

	// Dynamic Material Updates
	if (EntityMID)
	{
		// Dynamically update melt in MID
		float MeltFactor = FMath::Clamp(1000.f / FMath::Max(DistanceToPlayer, 1.f), 0.2f, 1.5f);;
		EntityMID->SetScalarParameterValue(FName("Melt"), MeshMeltAmount * MeltFactor);

		// Glitch SDF by hovering on the material
		FVector MaterialPos = EntityPos;
		MaterialPos.Z += FMath::Sin(FloatTime * 2.f) * HoverAmplitude;

		if (BurstTimer > 0.f)
		{
			MaterialPos.X += FMath::RandRange(-GlitchAmplitude, GlitchAmplitude);
			MaterialPos.Y += FMath::RandRange(-GlitchAmplitude, GlitchAmplitude);
			MaterialPos.Z += FMath::RandRange(-GlitchAmplitude, GlitchAmplitude);
		}

		// Update Entity's Position in MID
		EntityMID->SetVectorParameterValue(FName("EntityCentre"), FLinearColor(MaterialPos));
	}

	if (DistanceToPlayer <= CatchRadius && !bPlayerCaught && bCanCatchPlayer)
	{
		bPlayerCaught = true;

		#if ENTITY_DEBUG
		UE_LOG(LogTemp, Warning, TEXT("Caught Player!"));
		#endif
		
		AStealthGameMode* GameMode = Cast<AStealthGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		
		if (GameMode)
		{
			GameMode->HandlePlayerCaught();
		}
		return;
	}

	// Check if the Material Parameter Collection Exists in the First Place...
	//if (PlayerStateMPC)
	//{
	//	// Get Entity Position
	//	EntityWorldPos = GetActorLocation();
	//
	//	// Set the Vector
	//	UKismetMaterialLibrary::SetVectorParameterValue(GetWorld(), PlayerStateMPC, FName("EntityCentre"), FLinearColor(EntityWorldPos));
	//}

	// Check if player in front of entity (dot product check)
	FVector DirectionToPlayer = (PlayerPos - EntityPos).GetSafeNormal();
	FVector EntityForward = GetActorForwardVector();

	bool bIsPlayerInFront = FVector::DotProduct(EntityForward, DirectionToPlayer) > -0.2f;
	bool bIsPlayerStill = PlayerSpeed < GlideStillTreshold;
	bool bHasLineOfSight = true;
	
	// Line Trace if and only if the player is at front of entity
	if (bIsPlayerInFront || bIsPlayerStill)
	{
		// Line Tracing for Visibility Check
		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);
		QueryParams.AddIgnoredActor(PlayerPawn);

		bHasLineOfSight = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			EntityPos,
			PlayerPos,
			ECC_Visibility,
			QueryParams
		);
	}

	bool bPlayerVisible = !bHasLineOfSight;

	// Prevent player from camping behind a wall being idle
	// Like imitating a heartbeat
	if (DistanceToPlayer < SenseRadius && PlayerSpeed < GlideStillTreshold)
	{
		bPlayerVisible = true;
	}

	// Visibility Grace Period Logic
	if (bPlayerVisible)
	{
		VisibilityTimer = VisiblityGracePeriod;
	}
	else if (VisibilityTimer > 0.f)
	{
		VisibilityTimer -= DeltaTime;
		bPlayerVisible = true;
	}

	const bool bHasGainedSight = bPlayerVisible && !bHadSightLastFrame;

	if (bHasGainedSight)
	{
		if (DistanceToPlayer > ScareMinDistance)
		{
			BurstTimer = BurstDuration;
		}

		if (ScareSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, ScareSound, GetActorLocation());
		}
	}

	if (BurstTimer > 0.f)
	{
		BurstTimer -= DeltaTime;
	}

	#if ENTITY_DEBUG
	GEngine->AddOnScreenDebugMessage(5, 0.f, FColor::Yellow, FString::Printf(TEXT("Line Trace, hit=%d"), bHasLineOfSight));
	#endif

	#if ENTITY_DEBUG
	// Draw Debug Line for Visibility Check (turn off for actual gameplay)
	DrawDebugLine(
		GetWorld(),
		EntityPos,
		PlayerPos,
		bPlayerVisible ? FColor::Green : FColor::Red,
		false,  // Not persistent
		-1.f,   // One frame duration
		0,
		5.f     // Thickness of the line
	);
	#endif

	// Movement Vector Location
	FVector TargetPos = EntityPos;

	if (bCanCatchPlayer)
	{
		if (bPlayerVisible)
		{
			// Reset TimeSinceLastSeenPlayer
			LastKnownPlayerPos = PlayerPos;
			TimeSinceLastSeenPlayer = 0.f;

			float EnemySpeed = (PlayerSpeed < GlideStillTreshold) ? GlideSpeed : CreepSpeed;
			EnemySpeed *= (BurstTimer > 0.f) ? BurstMultiplier : 1.f;

			FVector PlayerVelocity = PlayerPawn->GetVelocity();
			FVector ExpectedPos = PlayerPos + PlayerVelocity * 0.5f;
			TargetPos = FMath::VInterpTo(EntityPos, ExpectedPos, DeltaTime, EnemySpeed);

			//if (PlayerSpeed < GlideStillTreshold)
			//{
			//	const FVector NewPlayerPos = FMath::VInterpTo(EntityPos, PlayerPos, DeltaTime, GlideSpeed);
			//	SetActorLocation(NewPlayerPos);
			//}
		}
		else
		{
			TimeSinceLastSeenPlayer += DeltaTime;
			// Investigate Mode: Go where the player was last seen, then return to that position after a delay
			if (FVector::Dist(EntityPos, LastKnownPlayerPos) > 50.f && TimeSinceLastSeenPlayer < ReturnDelay)
			{
				TargetPos = FMath::VInterpTo(EntityPos, LastKnownPlayerPos, DeltaTime, CreepSpeed);
			}
			// Return Mode: After the delay, return to the last spawn location
			else if (TimeSinceLastSeenPlayer >= ReturnDelay)
			{
				TargetPos = FMath::VInterpTo(EntityPos, SpawnLocation, DeltaTime, ReturnSpeed);
			}
		}

		// Apply movement after the conditional
		SetActorLocation(TargetPos);

		// Rotate entity so that it faces direction to be moved at
		if (FVector::Dist(TargetPos, EntityPos) > 5.f)
		{
			FRotator GetRotation = (TargetPos - EntityPos).Rotation();
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), GetRotation, DeltaTime, 5.f));
		}
		else
		{
			// Radar Scan
			AddActorLocalRotation(FRotator(0.f, DeltaTime * 45.f, 0.f));
		}		
	}
	else
	{
		TimeSinceLastSeenPlayer += DeltaTime;
	}

	// Ramp up Audio
	if (EntityAudio && EntityAudio->IsPlaying())
	{
		float VolumeMultiplier = FMath::Clamp(1.f - DistanceToPlayer / 2000.f, 0.1f, 1.f);
		EntityAudio->SetVolumeMultiplier(VolumeMultiplier);

		// Aggresive Pitch If Player Still
		float Pitch = (PlayerSpeed < GlideStillTreshold) ? 1.5f : 0.8f;
		float PitchMul = FMath::FInterpTo(EntityAudio->PitchMultiplier, Pitch, DeltaTime, 2.f);
		EntityAudio->SetPitchMultiplier(PitchMul);
	}

	bHadSightLastFrame = bPlayerVisible;
}
