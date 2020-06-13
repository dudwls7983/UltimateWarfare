// Fill out your copyright notice in the Description page of Project Settings.

#include "Idle.h"

CREATE_STATE_CPP(UIdle)


void UIdle::OnEnter(AActor *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UIdle::OnEnter"));
}


void UIdle::OnUpdate(AActor *owner)
{
	//UE_LOG(LogTemp, Display, TEXT("UIdle::OnUpdate"));
}


void UIdle::OnExit(AActor *owner)
{
	UE_LOG(LogTemp, Display, TEXT("UIdle::OnExit"));
}