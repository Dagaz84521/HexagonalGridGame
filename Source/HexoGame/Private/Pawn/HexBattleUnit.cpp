// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/HexBattleUnit.h"
#include "Data/BattleUnitDefinition.h"
#include "ASC/HexagonalAbilitySystemComponent.h"
#include "ASC/HexagonalAttributeSet.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayEffect.h"

UAbilitySystemComponent* AHexBattleUnit::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AHexBattleUnit::InitializeFromUnitDefinition(const UBattleUnitDefinition* UnitDefinition)
{
	if (!UnitDefinition)
	{
		UE_LOG(LogTemp, Warning, TEXT("InitializeFromUnitDefinition called with null UnitDefinition on %s."), *GetNameSafe(this));
		return;
	}

	if (USkeletalMesh* UnitMesh = UnitDefinition->Mesh.LoadSynchronous())
	{
		Mesh->SetSkeletalMesh(UnitMesh);
	}
	else if (!UnitDefinition->Mesh.IsNull())
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("%s: Failed to load mesh from UnitDefinition %s."),
			*GetNameSafe(this),
			*GetNameSafe(UnitDefinition)
		);
	}

	if (UnitDefinition->AnimClass)
	{
		Mesh->SetAnimInstanceClass(UnitDefinition->AnimClass);
	}

	EnsureAbilitySystemInitialized();

	if (!AbilitySystemComponent || !HasAuthority())
	{
		return;
	}

	for (const TSubclassOf<UGameplayEffect>& InitialEffect : UnitDefinition->InitialEffects)
	{
		ApplyInitialGameplayEffect(InitialEffect);
	}

	for (const TSubclassOf<UGameplayAbility>& StartupAbility : UnitDefinition->StartupAbilities)
	{
		if (!StartupAbility)
		{
			continue;
		}

		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(StartupAbility, 1, INDEX_NONE, this));
	}
}

int32 AHexBattleUnit::GetMoveRange() const
{
	return 4; // Placeholder value, replace with actual logic to determine move range based on attributes or unit definition
}

AHexBattleUnit::AHexBattleUnit()
{
	AbilitySystemComponent = CreateDefaultSubobject<UHexagonalAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UHexagonalAttributeSet>(TEXT("AttributeSet"));
}

void AHexBattleUnit::BeginPlay()
{
	Super::BeginPlay();

	EnsureAbilitySystemInitialized();
}

void AHexBattleUnit::EnsureAbilitySystemInitialized()
{
	if (!AbilitySystemComponent || bAbilitySystemInitialized)
	{
		return;
	}

	if (!AttributeSet)
	{
		AttributeSet = NewObject<UHexagonalAttributeSet>(this, TEXT("AttributeSet"));
		UE_LOG(LogTemp, Warning, TEXT("%s created a missing HexagonalAttributeSet at runtime. Recompile and save this unit Blueprint if this keeps happening."), *GetNameSafe(this));
	}

	if (AttributeSet)
	{
		AbilitySystemComponent->AddAttributeSetSubobject(AttributeSet.Get());
	}

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	bAbilitySystemInitialized = true;

	if (HasAuthority())
	{
		ApplyInitialGameplayEffect(InitialAttributeEffect);
	}
}

void AHexBattleUnit::ApplyInitialGameplayEffect(TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	if (!AbilitySystemComponent || !GameplayEffectClass)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffectClass, 1.0f, EffectContext);
	if (SpecHandle.IsValid())
	{
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
