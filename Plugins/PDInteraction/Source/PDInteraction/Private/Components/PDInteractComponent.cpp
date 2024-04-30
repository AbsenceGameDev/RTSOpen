/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "Components/PDInteractComponent.h"
#include "PDInteractCommon.h"
#include "Interfaces/PDInteractInterface.h"

#include <CollisionQueryParams.h>
#include <Kismet/KismetSystemLibrary.h>

FPDTraceResult DummyTrace;

UPDInteractComponent::UPDInteractComponent(const class FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = true;
}

void UPDInteractComponent::BeginPlay()
{
	Super::BeginPlay();
	Prerequisites();
}

void UPDInteractComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (GetOwner<APawn>() == nullptr || GetOwner<APawn>()->IsLocallyControlled() == false)
	{
		SetComponentTickEnabled(false);
		return;
	}

	switch(TraceSettings.TickTraceType)
	{
	case EPDTickTraceType::TRACE_RADIAL:
		PerformRadialTrace(DeltaTime);
		break;
	case EPDTickTraceType::TRACE_LINESHAPE:
		PerformLineShapeTrace(DeltaTime);
		break;
	case EPDTickTraceType::TRACE_MAX:
	default: ;
		PerformLineShapeTrace(DeltaTime);
		PerformRadialTrace(DeltaTime);
		break;
	}
}

void UPDInteractComponent::BeginInteraction()
{
	// @todo interaction timers?
}

void UPDInteractComponent::EndInteraction()
{
	// @todo interaction timers?
}

const FPDTraceResult& UPDInteractComponent::GetTraceResult(const bool bSearchForValidCachedResults) const
{
	if (TraceBuffer.HasValidResults() == false) { return DummyTrace; }
	
	const FPDTraceResult& PDTraceResult = bSearchForValidCachedResults ? TraceBuffer.GetLastValidResult() : TraceBuffer.GetLastTraceResult();
	const FHitResult& TraceResult = PDTraceResult.HitResult;
	
	const IPDInteractInterface* Interface = Cast<IPDInteractInterface>(TraceResult.GetActor());

	const bool bValidHit = Interface != nullptr && TraceResult.IsValidBlockingHit();
	const double InteractionDistance = bValidHit ? Interface->GetMaxInteractionDistance() : 0.0f;
	const double TraceDistance       = bValidHit ? TraceResult.Distance : 1.0f;
	
	return TraceDistance < InteractionDistance ? PDTraceResult : DummyTrace; // Do not allow returning items that are traced past their max interaction distance
}

const FPDTraceSettings& UPDInteractComponent::GetTraceSettings() const
{
	return TraceSettings;
}

void UPDInteractComponent::SetTraceSettings(const FPDTraceSettings& NewSettings)
{
	TraceSettings = NewSettings;
}

bool UPDInteractComponent::ContainsValidTraceResults() const
{
	return GetTraceResult(true) != DummyTrace;
}

TArray<AActor*> UPDInteractComponent::GetAllInteractablesInRadius(double Radius, bool bIgnorePerObjectInteractionDistance)
{
	TArray<AActor*> Aggregate{};
	APawn* OwnerPawn = GetOwner<APawn>();
	if (OwnerPawn == nullptr) { return Aggregate; }

	TArray<AActor*> OverlapActors{};
	UKismetSystemLibrary::SphereOverlapActors(
		OwnerPawn,
		OverridePosition == FVector::ZeroVector ? OwnerPawn->GetActorLocation() : OverridePosition,
		Radius,
		{TraceSettings.GeneratedObjectType},
		nullptr, 
		{OwnerPawn},
		Aggregate);
	OverridePosition = bResetOverrideNextFrame ? FVector::ZeroVector : OverridePosition;
	
	if (bIgnorePerObjectInteractionDistance)
	{
		for (AActor* OverlappingActor : OverlapActors)
		{
			const IPDInteractInterface* ActorAsInterface = Cast<IPDInteractInterface>(OverlappingActor);
			if (ActorAsInterface == nullptr) { continue; }
			Aggregate.Emplace(OverlappingActor);
		}
	}
	else
	{
		for (AActor* OverlappingActor : OverlapActors)
		{
			const IPDInteractInterface* ActorAsInterface = Cast<IPDInteractInterface>(OverlappingActor);
			if (ActorAsInterface == nullptr) { continue; }

			FVector DistanceVector = OverlappingActor->GetActorLocation() - OwnerPawn->GetActorLocation();
			const bool bPastInteractionDistance = DistanceVector.Length() > ActorAsInterface->GetMaxInteractionDistance();

			if (bPastInteractionDistance) { continue; }
			Aggregate.Emplace(OverlappingActor);
		}		
	}
	
	return Aggregate;
}

void UPDInteractComponent::TraceToTarget(const FVector& TraceEnd)
{
	FCollisionQueryParams TraceParams;
	TraceToTarget(TraceEnd,TraceParams);
}

void UPDInteractComponent::TraceToTarget(const FVector& TraceStart, const FVector& TraceEnd)
{
	FCollisionQueryParams TraceParams;
	TraceToTarget(TraceStart, TraceEnd,TraceParams);
}

void UPDInteractComponent::TraceToTarget(const FVector& TraceEnd, FCollisionQueryParams& TraceParams)
{
	const FVector TraceStart = FVector::ZeroVector;
	TraceToTarget(TraceStart, TraceEnd, TraceParams);
}
void UPDInteractComponent::TraceToTarget(const FVector& TraceStart, const FVector& TraceEnd, FCollisionQueryParams& TraceParams)
{
	FHitResult TraceHitResult;
	bool bInteractTraceResult = false;
	PerformSimpleTrace(TraceStart, TraceEnd, TraceParams, TraceHitResult, bInteractTraceResult);
	TraceBuffer.AddTraceFrame(bInteractTraceResult ? EPDTraceResult::TRACE_SUCCESS : EPDTraceResult::TRACE_FAIL, TraceHitResult);
}
FPDTraceResult UPDInteractComponent::TraceToTargetAndReset(const FVector& TraceEnd)
{
	FCollisionQueryParams TraceParams;
	return TraceToTargetAndReset(TraceEnd, TraceParams);
}

FPDTraceResult UPDInteractComponent::TraceToTargetAndReset(const FVector& TraceStart, const FVector& TraceEnd)
{
	FCollisionQueryParams TraceParams;
	return TraceToTargetAndReset(TraceStart, TraceEnd, TraceParams);	
}

FPDTraceResult UPDInteractComponent::TraceToTargetAndReset(const FVector& TraceEnd, FCollisionQueryParams& TraceParams)
{
	const FVector TraceStart = FVector::ZeroVector;
	return TraceToTargetAndReset(TraceStart, TraceEnd, TraceParams);
}
FPDTraceResult UPDInteractComponent::TraceToTargetAndReset(const FVector& TraceStart, const FVector& TraceEnd, FCollisionQueryParams& TraceParams)
{
	const APawn* OwnerPawn = GetOwner<APawn>();
	if (OwnerPawn == nullptr) { return FPDTraceResult(); }
	
	TraceToTarget(TraceStart, TraceEnd, TraceParams);
	FPDTraceResult InteractTraceResult = GetTraceResult(true);
	ClearTraceResults();
	return InteractTraceResult;
}

void UPDInteractComponent::Prerequisites()
{
	const FLatentActionInfo DelayInfo{0,0, TEXT("Prerequisites"), this};
	const AController* PC = GetOwner<APawn>() != nullptr ? GetOwner<APawn>()->GetController() : nullptr;
	if (PC == nullptr)
	{
		UKismetSystemLibrary::Delay(GetOwner(), 1.0f, DelayInfo);
		return;
	}

	const FPDTraceSettings* TraceSettingsPtr = TraceSettingsHandle.GetRow<FPDTraceSettings>("");
	if (TraceSettingsPtr != nullptr) { TraceSettings = *TraceSettingsPtr; }
	
	TraceSettings.Setup();
	TraceBuffer.Setup();
	SetComponentTickEnabled(true);
}

void UPDInteractComponent::ClearTraceResults()
{
	TraceBuffer.ClearTraceResults();
}

void UPDInteractComponent::PerformLineShapeTrace_Implementation(double DeltaSeconds)
{
	TraceBuffer.LineTraceTickTime += DeltaSeconds;
	const bool bCanTick = TraceBuffer.LineTraceTickTime >= TraceSettings.LineTraceTickInterval && GetOwner<APawn>() != nullptr;
	if (bCanTick == false) { return; }
	
	FVector TraceStart, TraceEnd;
	FCollisionQueryParams TraceParams;
	FHitResult InteractHitResult;
	
	EPDTraceResult TraceResult;
	PerformComparativeTraces(TraceStart, TraceEnd, TraceParams, InteractHitResult, TraceResult);
	TraceBuffer.AddTraceFrame(TraceResult, InteractHitResult);
}

void UPDInteractComponent::PerformRadialTrace_Implementation(double DeltaSeconds)
{
	TraceBuffer.RadialTraceTickTime += DeltaSeconds;
	const bool bCanTick = TraceBuffer.RadialTraceTickTime >= TraceSettings.RadialTraceTickInterval;
	if (bCanTick == false) { return; }

	// Clear accumulated time and Store any interactables
	TraceBuffer.RadialTraceTickTime = 0;
	TraceBuffer.RadialTraceActors = GetAllInteractablesInRadius(TraceSettings.MaxRadialTraceDistanceInUnrealUnits);
}

void UPDInteractComponent::PerformComparativeTraces(FVector& TraceStart, FVector& TraceEnd, FCollisionQueryParams& TraceParams, FHitResult& TraceHitResult, EPDTraceResult& TraceResultFlag) const
{
	APawn* OwnerPawn = GetOwner<APawn>();
	if (OwnerPawn == nullptr) { return; }

	APlayerController* OwnerPC = Cast<APlayerController>(GetOwner<APawn>()->GetController());
	if (OwnerPC == nullptr) { return; }
	
	FRotator PawnViewRotation;
	OwnerPC->GetPlayerViewPoint(TraceStart, PawnViewRotation);
	TraceEnd = TraceStart + (GetMaxTraceDistance() * PawnViewRotation.Vector());

	// Trace 1 (Leading trace)
	FHitResult CameraTraceResult;
	bool bTraceResult = false;
	TracePass(TraceStart, TraceEnd, TraceParams, CameraTraceResult, bTraceResult);

	// Trace 2 (Comparison trace)	
	const FVector PawnEyeLocation = OwnerPawn->GetActorLocation() + FVector(0.f,0.f, OwnerPawn->BaseEyeHeight);
	const FVector TargetEndLocation = 
		CameraTraceResult.ImpactPoint != FVector::ZeroVector ? CameraTraceResult.ImpactPoint :
		CameraTraceResult.Location    != FVector::ZeroVector ? CameraTraceResult.Location    :
		FVector::ZeroVector;

	bool bComparativeTraces = false;
	TracePass(PawnEyeLocation, TargetEndLocation , TraceParams, TraceHitResult, bComparativeTraces);

	// Combine trace results
	bool bInteractTracesValid   = bTraceResult && bComparativeTraces && TraceHitResult.GetActor() != nullptr && CameraTraceResult.GetActor() != nullptr;
	bool bCompareInteractTraces = TraceHitResult.GetActor() == CameraTraceResult.GetActor();	
	TraceResultFlag = bTraceResult ? EPDTraceResult::TRACE_SUCCESS : EPDTraceResult::TRACE_FAIL; 
	
	if (bInteractTracesValid && bCompareInteractTraces) { return; }
	TraceHitResult = FHitResult(); // Clear hit-results if comparison checks or validity checks fail
}

void UPDInteractComponent::PerformSimpleTrace(const FVector& TraceStart, const FVector& TraceEnd, FCollisionQueryParams& TraceParams, FHitResult& TraceHitResult, bool& bTraceResultFlag) const
{
	const APawn* OwnerPawn = GetOwner<APawn>();

	const bool bTraceFromView = (TraceStart == FVector::ZeroVector && OwnerPawn != nullptr);
	const FVector ViewOffset = FVector(0.f,0.f, OwnerPawn->BaseEyeHeight);
	const FVector SelectedStart = bTraceFromView ? OwnerPawn->GetActorLocation() + ViewOffset : TraceStart;

	TracePass(SelectedStart, TraceEnd, TraceParams, TraceHitResult, bTraceResultFlag);
	if (bTraceResultFlag == false || TraceHitResult.GetActor() == nullptr)
	{
		TraceHitResult = FHitResult(); 
	}
}

void UPDInteractComponent::TracePass(const FVector& TraceFromLocation, const FVector& TraceEnd, FCollisionQueryParams& TraceParams, FHitResult& InteractHitResult, bool& bTraceResultFlag) const
{
	const APawn* OwnerPawn = GetOwner<APawn>();

	TraceParams = FCollisionQueryParams(FName(""), false, OwnerPawn);
	TraceParams.AddIgnoredActor(OwnerPawn);
	for (const AActor* ActorToIgnore: RuntimeIgnoreList)
	{
		TraceParams.AddIgnoredActor(ActorToIgnore);
	}
	
	InteractHitResult = FHitResult(ForceInit);
	bTraceResultFlag = GetWorld()->SweepSingleByChannel(InteractHitResult, TraceFromLocation, TraceEnd, FQuat::Identity, TraceSettings.TraceChannel, FCollisionShape::MakeBox(FVector(BoxTraceExtent)), TraceParams);
}


/**
Business Source License 1.1

Parameters

Licensor:             Ario Amin (@ Permafrost Development)
Licensed Work:        RTSOpen (Source available on github)
                      The Licensed Work is (c) 2024 Ario Amin (@ Permafrost Development)
Additional Use Grant: You may make free use of the Licensed Work in a commercial product or service provided these three additional conditions as met; 
                      1. Must give attributions to the original author of the Licensed Work, in 'Credits' if that is applicable.
                      2. The Licensed Work must be Compiled before being redistributed.
                      3. The Licensed Work Source may be linked but may not be packaged into the product or service being sold

                      "Credits" indicate a scrolling screen with attributions. This is usually in a products end-state

                      "Package" means the collection of files distributed by the Licensor, and derivatives of that collection
                      and/or of those files..   

                      "Source" form means the source code, documentation source, and configuration files for the Package, usually in human-readable format.

                      "Compiled" form means the compiled bytecode, object code, binary, or any other
                      form resulting from mechanical transformation or translation of the Source form.

Change Date:          2028-04-17

Change License:       Apache License, Version 2.0

For information about alternative licensing arrangements for the Software,
please visit: https://permadev.se/

Notice

The Business Source License (this document, or the “License”) is not an Open
Source license. However, the Licensed Work will eventually be made available
under an Open Source License, as stated in this License.

License text copyright (c) 2017 MariaDB Corporation Ab, All Rights Reserved.
“Business Source License” is a trademark of MariaDB Corporation Ab.

-----------------------------------------------------------------------------

Business Source License 1.1

Terms

The Licensor hereby grants you the right to copy, modify, create derivative
works, redistribute, and make non-production use of the Licensed Work. The
Licensor may make an Additional Use Grant, above, permitting limited
production use.

Effective on the Change Date, or the fourth anniversary of the first publicly
available distribution of a specific version of the Licensed Work under this
License, whichever comes first, the Licensor hereby grants you rights under
the terms of the Change License, and the rights granted in the paragraph
above terminate.

If your use of the Licensed Work does not comply with the requirements
currently in effect as described in this License, you must purchase a
commercial license from the Licensor, its affiliated entities, or authorized
resellers, or you must refrain from using the Licensed Work.

All copies of the original and modified Licensed Work, and derivative works
of the Licensed Work, are subject to this License. This License applies
separately for each version of the Licensed Work and the Change Date may vary
for each version of the Licensed Work released by Licensor.

You must conspicuously display this License on each original or modified copy
of the Licensed Work. If you receive the Licensed Work in original or
modified form from a third party, the terms and conditions set forth in this
License apply to your use of that work.

Any use of the Licensed Work in violation of this License will automatically
terminate your rights under this License for the current and all other
versions of the Licensed Work.

This License does not grant you any right in any trademark or logo of
Licensor or its affiliates (provided that you may use a trademark or logo of
Licensor as expressly required by this License).

TO THE EXTENT PERMITTED BY APPLICABLE LAW, THE LICENSED WORK IS PROVIDED ON
AN “AS IS” BASIS. LICENSOR HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS,
EXPRESS OR IMPLIED, INCLUDING (WITHOUT LIMITATION) WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT, AND
TITLE.

MariaDB hereby grants you permission to use this License’s text to license
your works, and to refer to it using the trademark “Business Source License”,
as long as you comply with the Covenants of Licensor below.

Covenants of Licensor

In consideration of the right to use this License’s text and the “Business
Source License” name and trademark, Licensor covenants to MariaDB, and to all
other recipients of the licensed work to be provided by Licensor:

1. To specify as the Change License the GPL Version 2.0 or any later version,
   or a license that is compatible with GPL Version 2.0 or a later version,
   where “compatible” means that software provided under the Change License can
   be included in a program with software provided under GPL Version 2.0 or a
   later version. Licensor may specify additional Change Licenses without
   limitation.

2. To either: (a) specify an additional grant of rights to use that does not
   impose any additional restriction on the right granted in this License, as
   the Additional Use Grant; or (b) insert the text “None”.

3. To specify a Change Date.

4. Not to modify this License in any other way.
 **/