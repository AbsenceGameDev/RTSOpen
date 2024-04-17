// Fill out your copyright notice in the Description page of Project Settings.

#include "Mass/Tasks/PDAITask_Wait.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus UPDAITask_Wait::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	Super::Tick(Context, DeltaTime);

	TickAccumlator += DeltaTime;
	RunStatus = TickAccumlator >= TickAccumlatorLimit ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Running;
	return RunStatus;
}

EStateTreeRunStatus UPDAITask_Wait::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::EnterState(Context, Transition);
	TickAccumlator = 0.0;
	return EStateTreeRunStatus::Running;
}

void UPDAITask_Wait::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	Super::ExitState(Context, Transition);
}
