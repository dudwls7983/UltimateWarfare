// Fill out your copyright notice in the Description page of Project Settings.

#include "Ultimate_WarfareEnemy.h"
#include "FSM/State/Idle.h"
#include "FSM/State/Chase.h"
#include "FSM/State/Attack.h"

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

	target = nullptr;
	canSeeTarget = false;
}

// �þ߿� ���� ��ȭ�� �Ͼ ������ �Լ� ȣ��
void AUltimate_WarfareEnemy::OnSightUpdate(const TArray<AActor*> &actors)
{
	for (auto &actor : actors)
	{
		const FActorPerceptionInfo* info = AIPerception->GetActorInfo(*actor);
		// ���� ���̹Ƿ� ���̴� ����
		if (info != nullptr && AIPerception->HasActiveStimulus(*actor, UAISense::GetSenseID<UAISense_Sight>()))
		{
			UE_LOG(LogTemp, Warning, TEXT("%s see %s"), *GetName(), *actor->GetName());
			canSeeTarget = true;

			if (target == nullptr) target = actor;
		}
		else // ���� ������ �����Ƿ� �� ���̴� ����
		{
			UE_LOG(LogTemp, Error, TEXT("%s can't see %s"), *GetName(), *actor->GetName());
			canSeeTarget = false;
		}
	}
}

void AUltimate_WarfareEnemy::BeginPlay()
{
	Super::BeginPlay();

	stateMap.Add(EEnemyState::ES_Idle, UIdle::GetInstance());
	stateMap.Add(EEnemyState::ES_Chase, UChase::GetInstance());
	stateMap.Add(EEnemyState::ES_Attack, UAttack::GetInstance());
}
