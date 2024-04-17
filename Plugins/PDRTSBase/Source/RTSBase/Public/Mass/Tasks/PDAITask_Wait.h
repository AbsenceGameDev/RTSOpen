// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "PDAITask_Wait.generated.h"

/**
 *  Task node extensions
 */
UCLASS()
class PDRTSBASE_API UPDAITask_Wait : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TickAccumlator = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	double TickAccumlatorLimit = 0.0;	
};
