// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/CameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ACameraPawn::ACameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 1800.f;
	SpringArm->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	SpringArm->bDoCollisionTest = false;
	SpringArm->bUsePawnControlRotation = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	bFindCameraComponentWhenViewTarget = true;
}

void ACameraPawn::MoveByEdgeScrollInput(const FVector2D& ScreenInput, float ScrollStrength, float DeltaTime)
{
	if (ScreenInput.IsNearlyZero())
	{
		return;
	}

	FVector Forward = Camera->GetForwardVector();
	Forward.Z = 0.f;
	Forward.Normalize();

	FVector Right = Camera->GetRightVector();
	Right.Z = 0.f;
	Right.Normalize();

	FVector MoveDir =
		Right * ScreenInput.X +
		Forward * ScreenInput.Y;

	if (MoveDir.IsNearlyZero())
	{
		return;
	}

	MoveDir.Normalize();

	const float CurrentScrollSpeed =
		FMath::Lerp(MinScrollSpeed, MaxScrollSpeed, ScrollStrength);

	FVector NewLocation =
		GetActorLocation() + MoveDir * CurrentScrollSpeed * DeltaTime;

	NewLocation.X = FMath::Clamp(NewLocation.X, MinCameraBounds.X, MaxCameraBounds.X);
	NewLocation.Y = FMath::Clamp(NewLocation.Y, MinCameraBounds.Y, MaxCameraBounds.Y);

	SetActorLocation(NewLocation);
}

void ACameraPawn::SetFocusTarget(AActor* NewFocusTarget)
{
	FocusTarget = NewFocusTarget;
}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACameraPawn::UpdateFocusFollow(float DeltaTime)
{
    if (!IsValid(FocusTarget))
    {
        return;
    }

    const FVector TargetLocation = FocusTarget->GetActorLocation();

    FVector DesiredLocation = GetActorLocation();
    DesiredLocation.X = TargetLocation.X;
    DesiredLocation.Y = TargetLocation.Y;

    const FVector NewLocation = FMath::VInterpTo(
        GetActorLocation(),
        DesiredLocation,
        DeltaTime,
        FollowInterpSpeed
    );

    SetActorLocation(NewLocation);
}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateFocusFollow(DeltaTime);
}


