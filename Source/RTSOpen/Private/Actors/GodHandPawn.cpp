/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

/* Permadev - Actors */
#include "Actors/GodHandPawn.h"
#include "Actors/PDInteractActor.h"
#include "Actors/RTSOController.h"
#include "Pawns/PDRTSBaseUnit.h"
#include "PDRTSCommon.h"

/* Permadev - Subsystems */
#include "PDRTSBaseSubsystem.h"

/* Permadev - Components */
#include "Components/PDInteractComponent.h"
#include "Components/PDInventoryComponent.h"

/* Engine - Components */
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"

/* Engine - Input */
#include "EnhancedInputComponent.h"

/* Engine - Effects */
#include "MassCommonFragments.h"
#include "MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Chaos/DebugDrawQueue.h"

// Navigation
#include "ConversationParticipantComponent.h"
#include "MassCrowdRepresentationSubsystem.h"
#include "MassRepresentationFragments.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "AI/Mass/PDMassFragments.h"
#include "Core/RTSOInputStackSubsystem.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

//
// Godhand functionality

// Called every frame
void AGodHandPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TrackMovement(DeltaTime);
	HoverTick(DeltaTime);
	RotationTick(DeltaTime);

	if (bUpdatePathOnTick) { RefreshPathingEffects(); }
	
}

void AGodHandPawn::TrackMovement(float DeltaTime)
{
	const ARTSOController* PC = GetController<ARTSOController>();
	if (PC == nullptr) { return; }
		
	// // Update collision and mesh placement
	FVector2D ScreenCoordinates;
	FVector IntersectionPoint;
	bool bFoundInputType;

	PC->ProjectMouseToGroundPlane(ScreenCoordinates,IntersectionPoint, bFoundInputType);

	IntersectionPoint += FVector{0.0, 0.0, 10.0};
	Collision->SetWorldLocation(IntersectionPoint);
	InteractComponent->SetOverrideTracePosition(IntersectionPoint);
	UpdateCursorLocation(DeltaTime);
}

void AGodHandPawn::TrackUnitMovement()
{
	const ARTSOController* PC = GetController<ARTSOController>();
	if (PC == nullptr) { return; }
	
	FVector2D v2Dummy{};
	FVector MousePlaneIntersection{};
	bool bHasInput{};
	PC->ProjectMouseToGroundPlane(v2Dummy, MousePlaneIntersection, bHasInput);
	if (bHasInput == false) { return; }
	
	const FVector CameraLagOffsetCompensation = FVector::ZeroVector; /**< @todo camera lag is controlled by the pd camera manager, might write a function there */
	const FVector FinalMove = WorkUnitTargetLocation - MousePlaneIntersection - CameraLagOffsetCompensation;
	AddActorWorldOffset(FVector{FinalMove.X, FinalMove.Y, 0.0});
}

void AGodHandPawn::HoverTick(const float DeltaTime)
{
	AActor* ClosestActor = FindClosestInteractableActor();
	// Overwrite HoveredActor if they are not the same
	if (ClosestActor != nullptr && ClosestActor != HoveredActor)
	{
		HoveredActor = ClosestActor;
		UE_LOG(LogTemp, Warning, TEXT("HoverTick - Found New Hover Actor"))
	}
	
	const FMassEntityHandle ClosestMeshInstance = FindClosestMeshInstance();
	if (ClosestMeshInstance.Index != INDEX_NONE && SelectedWorkerUnitHandle.Index != ClosestMeshInstance.Index)
	{
		// Not functionally relevant, keep here in case we want to put something here
		UE_LOG(LogTemp, Warning, TEXT("HoverTick - Found New Unit ISM"))
	}

	// Dont overwrite in case we are dragging a path from this
	if (bUpdatePathOnTick == false ) { SelectedWorkerUnitHandle = ClosestMeshInstance; }
}

// @todo force it to cardinal directions, move to timer 
void AGodHandPawn::RotationTick(float& DeltaTime)
{
	if (CurrentRotationLeft <= 0.0) { return; }

	const int32 Direction = RotationDeque.First();

	DeltaTime *= RotationRateModifier;

	const double DeltaYaw = FMath::Clamp(DeltaTime / (CurrentRotationLeft / 90.0), 0.0, 2.0);
	CurrentRotationLeft -= DeltaYaw;
	
	if (CurrentRotationLeft < 0)
	{
		RotationDeque.PopFirst();
		
		FRotator FinalRotation = GetActorRotation();
		FinalRotation.Yaw = TargetYaw;
		SetActorRotation(FinalRotation);
		
		TargetYaw =
			RotationDeque.IsEmpty() ? TargetYaw
				: static_cast<int32>(GetActorRotation().Yaw + 0.5 + (90.0 * RotationDeque.First()));

		CurrentRotationLeft = RotationDeque.IsEmpty() ? 0.0 : 90.0;
		bIsInRotation = false;

		return;
	}
	
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += DeltaYaw * Direction;
	SetActorRotation(NewRotation);
}

void AGodHandPawn::UpdateMagnification()
{
	MagnificationValue = FMath::Clamp((MagnificationStrength * 0.01) + MagnificationValue, 0.01, 1.0);

	const float Alpha = MagnificationCurve != nullptr
		? MagnificationCurve->GetFloatValue(MagnificationValue)
		: Springarm->TargetArmLength / FMath::Clamp(Springarm->TargetArmLength * MagnificationValue, Springarm->TargetArmLength * 0.5, Springarm->TargetArmLength  * 1.2);
	
	Springarm->TargetArmLength = FMath::Lerp(600, 18000, Alpha);
	Springarm->SetRelativeRotation(FRotator{FMath::Lerp(-35.0,-55.0, Alpha),0,0});
	PawnMovement->MaxSpeed =  FMath::Lerp(1000, 5000, Alpha);

	// depth of field when magnifying
	FPostProcessSettings PPSettings{};
	PPSettings.DepthOfFieldFocalDistance = Springarm->TargetArmLength;
	PPSettings.DepthOfFieldSensorWidth = 175.f;
	Camera->PostProcessSettings = PPSettings;

	Camera->SetFieldOfView(FMath::Lerp(25.0f, 15.0f, Alpha));
}

void AGodHandPawn::UpdateCursorLocation(float DeltaTime)
{
	// @todo handle input types+ if so reuse enum typ ECommonInputType and set up some events to register different input from different types

	AccumulatedPlayTime += DeltaTime;

	FVector Origin;
	FVector Extent;

	bool bProcessTargetOrigin = false;
	const bool bInvalidEntityHandle = SelectedWorkerUnitHandle.Index == INDEX_NONE || EntityManager->IsEntityValid(SelectedWorkerUnitHandle) == false;
	if ((bUpdatePathOnTick && HoveredActor == nullptr) || (HoveredActor == nullptr && bInvalidEntityHandle))
	{
		TargetTransform = Collision->GetComponentTransform();
		TargetTransform.SetScale3D(FVector{2.0, 2.0,1.0});
	}
	else if (HoveredActor != nullptr)
	{
		HoveredActor->GetActorBounds(true, Origin, Extent);
		bProcessTargetOrigin = true;
	}
	else // elif (bInvalidEntityHandle == false) 
	{
		const FTransform& InstanceTransform = GetEntityTransform(SelectedWorkerUnitHandle);
		Origin = InstanceTransform.GetLocation();
		Extent = ISMAgentComponent != nullptr ? ISMAgentComponent->GetStaticMesh()->GetBounds().BoxExtent : FVector{30};
		bProcessTargetOrigin = true;
	}

	if (bProcessTargetOrigin)
	{
		Extent.Z = 0;
		Origin.Z += 20.0;
		const double CursorTargetScalar = Extent.GetAbsMax() / TargetPadding;
		const double CursorScalarOffset = FMath::Sin(AccumulatedPlayTime * CursorSelectedScalePhaseSpeed) * CursorSelectedSinScaleIntensity;
	
		const double CursorScalarFinalXY = CursorTargetScalar + CursorScalarOffset;
		const FVector ScalarXY = FVector{CursorScalarFinalXY, CursorScalarFinalXY, 1.0};

		const FTransform WorldSpace = CursorMesh->GetComponentTransform();
		//CursorMesh->GetInstanceTransform(0, WorldSpace,true);
		const FQuat OldQuat = WorldSpace.GetRotation();
		TargetTransform = FTransform{OldQuat, Origin, ScalarXY };
	}

	const FTransform WorldSpace =  CursorMesh->GetComponentTransform();	
	const FTransform InterpTransform = UKismetMathLibrary::TInterpTo(WorldSpace, TargetTransform, DeltaTime, SelectionRescaleSpeed);
	CursorMesh->SetWorldTransform(InterpTransform);
}


//
// Effects - Visuals 

void AGodHandPawn::RefreshPathingEffects()
{
	if (EntityManager->IsEntityValid(SelectedWorkerUnitHandle) == false || Collision == nullptr ) { return; }
	
	const FVector CollisionLocation = Collision->GetComponentLocation();
	const FTransform& TargetInstanceTransform = GetEntityTransform(SelectedWorkerUnitHandle);
	const FVector EntityLocation = TargetInstanceTransform.GetLocation(); 
	const FVector TargetLocation = CollisionLocation;
	const UNavigationPath* Navpath = UNavigationSystemV1::FindPathToLocationSynchronously(this, EntityLocation, TargetLocation);
	if (Navpath == nullptr || Navpath->PathPoints.IsEmpty()) { return; }

	PathPoints = Navpath->PathPoints;
	PathPoints[0] = EntityLocation;
	PathPoints.Last() = TargetLocation;
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NC_WorkerPath, FName("TargetPath"), PathPoints);
}


//
// Enhanced Input - Action functions

void AGodHandPawn::ActionMove_Implementation(const FInputActionValue& Value)
{
	const FVector2D& ImmutableMoveInput = Value.Get<FVector2D>();
	
	AddMovementInput(GetActorForwardVector(), ImmutableMoveInput.Y * 100);
	AddMovementInput(GetActorRightVector(), ImmutableMoveInput.X * 100);	
}

void AGodHandPawn::ActionMagnify_Implementation(const FInputActionValue& Value)
{
	const float ImmutableMoveInput = Value.Get<float>();
	MagnificationStrength = ImmutableMoveInput;
	UpdateMagnification();
}

void AGodHandPawn::ActionRotate_Implementation(const FInputActionValue& Value)
{
	const float ImmutableMoveInput = Value.Get<float>(); // rotate yaw
	const int8 Direction = ImmutableMoveInput > 0 ? 1 : -1;


	if (RotationDeque.Num() > 1 && (RotationDeque.Last() + Direction) == 0)
	{
		RotationDeque.PopLast();
	}
	else
	{
		// Truncated, should stay at 90 degree increments this way
		TargetYaw = static_cast<int32>(GetActorRotation().Yaw + 0.5 + (90.0 * Direction));
		RotationDeque.EmplaceLast(Direction);
	}

	if (bIsInRotation == false)
	{
		CurrentRotationLeft = 90.0;
	}


	bIsInRotation = true;
}

void AGodHandPawn::ActionDragMove_Implementation(const FInputActionValue& Value)
{
	// Handles click&drag, does not handle touch&drag
	UE_LOG(LogTemp, Warning, TEXT("AGodHandPawn::ActionDragMove"))
	
	
	TrackUnitMovement();
}

void AGodHandPawn::ActionWorkerUnit_Started_Implementation(const FInputActionValue& Value)
{
	// const bool ImmutableMoveInput = Value.Get<bool>();
	UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Started"))
	
	
	if (SelectedWorkerUnitHandle.Index != INDEX_NONE)
	{
		if (NS_WorkerPath != nullptr && NC_WorkerPath == nullptr)
		{
			NC_WorkerPath = UNiagaraFunctionLibrary::SpawnSystemAttached(NS_WorkerPath, SceneRoot, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::SnapToTarget, false);
			bUpdatePathOnTick = true;
		}
		else if (NC_WorkerPath != nullptr)
		{
			NC_WorkerPath->ReinitializeSystem();
			NC_WorkerPath->Activate();
			bUpdatePathOnTick = true;
		}
		return;
	}

	ARTSOController* PC = GetController<ARTSOController>();
	if (PC != nullptr)
	{
		PC->MarqueeSelection(EMarqueeSelectionEvent::STARTMARQUEE);
	}	
	
	
	// else
	// AddMappingContext(GetController<APlayerController>(), TAG_CTRL_Ctxt_DragMove);
}

void AGodHandPawn::ActionWorkerUnit_Triggered_Implementation(const FInputActionValue& Value)
{
	// const bool ImmutableMoveInput = Value.Get<bool>(); 

	// @todo write class to throttle output in functions that run at high rate
	// UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Triggered"))

	ARTSOController* PC = GetController<ARTSOController>();
	if (PC != nullptr && PC->IsDrawingMarquee())
	{
		PC->MarqueeSelection(EMarqueeSelectionEvent::HOLDMARQUEE);
	}	
	
	WorkerUnitActionTarget = HoveredActor; 
}

void AGodHandPawn::ActionWorkerUnit_Cancelled_Implementation(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Cancelled"))
	IRTSOInputInterface::Execute_ActionWorkerUnit_Completed(this, Value);
}

//
// @thoughts: How to handle selected units?
// - How to deselect them intuitively
// -- Escape needs to be mapped to menu, so can't be used to de-selecting
// -- 

void AGodHandPawn::ActionWorkerUnit_Completed_Implementation(const FInputActionValue& Value)
{
	// const bool ImmutableMoveInput = Value.Get<bool>();
	ARTSOController* PC = GetController<ARTSOController>();
	if (PC == nullptr || PC->IsValidLowLevelFast() == false) { return; }

	if (PC->IsDrawingMarquee())
	{
		PC->MarqueeSelection(EMarqueeSelectionEvent::RELEASEMARQUEE);
		return;
	}
	PC->DeactivateMappingContext(TAG_CTRL_Ctxt_DragMove);
	
	const FGameplayTagContainer AssociatedTags = WorkerUnitActionTarget != nullptr && WorkerUnitActionTarget->Implements<UPDInteractInterface>()
		? IPDInteractInterface::Execute_GetGenericTagContainer(WorkerUnitActionTarget)
		: FGameplayTagContainer{}; 

	if (NC_WorkerPath != nullptr) { NC_WorkerPath->Deactivate(); }
	if (AssociatedTags.IsEmpty() || EntityManager->IsEntityValid(SelectedWorkerUnitHandle) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Completed -- Unexpected error -- Clearing selected worker unit "))

		bUpdatePathOnTick = false;
		WorkUnitTargetLocation = FVector::ZeroVector;
		WorkerUnitActionTarget = nullptr;
		return;
	}

	FVector2D ScreenCoordinates;
	bool bFoundInputType;

	FMassInt16Vector FallbackTargetLocation{};
	if (WorkerUnitActionTarget == nullptr)
	{
		FHitResult HitResult = PC->ProjectMouseToGroundPlane(PC->DedicatedLandscapeTraceChannel, ScreenCoordinates, bFoundInputType);
		FallbackTargetLocation = HitResult.bBlockingHit ? FMassInt16Vector{HitResult.Location} : FMassInt16Vector{};
	}


	const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& ISMs = UPDRTSBaseSubsystem::GetMassISMs(GetWorld());
	if (ISMs.IsEmpty() == false && Cast<UPDRTSBaseUnit>(ISMs[0]))
	{
		FPDTargetCompound OptTarget = {InvalidHandle, FallbackTargetLocation, WorkerUnitActionTarget};
		Cast<UPDRTSBaseUnit>(ISMs[0])->RequestAction(PC->GetActorID(), OptTarget, AssociatedTags.GetByIndex(0), SelectedWorkerUnitHandle);
	}
	
	FPDMFragment_RTSEntityBase* PermadevEntityBase = EntityManager->GetFragmentDataPtr<FPDMFragment_RTSEntityBase>(SelectedWorkerUnitHandle);
	if (PermadevEntityBase != nullptr)
	{
		// The initial call to the walk task will overwrite this in case it has an entry,
		// but if it is empty it will default to using a shared path instead
		PermadevEntityBase->QueuedUnitPath.Emplace(FVector::ZeroVector);
		PermadevEntityBase->OwnerID = PC->GetActorID();
	}
	
	if (NC_WorkerPath != nullptr) { NC_WorkerPath->Deactivate(); }
	bUpdatePathOnTick = false;
	SelectedWorkerUnitHandle = {INDEX_NONE, INDEX_NONE};
	UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Completed -- Clearing selected worker unit "))
	
}

void AGodHandPawn::ActionBuildMode_Implementation(const FInputActionValue& Value)
{
	// const bool ImmutableMoveInput = Value.Get<bool>(); 
}

void AGodHandPawn::ActionClearSelection_Implementation(const FInputActionValue& Value)
{
}

// Todo:      I need to write a shared fragment which holds four values,
// Todo cont: a location or target, a selection group index, and a boolean to tell if that shared fragment is valid for the selection index
// Todo cont: This also requires some fomr of selection grouping and controls for it
void AGodHandPawn::ActionMoveSelection_Implementation(const FInputActionValue& Value)
{
	FString BuildString = FString::Printf(TEXT("AGodHandPawn(%s)::ActionMoveSelection -- "), *GetName());
	// const bool ImmutableMoveInput = Value.Get<bool>();
	ARTSOController* PC = GetController<ARTSOController>();
	if (PC == nullptr || PC->IsValidLowLevelFast() == false)
	{
		// BuildString += FString::Printf(TEXT("\n PC INVALID "));
		// UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);
		return;
	}
	
	const int32 CurrentGroupID = PC->GetCurrentGroupID();
	if (PC->GetMarqueeSelectionMap().Contains(CurrentGroupID) == false)
	{
		return;
	}
	
	const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& ISMs = UPDRTSBaseSubsystem::GetMassISMs(GetWorld());
	if (ISMs.IsEmpty() || Cast<UPDRTSBaseUnit>(ISMs[0]) == nullptr)
	{
		// BuildString += FString::Printf(TEXT("\n ISM INVALID "));
		// UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);
		return;
	}
	
	// refresh hovered actor as target
	WorkerUnitActionTarget = HoveredActor;

	// go to mouse location is no valid target
	FMassInt16Vector SelectedLocation = InvalidLoc;

	FGameplayTagContainer FallbackContainer{TAG_AI_Job_Idle};
	if (WorkerUnitActionTarget == nullptr)
	{
		// BuildString += FString::Printf(TEXT("\n ACTOR (todo: OR ENTITY) TARGET INVALID, TRACING TO NEW STATIC LOCATION TARGET"));
		// UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);
		
		FVector2D ScreenCoordinates;
		bool bFoundInputType;

		FHitResult HitResult = PC->ProjectMouseToGroundPlane(PC->DedicatedLandscapeTraceChannel, ScreenCoordinates, bFoundInputType);
		if (HitResult.bBlockingHit)
		{
			FallbackContainer = FGameplayTagContainer{TAG_AI_Job_WalkToTarget};
			SelectedLocation.Set(HitResult.Location);
		}
	}
	
	//
	// Params for entity AI
	const FPDTargetCompound OptTarget = {InvalidHandle, SelectedLocation, WorkerUnitActionTarget};
	const FGameplayTagContainer AssociatedTags = WorkerUnitActionTarget != nullptr && WorkerUnitActionTarget->Implements<UPDInteractInterface>()
		? IPDInteractInterface::Execute_GetGenericTagContainer(WorkerUnitActionTarget)
		: FallbackContainer;
	
	const FHitResult& Start = PC->GetLatestStartHitResult();
	const FHitResult& Center = PC->GetLatestCenterHitResult();
	const FHitResult& End    = PC->GetLatestEndHitResult();
	const FVector StartLocation =
		Center.bBlockingHit  ? Center.Location
		: Start.bBlockingHit ? Start.Location
		: End.bBlockingHit   ? StartLocation
		: FVector::ZeroVector;
	
	Cast<UPDRTSBaseUnit>(ISMs[0])->RequestActionMulti(
		PC->GetActorID(),
		OptTarget,
		AssociatedTags.GetByIndex(0),
		*PC->GetMarqueeSelectionMap().Find(CurrentGroupID), 
		StartLocation,
		CurrentGroupID);
}

void AGodHandPawn::ActionAssignSelectionToHotkey_Implementation(const FInputActionValue& Value)
{

}

void AGodHandPawn::ActionHotkeySelection_Implementation(const FInputActionValue& Value)
{
	
}

void AGodHandPawn::ActionChordedBase_Implementation(const FInputActionValue& Value)
{

}

//
// Utility functions

FMassEntityHandle AGodHandPawn::OctreeEntityTrace(const FVector& StartLocation, const FVector& EndLocation)
{
	UPDRTSBaseSubsystem* RTSBaseSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	PD::Mass::Entity::FPDSafeOctree& WorldOctree = RTSBaseSubsystem->WorldOctree;
	if (WorldOctree.GetNumNodes() == 0) { return FMassEntityHandle(); } // @todo possibly just replace with an ensure
	
	const FVector Direction = EndLocation - StartLocation;

	// Halving extents
	const FBoxCenterAndExtent QueryBounds = FBoxCenterAndExtent(StartLocation + (Direction / 2), (Direction.GetAbs() / 2));

	FOctreeElementId2* ClosestIDSlow{}; 
	double ClosestDistanceSlow{AGodHandPawn::InvalidDistance};

	WorldOctree.FindElementsWithBoundsTest(QueryBounds, [&](const FPDEntityOctreeCell& Cell)
	{
		if (Cell.EntityHandle.Index == INDEX_NONE) { return; } 
		
		const double Delta = (EndLocation - Cell.Bounds.GetBox().GetCenter()).Length();
		ClosestIDSlow = Delta < ClosestDistanceSlow ? Cell.SharedOctreeID.Get() : ClosestIDSlow;
		ClosestDistanceSlow = Delta < ClosestDistanceSlow ? Delta : ClosestDistanceSlow;
	}, true);

	const double MinDistance = Collision->GetScaledSphereRadius();  // Not valid selection if above this
	FMassEntityHandle RetHandleSlow{INDEX_NONE, INDEX_NONE};
	FVector ClosestVectorSlow{AGodHandPawn::InvalidVector};
	if (ClosestIDSlow != nullptr && ClosestDistanceSlow <= MinDistance)
	{
		const FPDEntityOctreeCell& Cell = static_cast<FPDEntityOctreeCell&>(WorldOctree.GetElementById(*ClosestIDSlow));
		ClosestVectorSlow = Cell.Bounds.GetBox().GetCenter();
		RetHandleSlow = Cell.EntityHandle;
	}

#if 1

#if CHAOS_DEBUG_DRAW
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_InteractGodHandDraw)
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(ClosestVectorSlow, FVector(25.0), FQuat::Identity, FColor::White, false, 0, 0, 1.0f);
		const FVector& TextLocation = ClosestVectorSlow + FVector(0, 0, (FVector(25.0).Z * 2));
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugString(TextLocation,FString("Hovered Entity"), nullptr, FColor::Blue, 0, true, 2);
	}
#endif
	
#endif

	return RetHandleSlow;
}


AActor* AGodHandPawn::FindClosestInteractableActor() const
{
	TArray<AActor*> Overlap;
	Collision->GetOverlappingActors(Overlap);
	
	AActor* ClosestActor = Overlap.IsEmpty() == false ? Overlap[0] : nullptr;
	for (AActor* FoundActor : Overlap)
	{
		const IPDInteractInterface* AsInterface = Cast<IPDInteractInterface>(FoundActor);
		if (AsInterface == nullptr) { continue; }

		// Check type of interactable here, is a special actor unit/character or a world-item
		
		const float LengthToOldActor = (ClosestActor->GetActorLocation() - Collision->GetComponentLocation()).Length();
		const float LengthToNewActor = (FoundActor->GetActorLocation() - Collision->GetComponentLocation()).Length();
		if (LengthToNewActor < LengthToOldActor)
		{
			ClosestActor = FoundActor; // <- find closest actor and store in this
		}
	}
	return ClosestActor;
}

const FTransform& AGodHandPawn::GetEntityTransform(const FMassEntityHandle& Handle) const
{
	// For our purposes transform fragment should never be nullptr in case the handle itself is valid
	check(EntityManager->GetFragmentDataPtr<FTransformFragment>(Handle) != nullptr);
	return EntityManager->IsEntityValid(Handle) ? EntityManager->GetFragmentDataPtr<FTransformFragment>(Handle)->GetTransform() : Collision->GetComponentTransform();
}

FMassEntityHandle AGodHandPawn::FindClosestMeshInstance()
{
	const ARTSOController* PC = GetController<ARTSOController>();
	if (PC == nullptr || PC->IsValidLowLevelFast() == false) { return FMassEntityHandle{INDEX_NONE,INDEX_NONE}; }
	
	FHitResult HitResult;
	bool bFoundInputType;
	FVector2D ScreenCoordinates;
	FVector LocalWorkUnitTargetLocation{};
	
	PC->ProjectMouseToGroundPlane(HitResult, ECollisionChannel::ECC_Visibility, ScreenCoordinates, LocalWorkUnitTargetLocation, bFoundInputType);
	return OctreeEntityTrace(HitResult.TraceStart, LocalWorkUnitTargetLocation); 
}

//
// Boilerplate - Setup

AGodHandPawn::AGodHandPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneRoot  = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	InteractComponent  = CreateDefaultSubobject<UPDInteractComponent>(TEXT("InteractComponent"));
	InventoryComponent = CreateDefaultSubobject<UPDInventoryComponent>(TEXT("InventoryComponent"));
	
	Springarm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Springarm"));
	Springarm->SetupAttachment(SceneRoot);
	Springarm->TargetArmLength = 800.0f;
	Springarm->SocketOffset = FVector{250.0f, 0, 0};
	Springarm->CameraLagSpeed = 5.0f;
	Springarm->bEnableCameraRotationLag = true;
	Springarm->bDoCollisionTest = false;
	
	Camera     = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Springarm);
	Camera->FieldOfView = 25.0f;
	
	CursorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CursorMesh"));
	CursorMesh->SetupAttachment(SceneRoot);
	CursorMesh->SetGenerateOverlapEvents(false);
	CursorMesh->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);

	Collision  = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetupAttachment(SceneRoot);
	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
	ParticipantComponent = CreateDefaultSubobject<UConversationParticipantComponent>(TEXT("ConversationParticipantComponent"));	
}

void AGodHandPawn::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &AGodHandPawn::OnCollisionBeginOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &AGodHandPawn::OnCollisionEndOverlap);
	EntityManager = &GetWorld()->GetSubsystem<UMassEntitySubsystem>()->GetEntityManager();

	UpdateMagnification();
	InitializeISMAgent();



	// There is a bug causing any modifier I am applying to the IA to be called but the modified value to be discarded,
	// I have however confirmed that the value in here at-least is correct
	// A temporary way around it maybe have to be a subsystem or other singleton which caches a stack of modifier values 	
	URTSOInputStackSubsystem* InputStackWorkaround = GetWorld()->GetSubsystem<URTSOInputStackSubsystem>();
	InputStackWorkaround->DispatchValueStackReset();
}

void AGodHandPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// CursorMesh->AddInstance(Transform);
	
	//MarqueeSelectedHandles
}

void AGodHandPawn::InitializeISMAgent()
{
	const FLatentActionInfo DelayInfo{0,0, TEXT("InitializeISMAgent"), this};
	if (UPDRTSBaseSubsystem::GetMassISMs(GetWorld()).IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("AGodHandPawn::InitializeISMAgent - Dispatching another attempt in 1 second"))

		UKismetSystemLibrary::Delay(GetOwner(), 1.0f, DelayInfo);
		return;
	}
	
	// @todo debug why calling this invalidates the whole octree
	// GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>()->SetupOctreeWithNewWorld(GetWorld());
	
	ISMAgentComponent = UPDRTSBaseSubsystem::GetMassISMs(GetWorld())[0];
	UPDRTSBaseUnit* AsBaseUnit = Cast<UPDRTSBaseUnit>(ISMAgentComponent);
	check(AsBaseUnit != nullptr)
	
	AsBaseUnit->SetEntityManager(EntityManager);
}

void AGodHandPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (GetWorld() == nullptr) { return; }
	
	// There is a bug causing any modifier I am applying to the IA to be called but the modified value to be discarded,
	// I have however confirmed that the value in here at-least is correct
	// A temporary way around it maybe have to be a subsystem or other singleton which caches a stack of modifier values 	
	URTSOInputStackSubsystem* InputStackWorkaround = GetWorld()->GetSubsystem<URTSOInputStackSubsystem>();
	InputStackWorkaround->DispatchValueStackReset();	
}

//
// Boilerplate - Collisions
void AGodHandPawn::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("AGodHandPawn::OnCollisionBeginOverlap"))
	
	// neither an ai or any interactable
	if (Cast<IPDInteractInterface>(OtherActor) == nullptr) { return; }
	HoveredActor = OtherActor;
}

void AGodHandPawn::ActorEndOverlapValidation()
{
	TArray<AActor*> Overlap;
	GetOverlappingActors(Overlap, AActor::StaticClass());
	HoveredActor = Overlap.IsEmpty() ? nullptr : HoveredActor;
}

void AGodHandPawn::OnCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ActorEndOverlapValidation();
}

void AGodHandPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// not any interactable 
	if (Cast<IPDInteractInterface>(OtherActor) == nullptr)
	{
		HoveredActor = OtherActor;
		return;
	}
	
	ISMAgentComponent = OtherActor->GetComponentByClass<UPDRTSBaseUnit>(); 
}

void AGodHandPawn::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	ActorEndOverlapValidation();
}

void AGodHandPawn::BeginBuild_Implementation(TSubclassOf<AActor> TargetClass, TMap<FGameplayTag, FPDItemCosts>& ResourceCost)
{
	TempSpawnClass = TargetClass;
	CurrentResourceCost = ResourceCost;

	if (GetWorld() == nullptr
		|| SpawnedInteractable == nullptr
		|| SpawnedInteractable->IsValidLowLevelFast() == false)
	{
		return;
	}
	SpawnedInteractable->Destroy(true);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.TransformScaleMethod = ESpawnActorScaleMethod::SelectDefaultAtRuntime;
	SpawnInfo.Owner = this;
	SpawnInfo.Instigator = this;
	SpawnInfo.bDeferConstruction = false;

	const FVector ActorLocation = GetActorLocation();
	SpawnedInteractable = Cast<APDInteractActor>(GetWorld()->SpawnActor(TempSpawnClass, &ActorLocation, &FRotator::ZeroRotator, SpawnInfo));
	if (SpawnedInteractable == nullptr) { return; }

	// SpawnedInteractable: Placement Mod e
	// SpawnedInteractable->CreateOverlay
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
