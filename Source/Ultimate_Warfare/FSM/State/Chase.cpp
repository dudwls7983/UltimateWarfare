// Fill out your copyright notice in the Description page of Project Settings.

#include "Chase.h"
#include "AI/Navigation/NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "AIController.h"

CREATE_STATE_CPP(UChase)


void UChase::OnEnter(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UChase::OnEnter"));

	owner->SetChaseStartTime(owner->GetWorld()->GetTimeSeconds());
}


void UChase::OnUpdate(AUltimate_WarfareEnemy *owner)
{
	//UE_LOG(LogTemp, Display, TEXT("UChase::OnUpdate"));
	AActor *target = owner->GetTarget();
	if (target == nullptr)
	{
		return;
	}

	if (owner->CanSeeTarget())
	{
		owner->SetState(owner->stateMap[EEnemyState::ES_Attack]);
		return;
	}

	if (owner->GetWorld()->GetTimeSeconds() - owner->GetChaseStartTime() > owner->chaseTime)
	{
		owner->SetTarget(nullptr);
		owner->SetState(owner->stateMap[EEnemyState::ES_Idle]);
		return;
	}

	UNavigationPath *path = UNavigationSystem::FindPathToLocationSynchronously(GetWorld(), owner->GetActorLocation(), target->GetActorLocation(), target);
	if (path && path->IsValid())
	{
		FAIMoveRequest request;
		request.SetAcceptanceRadius(100.f);
		request.SetUsePathfinding(true);

		AAIController *controller = Cast<AAIController>(owner->GetController());
		if (controller)
		{
			controller->RequestMove(request, path->GetPath());
		}
	}
}


void UChase::OnExit(AUltimate_WarfareEnemy *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UChase::OnExit"));

	// 추격을 종료했으니 모든 행동을 멈춘다.
	AAIController *controller = Cast<AAIController>(owner->GetController());
	if (controller)
	{
		controller->StopMovement();
	}
}


