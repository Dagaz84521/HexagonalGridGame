// Fill out your copyright notice in the Description page of Project Settings.

#include "Units/HexUnitBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AHexUnitBase::AHexUnitBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>("Root");
	RootComponent = Root;
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetReceivesDecals(false);
	
}

// Called when the game starts or when spawned
void AHexUnitBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyFacingRotation();
	
}

// Called every frame
void AHexUnitBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving || MovePathPoints.Num() == 0 || !MovePathPoints.IsValidIndex(MovePathIndex))
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = MovePathPoints[MovePathIndex];
	const FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, MoveSpeed);

	SetActorLocation(NewLocation);

	if (FVector::DistSquared(NewLocation, TargetLocation) <= FMath::Square(1.0f))
	{
		SetActorLocation(TargetLocation);
		++MovePathIndex;

		if (!MovePathPoints.IsValidIndex(MovePathIndex))
		{
			MovePathPoints.Empty();
			MovePathCoords.Empty();
			MovePathIndex = 0;
			bIsMoving = false;
		}
		else
		{
			UpdateFacingForMoveSegment();
		}
	}
}

void AHexUnitBase::MoveAlongWorldPath(const TArray<FVector>& InPathPoints)
{
	MovePathPoints = InPathPoints;
	MovePathCoords.Empty();
	MovePathIndex = 0;
	bIsMoving = MovePathPoints.Num() > 0;
}

void AHexUnitBase::MoveAlongHexPath(const FHexCoord& StartCoord, const TArray<FHexCoord>& InPathCoords, const TArray<FVector>& InPathPoints)
{
	MovePathStartCoord = StartCoord;
	MovePathCoords = InPathCoords;
	MovePathPoints = InPathPoints;
	MovePathIndex = 0;
	bIsMoving = MovePathPoints.Num() > 0;

	if (bIsMoving)
	{
		UpdateFacingForMoveSegment();
	}
}

void AHexUnitBase::SetFacingDirection(EHexDirection NewDirection)
{
	FacingDirection = NewDirection;
	ApplyFacingRotation();
}

void AHexUnitBase::ApplyFacingRotation()
{
	SetActorRotation(UHexMathLibrary::GetDirectionRotation(FacingDirection, FacingYawOffset));
}

void AHexUnitBase::UpdateFacingForMoveSegment()
{
	if (!MovePathPoints.IsValidIndex(MovePathIndex))
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = MovePathPoints[MovePathIndex];
	const FVector MoveDirection = TargetLocation - CurrentLocation;

	if (!MoveDirection.IsNearlyZero())
	{
		const FRotator MoveRotation = MoveDirection.Rotation();
		SetActorRotation(FRotator(0.0f, MoveRotation.Yaw + FacingYawOffset, 0.0f));
	}

	if (MovePathCoords.IsValidIndex(MovePathIndex))
	{
		const FHexCoord FromCoord = MovePathIndex == 0 ? MovePathStartCoord : MovePathCoords[MovePathIndex - 1];
		FacingDirection = UHexMathLibrary::GetFacingDirection(FromCoord, MovePathCoords[MovePathIndex]);
	}
}

