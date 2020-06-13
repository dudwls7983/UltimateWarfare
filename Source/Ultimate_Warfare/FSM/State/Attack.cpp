// Fill out your copyright notice in the Description page of Project Settings.

#include "Attack.h"

CREATE_STATE_CPP(UAttack)


void UAttack::OnEnter(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UAttack::OnEnter"));
}


void UAttack::OnUpdate(AUltimate_WarfareEnemy *owner)
{
	AActor *target = owner->GetTarget();
	if (target == nullptr)
	{
		return;
	}
	if (owner->CanSeeTarget() == false)
	{
		owner->SetState(owner->stateMap[EEnemyState::ES_Chase]);
		return;
	}

	// Do Attack
}


void UAttack::OnExit(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UAttack::OnExit"));
}


