// Fill out your copyright notice in the Description page of Project Settings.

#include "StateMachine.h"
#include "GameFramework/Actor.h"
#include "FSM/State/Idle.h"


// Sets default values for this component's properties
UStateMachine::UStateMachine()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


void UStateMachine::SetState(UState * newState)
{
	if (currentState != nullptr)
	{
		UE_LOG(LogTemp, Display, TEXT("%s SetState %s -> %s"), *GetOwner()->GetName(), *currentState->GetName(), *newState->GetName());
		currentState->OnExit(GetOwner());
	}

	newState->OnEnter(GetOwner());

	currentState = newState;
}

// Called when the game starts
void UStateMachine::BeginPlay()
{
	Super::BeginPlay();

	// ...
	SetState(UIdle::GetInstance());
}


// Called every frame
void UStateMachine::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	if (currentState != nullptr)
	{
		currentState->OnUpdate(GetOwner());
	}
}

