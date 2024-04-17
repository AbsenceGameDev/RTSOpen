// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZoneGraphTypes.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "Tasks/MassZoneGraphPathFollowTask.h"

#include "PDAITask_GoTo.generated.h"

struct FMassZoneGraphLaneLocationFragment;
class UZoneGraphSubsystem;
class UZoneGraphAnnotationSubsystem;
class UMassCrowdSubsystem;
/**
 * 
 */
UCLASS()
class PDRTSBASE_API UPDAITask_GoTo : public UStateTreeTaskBlueprintBase
{
	GENERATED_BODY()

	virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Input")
	class AActor* TargetActor = nullptr;
	
	//
	// TStateTreeExternalDataHandle<FMassZoneGraphLaneLocationFragment> LocationHandle;
	// TStateTreeExternalDataHandle<UZoneGraphSubsystem> ZoneGraphSubsystemHandle;
	// TStateTreeExternalDataHandle<UZoneGraphAnnotationSubsystem> ZoneGraphAnnotationSubsystemHandle;
	// TStateTreeExternalDataHandle<UMassCrowdSubsystem> MassCrowdSubsystemHandle;
	//
	// UPROPERTY(EditAnywhere, Category = Parameter)
	// FZoneGraphTagFilter AllowedAnnotationTags;
	//
	// UPROPERTY(EditAnywhere, Category = Output)
	// FMassZoneGraphTargetLocation TargetPath;	
};
