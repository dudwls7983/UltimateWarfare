// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "State.generated.h"

#define CREATE_STATE_H(class_name) private:\
	static TWeakObjectPtr<class_name> instance;\
public:\
	static class_name* GetInstance() {\
		if (instance.Get() == nullptr)\
			instance = NewObject<class_name>();\
		return instance.Get();\
	}\
	virtual void OnEnter(AActor *owner) override;\
	virtual void OnUpdate(AActor *owner) override;\
	virtual void OnExit(AActor *owner) override;\
	FORCEINLINE virtual FString GetName() const override { return FString(#class_name);};

#define CREATE_STATE_CPP(class_name) TWeakObjectPtr<class_name> class_name::instance = nullptr;

/**
 * 
 */
UCLASS()
class ULTIMATE_WARFARE_API UState : public UObject
{
	GENERATED_BODY()

public:

	virtual void OnEnter(AActor *owner) PURE_VIRTUAL(UState::OnEnter,);
	virtual void OnUpdate(AActor *owner) PURE_VIRTUAL(UState::OnUpdate, );
	virtual void OnExit(AActor *owner) PURE_VIRTUAL(UState::OnExit, );
	virtual FString GetName() const PURE_VIRTUAL(UState::GetName, return FString(););
	
	
};
