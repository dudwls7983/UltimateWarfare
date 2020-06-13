// Fill out your copyright notice in the Description page of Project Settings.

#include "Ultimate_WarfareEnemy.h"

AUltimate_WarfareEnemy::AUltimate_WarfareEnemy()
{
	stateMachine = CreateDefaultSubobject<UStateMachine>(FName("State Machine"));

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(FName("AI Perception"));
	sightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(FName("Sight Config"));

	sightConfig->SightRadius = 3000.f;
	sightConfig->LoseSightRadius = 3600.f;
	sightConfig->PeripheralVisionAngleDegrees = 90.f;
	sightConfig->DetectionByAffiliation.bDetectEnemies = true;
	sightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	sightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	AIPerception->ConfigureSense(*sightConfig);
	AIPerception->SetDominantSense(sightConfig->GetSenseImplementation());
	AIPerception->OnPerceptionUpdated.AddDynamic(this, &AUltimate_WarfareEnemy::OnSightUpdate);

	TeamID = FGenericTeamId(1);
}

void AUltimate_WarfareEnemy::OnSightUpdate(const TArray<AActor*> &actors)
{
	for (auto &actor : actors)
	{
		const FActorPerceptionInfo* info = AIPerception->GetActorInfo(*actor);
		if (info != nullptr && AIPerception->HasActiveStimulus(*actor, UAISense::GetSenseID<UAISense_Sight>()))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s see %s"), *GetName(), *actor->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s can't see %s"), *GetName(), *actor->GetName());
		}
	}
}

void AUltimate_WarfareEnemy::BeginPlay()
{
	Super::BeginPlay();
}
