// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ultimate_WarfareCharacter.h"
#include "FSM/StateMachine.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Ultimate_WarfareEnemy.generated.h"

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

public:
	AUltimate_WarfareEnemy();

	UFUNCTION()
	void OnSightUpdate(const TArray<AActor *> &actors);

protected:
	virtual void BeginPlay();
	
	
};
