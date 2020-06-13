// Fill out your copyright notice in the Description page of Project Settings.

#include "Idle.h"

CREATE_STATE_CPP(UIdle)


void UIdle::OnEnter(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UIdle::OnEnter"));
}


void UIdle::OnUpdate(AUltimate_WarfareEnemy *owner)
{
	//UE_LOG(LogTemp, Display, TEXT("UIdle::OnUpdate"));
	AActor *target = owner->GetTarget();
	if (target == nullptr)
	{
		return;
	}

	if (owner->CanSeeTarget())
		owner->SetState(owner->stateMap[EEnemyState::ES_Attack]);
	else
		owner->SetState(owner->stateMap[EEnemyState::ES_Chase]);
}


void UIdle::OnExit(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UIdle::OnExit"));
}