// Fill out your copyright notice in the Description page of Project Settings.

#include "Ultimate_WarfareEnemy.h"

AUltimate_WarfareEnemy::AUltimate_WarfareEnemy()
{
	stateMachine = CreateDefaultSubobject<UStateMachine>(FName("State Machine"));
}

void AUltimate_WarfareEnemy::BeginPlay()
{
	Super::BeginPlay();
}
