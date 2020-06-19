// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ultimate_WarfareCharacter.h"
#include "FSM/StateMachine.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Ultimate_WarfareEnemy.generated.h"

enum class EEnemyState : uint8
{
	ES_Idle UMETA(DisplayName = "Idle"),
	ES_Chase UMETA(DisplayName = "Chase"),
	ES_Attack UMETA(DisplayName = "Attack"),
};

/**
 * 
 */
UCLASS()
class ULTIMATE_WARFARE_API AUltimate_WarfareEnemy : public AUltimate_WarfareCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStateMachine *stateMachine;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAIPerceptionComponent *AIPerception;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAISenseConfig_Sight *sightConfig;

	AActor *target; // Enemy의 타겟
	bool canSeeTarget; // 타겟을 볼수 있는지 여부
	float chaseStartTime; // Chase 시작 시간

public:
	AUltimate_WarfareEnemy();

	FORCEINLINE AActor *GetTarget() const { return target; }
	FORCEINLINE float GetChaseStartTime() const { return chaseStartTime; }
	FORCEINLINE bool CanSeeTarget() const { return canSeeTarget; }

	FORCEINLINE void SetTarget(AActor *newTarget) { target = newTarget; }
	FORCEINLINE void SetState(UState *newState) { stateMachine->SetState(newState); }
	FORCEINLINE void SetChaseStartTime(float time) { chaseStartTime = time; }

	FORCEINLINE void Fire() { OnFire(); }

	UFUNCTION()
	void OnSightUpdate(const TArray<AActor *> &actors);

protected:
	virtual void BeginPlay();

public:
	/** how many time to chase the target */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float chaseTime = 5.f;

	TMap<EEnemyState, UState *> stateMap;
};
