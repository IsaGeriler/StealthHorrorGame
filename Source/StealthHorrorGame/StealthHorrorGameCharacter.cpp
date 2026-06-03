// Copyright Epic Games, Inc. All Rights Reserved.

#include "StealthHorrorGameCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Math/Color.h"
#include "StealthHorrorGame.h"

AStealthHorrorGameCharacter::AStealthHorrorGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void AStealthHorrorGameCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AStealthHorrorGameCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AStealthHorrorGameCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AStealthHorrorGameCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AStealthHorrorGameCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AStealthHorrorGameCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogStealthHorrorGame, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AStealthHorrorGameCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void AStealthHorrorGameCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void AStealthHorrorGameCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AStealthHorrorGameCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void AStealthHorrorGameCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void AStealthHorrorGameCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void AStealthHorrorGameCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Check if the Material Parameter Collection Exists in the First Place...
	if (!PlayerStateMPC)
	{
		UE_LOG(LogStealthHorrorGame, Error, TEXT("'%s' Failed to find an UMaterialParameterCollection* instance!"), *GetNameSafe(this));
		return;
	}

	// Get Speed
	const float Speed = GetVelocity().Size2D();
	SmoothedSpeed = FMath::FInterpTo(SmoothedSpeed, Speed, DeltaTime, SpeedInterpolationRate);
	
	// Set the Scalar
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), PlayerStateMPC, FName("PlayerSpeed"), SmoothedSpeed);

	// Resolve Amount for Denoising
	StillnessDuration = (SmoothedSpeed < StillnessTreshold) ? (StillnessDuration += DeltaTime) : 0.f;
	float ResolveValue = FMath::Clamp(StillnessDuration / ResolveDuration, 0.f, 1.f);

	// Set the Scalar
	UKismetMaterialLibrary::SetScalarParameterValue(GetWorld(), PlayerStateMPC, FName("ResolveAmount"), ResolveValue);
}