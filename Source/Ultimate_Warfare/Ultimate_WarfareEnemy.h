// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ultimate_WarfareCharacter.h"
#include "FSM/StateMachine.h"
#include "Ultimate_WarfareEnemy.generated.h"

/**
 * 
 */
UCLASS()
class ULTIMATE_WARFARE_API AUltimate_WarfareEnemy : public AUltimate_WarfareCharacter
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UStateMachine *stateMachine;

public:
	AUltimate_WarfareEnemy();

protected:
	virtual void BeginPlay();
	
	
};
