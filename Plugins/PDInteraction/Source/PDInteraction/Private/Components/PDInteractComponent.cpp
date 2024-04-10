/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

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
		OwnerPawn->GetActorLocation(),
		Radius,
		{TraceSettings.GeneratedObjectType},
		nullptr, 
		{OwnerPawn},
		Aggregate);

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


/*
 * @copyright Permafrost Development (MIT license)
 * Authors: Ario Amin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */