// Fill out your copyright notice in the Description page of Project Settings.


#include "Pawn/HexBattleUnit.h"

#include "ASC/HexagonalAbilitySystemComponent.h"
#include "ASC/HexagonalAttributeSet.h"
#include "GameplayEffect.h"

UAbilitySystemComponent* AHexBattleUnit::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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

	if (AbilitySystemComponent)
	{
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

		if (HasAuthority() && InitialAttributeEffect)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(InitialAttributeEffect, 1.0f, EffectContext);
			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}
