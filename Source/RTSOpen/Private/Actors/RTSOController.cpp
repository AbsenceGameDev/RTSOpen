/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "Actors/RTSOController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MassCrowdRepresentationSubsystem.h"
#include "NativeGameplayTags.h"
#include "PDRTSBaseSubsystem.h"
#include "PDRTSCommon.h"
#include "Actors/GodHandPawn.h"
#include "AI/Mass/PDMassFragments.h"
#include "AI/Mass/PDMassProcessors.h"
#include "Chaos/DebugDrawQueue.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Misc/CString.h"


ARTSOController::ARTSOController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), bIsDrawingMarquee(0)
{
	bShowMouseCursor = true;
	bEnableMouseOverEvents = true;

	// Enforce these entries as default, need to actually be linked with mapping context assets in editor 
	MappingContexts.Emplace(TAG_CTRL_Ctxt_BaseInput);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_ConversationMode);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_WorkerUnitMode);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_DragMove);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_BuildMode);

	HitResultTraceDistance = 10000000.0;
}

void ARTSOController::BeginPlay()
{
	Super::BeginPlay();
	
	ActivateMappingContext(TAG_CTRL_Ctxt_BaseInput);
	ActivateMappingContext(TAG_CTRL_Ctxt_WorkerUnitMode);
}

void ARTSOController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* AsEnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (AsEnhancedInput == nullptr) { return; }
	AsEnhancedInput->BindAction(CtrlActionMove, ETriggerEvent::Triggered, this, &ARTSOController::ActionMove_Implementation);
	AsEnhancedInput->BindAction(CtrlActionMagnify, ETriggerEvent::Triggered, this, &ARTSOController::ActionMagnify_Implementation);	
	AsEnhancedInput->BindAction(CtrlActionRotate, ETriggerEvent::Started, this, &ARTSOController::ActionRotate_Implementation);
	AsEnhancedInput->BindAction(CtrlActionDragMove, ETriggerEvent::Triggered, this, &ARTSOController::ActionDragMove_Implementation);	

	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Triggered, this, &ARTSOController::ActionWorkerUnit_Triggered_Implementation);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Started, this, &ARTSOController::ActionWorkerUnit_Started_Implementation);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Canceled, this, &ARTSOController::ActionWorkerUnit_Cancelled_Implementation);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Completed, this, &ARTSOController::ActionWorkerUnit_Completed_Implementation);	
	AsEnhancedInput->BindAction(CtrlActionClearSelection, ETriggerEvent::Triggered, this, &ARTSOController::ActionClearSelection_Implementation);	
	AsEnhancedInput->BindAction(CtrlActionMoveSelection, ETriggerEvent::Triggered, this, &ARTSOController::ActionMoveSelection_Implementation);	

	AsEnhancedInput->BindAction(CtrlActionBuildMode, ETriggerEvent::Triggered, this, &ARTSOController::ActionBuildMode_Implementation);		
}

void ARTSOController::OverwriteMappingContextSettings(const FNativeGameplayTag& ContextTag, UInputMappingContext* NewContext)
{
	OverwriteMappingContextSettings(ContextTag.GetTag(), NewContext);
}

void ARTSOController::OverwriteMappingContextSettings(const FGameplayTag& ContextTag, UInputMappingContext* NewContext)
{
	if (ContextTag.IsValid() == false)
	{
		const FString BuildString = FString::Printf(TEXT("ARTSOController(%s)::OverwriteMappingContext -- "), *GetName())
		+ FString::Printf(TEXT("\n Input tag was invalid. Skipping processing entry "));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);
		return;
	}
	
	MappingContexts.FindOrAdd(ContextTag) = NewContext;
}

void ARTSOController::ActivateMappingContext(const FNativeGameplayTag& ContextTag)
{
	ActivateMappingContext(ContextTag.GetTag());
}

void ARTSOController::ActivateMappingContext(const FGameplayTag& ContextTag)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem == nullptr) { return; }

	if (MappingContexts.Contains(ContextTag) == false)
	{
		const FString BuildString = FString::Printf(TEXT("ARTSOController(%s)::AddMappingContext -- "), *GetName())
		+ FString::Printf(TEXT("\n Failed to find a valid mapping context mapped to tag (%s). Skipping processing entry "), *ContextTag.GetTagName().ToString());
		UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);
		return;
	}
	
	const FModifyContextOptions ModifyOptions{};
	Subsystem->AddMappingContext(*MappingContexts.Find(ContextTag), 0, ModifyOptions);
}

void ARTSOController::DeactivateMappingContext(const FNativeGameplayTag& ContextTag)
{
	DeactivateMappingContext(ContextTag.GetTag());
}

void ARTSOController::DeactivateMappingContext(const FGameplayTag& ContextTag)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem == nullptr) { return; }

	if (MappingContexts.Contains(ContextTag) == false)
	{
		const FString BuildString = FString::Printf(TEXT("ARTSOController(%s)::RemoveMappingContext -- "), *GetName())
		+ FString::Printf(TEXT("\n Failed to find a valid mapping context mapped to tag (%s). Skipping processing entry "), *ContextTag.GetTagName().ToString());
		UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);
		return;
	}

	Subsystem->RemoveMappingContext(*MappingContexts.Find(ContextTag));
}

void ARTSOController::ActionMove_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionMove(GetPawn(), Value);
}
void ARTSOController::ActionMagnify_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionMagnify(GetPawn(), Value);
}
void ARTSOController::ActionRotate_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionRotate(GetPawn(), Value);
}
void ARTSOController::ActionDragMove_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionDragMove(GetPawn(), Value);
}
void ARTSOController::ActionWorkerUnit_Triggered_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionWorkerUnit_Triggered(GetPawn(), Value);
}
void ARTSOController::ActionWorkerUnit_Started_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionWorkerUnit_Started(GetPawn(), Value);
}
void ARTSOController::ActionWorkerUnit_Cancelled_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionWorkerUnit_Cancelled(GetPawn(), Value);
}
void ARTSOController::ActionWorkerUnit_Completed_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionWorkerUnit_Completed(GetPawn(), Value);
}
void ARTSOController::ActionBuildMode_Implementation(const FInputActionValue& Value)
{
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionBuildMode(GetPawn(), Value);
}

void ARTSOController::ActionClearSelection_Implementation(const FInputActionValue& Value)
{
	IRTSOInputInterface::ActionClearSelection_Implementation(Value);

	OnSelectionChange(true);
	MarqueeSelectedHandles.Empty();
	TArray<int32> NoKeys{};
	OnMarqueeSelectionUpdated(NoKeys);
	
	if (GetPawn() == nullptr || GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false) { return; }
	IRTSOInputInterface::Execute_ActionClearSelection(GetPawn(), Value);	
}


void ARTSOController::ActionMoveSelection_Implementation(const FInputActionValue& Value)
{
	if (MarqueeSelectedHandles.IsEmpty()
		|| GetPawn() == nullptr
		|| GetPawn()->GetClass()->ImplementsInterface(URTSOInputInterface::StaticClass()) == false)
	{
		const FString BuildString = FString::Printf(TEXT("ARTSOController(%s)::ActionMoveSelection -- "), *GetName())
		+ FString::Printf(TEXT("\n MarqueeSelectedHandles.Num: %i, GetPawn(): %i"), MarqueeSelectedHandles.Num(), GetPawn() != nullptr);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);
		
		return;
	}
	
	IRTSOInputInterface::Execute_ActionMoveSelection(GetPawn(), Value);
}

//
// Mouse projections and marquee selection, @todo possibly move to controller class

void ARTSOController::ProjectMouseToGroundPlane(FVector2D& ScreenCoordinates, FVector& IntersectionPoint, bool& bFoundInputType) const
{
	ProjectMouseToGroundPlane(DedicatedLandscapeTraceChannel, ScreenCoordinates, IntersectionPoint, bFoundInputType);
}

void ARTSOController::ProjectMouseToGroundPlane(TEnumAsByte<ECollisionChannel> OverrideTraceChannel, FVector2D& ScreenCoordinates, FVector& IntersectionPoint, bool& bFoundInputType) const
{
	const FHitResult& Result = ProjectMouseToGroundPlane(OverrideTraceChannel, ScreenCoordinates, bFoundInputType);
	if (Result.bBlockingHit)
	{
		IntersectionPoint = Result.Location;
		return;
	}
	
	FVector WorldLocation{}, WorldDirection{};
	DeprojectScreenPositionToWorld(ScreenCoordinates.X, ScreenCoordinates.Y, WorldLocation, WorldDirection);
	IntersectionPoint = FMath::LinePlaneIntersection(
	WorldLocation, 
	WorldLocation + (WorldDirection * 10000000.0),
	GetPawn()->GetActorLocation(),
	FVector{0.0, 0.0, 1.0});
}

void ARTSOController::ProjectMouseToGroundPlane(FHitResult& HitResult, TEnumAsByte<ECollisionChannel> OverrideTraceChannel, FVector2D& ScreenCoordinates, FVector& IntersectionPoint, bool& bFoundInputType) const
{
	HitResult = ProjectMouseToGroundPlane(OverrideTraceChannel, ScreenCoordinates, bFoundInputType);
	if (HitResult.bBlockingHit)
	{
		IntersectionPoint = HitResult.Location;
		return;
	}
	
	FVector WorldLocation{}, WorldDirection{};
	DeprojectScreenPositionToWorld(ScreenCoordinates.X, ScreenCoordinates.Y, WorldLocation, WorldDirection);
	IntersectionPoint = FMath::LinePlaneIntersection(
	WorldLocation, 
	WorldLocation + (WorldDirection * 10000000.0),
	GetPawn()->GetActorLocation(),
	FVector{0.0, 0.0, 1.0});
}

FHitResult ARTSOController::ProjectMouseToGroundPlane(TEnumAsByte<ECollisionChannel> OverrideTraceChannel, FVector2D& ScreenCoordinates, bool& bFoundInputType) const
{
	// Lock projection to center screen
	int32 SizeX = 0, SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	FVector2D Size2D{static_cast<double>(SizeX), static_cast<double>(SizeY)};
	Size2D *= 0.5;
	
	// Mouse viewport coordinates
	float LocX = 0, LocY = 0;
	const bool bFoundMouse = GetMousePosition(LocX, LocY);
	FVector2D Coords2D{LocX, LocY};
	ScreenCoordinates = bFoundMouse ? Coords2D : Size2D;
	bFoundInputType = bFoundMouse;
	
	FHitResult FloorHitResult{};
	FCollisionQueryParams Params;
	Params.MobilityType = EQueryMobilityType::Static;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(GetPawn());
	
	GetHitResultAtScreenPosition(ScreenCoordinates, OverrideTraceChannel, Params, FloorHitResult);
	return FloorHitResult;
}

void ARTSOController::DistanceFromViewportCenter(const FVector2D& InMoveDirection, FVector& Direction, double& Strength) const
{
	// Viewport halfsize
	int32 iX=0,iY=0;
	GetViewportSize(iX,iY);
	const float fX=iX, fY=iY;

	const double XViewportHalfsize = (fX * 0.5) - ScreenEdgeMovementPadding;
	const double YViewportHalfsize = (fY * 0.5) - ScreenEdgeMovementPadding;
	
	const double DeadzoneX = FMath::Max((FMath::Abs(InMoveDirection.X) - XViewportHalfsize), 0) / ScreenEdgeMovementPadding;
	const double DeadzoneY = FMath::Max((FMath::Abs(InMoveDirection.Y) - YViewportHalfsize), 0) / ScreenEdgeMovementPadding;	
	
	Direction.X = FMath::Sign(InMoveDirection.Y) * -1 * DeadzoneY;
	Direction.Y = FMath::Sign(InMoveDirection.X) * DeadzoneX;
	Strength = 1.0f;
}

void ARTSOController::ClampMovementToScreen(FVector& MoveDirection, double& Strength) const
{
	FVector2D ScreenCoordinates;
	FVector IntersectionPoint;
	bool bFoundInputType;
	ProjectMouseToGroundPlane(ScreenCoordinates,IntersectionPoint, bFoundInputType);

	int32 X=0,Y=0;
	GetViewportSize(X, Y);
	const FVector2D& ViewportSize = FVector2D{static_cast<double>(X),static_cast<double>(Y)} * 2.0;

	const FVector2D& DeltaScreenCoords = ScreenCoordinates - ViewportSize;
	DistanceFromViewportCenter(DeltaScreenCoords, MoveDirection, Strength);

	MoveDirection = UKismetMathLibrary::TransformDirection( GetPawn()->GetActorTransform(), MoveDirection);
}


void ARTSOController::MarqueeSelection(EMarqueeSelectionEvent SelectionEvent)
{
	// Viewport halfsize
	
	// Lock projection to center screen
	int32 SizeX = 0, SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	FVector2D Size2D{static_cast<double>(SizeX), static_cast<double>(SizeY)};
	Size2D *= 0.5;
	
	// Mouse viewport coordinates
	float LocX = 0, LocY = 0;
	const bool bFoundInputType = GetMousePosition(LocX, LocY);
	const FVector2D Coords2D{LocX, LocY};
	const FVector2D ScreenCoordinates = bFoundInputType ? Coords2D : Size2D;
	
	switch (SelectionEvent)
	{
	case EMarqueeSelectionEvent::STARTMARQUEE:
		StartMousePositionMarquee = CurrentMousePositionMarquee = ScreenCoordinates;
		bIsDrawingMarquee = true;
		break;
	case EMarqueeSelectionEvent::HOLDMARQUEE:
		CurrentMousePositionMarquee = ScreenCoordinates;
		break;
	case EMarqueeSelectionEvent::RELEASEMARQUEE:
		{
			GetEntitiesOrActorsInMarqueeSelection();
			TArray<int32> Keys;
			MarqueeSelectedHandles.GetKeys(Keys);
			OnMarqueeSelectionUpdated(Keys);
			bIsDrawingMarquee = false;
		}
		break;
	}
}

void ARTSOController::DrawBoxAndTextChaos(const FVector BoundsCenter, FQuat Rotation, const FVector DebugExtent, const FString DebugBoxTitle, FColor LineColour)
{
#if CHAOS_DEBUG_DRAW
	constexpr int32 SeedOffset = 0x00a7b95; // just some random number with little significance, used to offset value so a seed based on the same hex colour code
	static int32 StaticShift = 0x0; // just some random number with little significance, used to offset value so a seed based on the same hex colour code
	StaticShift = (StaticShift + 0x1) % 0x7;
	
	Chaos::FDebugDrawQueue::GetInstance()
		.DrawDebugBox(
			BoundsCenter,
			DebugExtent,
			Rotation,
			LineColour,
			false,
			20,
			0,
			5.0
		);
	Chaos::FDebugDrawQueue::GetInstance()
		.DrawDebugString(
			BoundsCenter + FVector(0, 0, (DebugExtent.Z * 2)),
			DebugBoxTitle,
			nullptr,
			FColor::MakeRandomSeededColor((LineColour.R < StaticShift) + (LineColour.G < StaticShift) + (LineColour.B < StaticShift) + (SeedOffset < StaticShift)),
			20,
			true,
			2);

#endif // CHAOS_DEBUG_DRAW
}

void ARTSOController::AdjustMarqueeHitResultsToMinimumHeight(FHitResult& StartHitResult, FHitResult& CenterHitResult, FHitResult& EndHitResult)
{
	const double TargetZ = FMath::Min3(StartHitResult.Location.Z, CenterHitResult.Location.Z, EndHitResult.Location.Z);
	FVector StartHitDirection = (StartHitResult.Location - StartHitResult.TraceStart).GetSafeNormal();
	FVector CenterHitDirection = (CenterHitResult.Location - CenterHitResult.TraceStart).GetSafeNormal();
	FVector EndHitDirection = (EndHitResult.Location - EndHitResult.TraceStart).GetSafeNormal();

	if (StartHitResult.Location.Z > TargetZ + SMALL_NUMBER)
	{
		const double ZPerDirection = StartHitDirection.Z;
		const double ZToDecrease = StartHitResult.Location.Z - TargetZ;
		const double DirectionIncrease = ZToDecrease / ZPerDirection;
		
		StartHitResult.Location = StartHitResult.Location + (StartHitDirection * DirectionIncrease);
	}	
	if (CenterHitResult.Location.Z > TargetZ + SMALL_NUMBER)
	{
		const double ZPerDirection = CenterHitDirection.Z;
		const double ZToDecrease = CenterHitResult.Location.Z - TargetZ;
		const double DirectionIncrease = ZToDecrease / ZPerDirection;
		
		CenterHitResult.Location = CenterHitResult.Location + (CenterHitDirection * DirectionIncrease);		
	}	
	if (EndHitResult.Location.Z > TargetZ + SMALL_NUMBER)
	{
		const double ZPerDirection = EndHitDirection.Z;
		const double ZToDecrease = EndHitResult.Location.Z - TargetZ;
		const double DirectionIncrease = ZToDecrease / ZPerDirection;
		
		EndHitResult.Location = EndHitResult.Location + (EndHitDirection * DirectionIncrease);		
	}
}

void ARTSOController::GetEntitiesOrActorsInMarqueeSelection()
{
	// Viewport halfsize
	PD::Mass::Entity::FPDSafeOctree& WorldOctree =  GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>()->WorldOctree;

	FCollisionQueryParams Params;
	Params.MobilityType = EQueryMobilityType::Static;
	Params.AddIgnoredActor(this);

	// Sample three points at the ground, min their z component
	FHitResult StartHitResult{};
	FHitResult CenterHitResult{};
	FHitResult EndHitResult{};
	FVector2D CenterSelection = StartMousePositionMarquee + ((CurrentMousePositionMarquee - StartMousePositionMarquee) * 0.5) ; 
	GetHitResultAtScreenPosition(CenterSelection, ECC_Visibility, Params, CenterHitResult);
	GetHitResultAtScreenPosition(StartMousePositionMarquee, ECC_Visibility, Params, StartHitResult);
	GetHitResultAtScreenPosition(CurrentMousePositionMarquee, ECC_Visibility, Params, EndHitResult);
	
	// @todo Finish function below: Adjusts bounds in case on of our tracers hit something in-between in the foreground 
	// AdjustMarqueeHitResultsToMinimumHeight(StartHitResult, CenterHitResult, EndHitResult);
	
	FMinimalViewInfo OutResult;
	CalcCamera(GetWorld()->GetDeltaSeconds(), OutResult);
	
	const FVector Distance = EndHitResult.Location - StartHitResult.Location; 
	FQuat CameraRotation = Distance.GetSafeNormal().ToOrientationQuat();

	FVector BoundsCenter = CenterHitResult.Location;
	FVector Extent = FVector(Distance.X * 0.5, Distance.Y * 0.5, (StartHitResult.TraceStart - StartHitResult.Location).Z * 0.5);
	Extent = CameraRotation.RotateVector(Extent); // * -1;

	const FVector OffsetZVector = FVector{0,0, (StartHitResult.TraceStart - StartHitResult.Location).Z};
	const FVector OffsetXVector = FVector{Distance.X,0, 0};

	// assumes we are rotated along one of four cardinal directions
	FVector RotatedX = OutResult.Rotation.RotateVector(OffsetXVector);
	const FBox MarqueeWorldBox{
			{
				// Bottom points
				StartHitResult.Location,
				StartHitResult.Location + RotatedX,
				EndHitResult.Location,
				EndHitResult.Location   - RotatedX,

				// Upper Points
				StartHitResult.Location + OffsetZVector,
				StartHitResult.Location + OffsetZVector + RotatedX,
				EndHitResult.Location   + OffsetZVector,
				EndHitResult.Location   + OffsetZVector - RotatedX
			}};
	
	MarqueeWorldBox.GetCenterAndExtents(BoundsCenter, Extent);

	OnSelectionChange(true);
	MarqueeSelectedHandles.Empty(); // clear previous selection
	FBoxCenterAndExtent QueryBounds = FBoxCenterAndExtent(BoundsCenter, Extent);
	QueryBounds.Center.W = QueryBounds.Extent.W = 0;
	
	// QueryBounds.Extent.Y = 500; // Distance.Y * 0.5;
	WorldOctree.FindElementsWithBoundsTest(QueryBounds, [&](const FPDEntityOctreeCell& Cell)
	{
		if (Cell.EntityHandle.Index == INDEX_NONE) { return; } 

		MarqueeSelectedHandles.Emplace(Cell.EntityHandle.Index, Cell.EntityHandle);
	}, true);

	OnSelectionChange(false);
	
	
#if CHAOS_DEBUG_DRAW
	if (UPDOctreeProcessor::CVarDrawCells.GetValueOnAnyThread() == false) { return; }

	// Chaos debug draws on an async call
	const FVector DebugExtent = FVector(25.0);
	DrawBoxAndTextChaos(StartHitResult.Location, QueryBounds.Extent.ToOrientationQuat(), DebugExtent, FString("StartSelection"));
	DrawBoxAndTextChaos(CenterHitResult.Location, QueryBounds.Extent.ToOrientationQuat(), DebugExtent, FString("CenterSelection"));
	DrawBoxAndTextChaos(EndHitResult.Location, QueryBounds.Extent.ToOrientationQuat(), DebugExtent, FString("EndSelection"));
	DrawBoxAndTextChaos(FVector(QueryBounds.Center), FQuat(0) , FVector(QueryBounds.Extent), FString("MarqueeSelection"), FColor::Yellow);
	
#endif
	
}

void ARTSOController::OnSelectionChange(const bool bClearSelection)
{
	// Parallel task for enabling custom mesh data 
	
	AGodHandPawn* PawnAsGodhand = GetPawn<AGodHandPawn>();
	if (PawnAsGodhand == nullptr) { return; }
	
	// const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& ISMs = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>()->GetMassISMs(PawnAsGodhand->GetWorld());
	TArray<FMassEntityHandle> MarqueeSelectionArray;
	GetMarqueeSelectionMap().GenerateValueArray(MarqueeSelectionArray);

	for (const FMassEntityHandle& EntityHandle : MarqueeSelectionArray)
	{
		if (PawnAsGodhand->EntityManager->IsEntityValid(EntityHandle) == false) { return; }

		FPDMFragment_RTSEntityBase* PermadevEntityBase = PawnAsGodhand->EntityManager->GetFragmentDataPtr<FPDMFragment_RTSEntityBase>(EntityHandle);
		if (PermadevEntityBase == nullptr) { return; }
		
		PermadevEntityBase->SelectionState = bClearSelection ? EPDEntitySelectionState::ENTITY_NOTSELECTED : EPDEntitySelectionState::ENTITY_SELECTED; // Processing handled in UPDMProcessor_EntityCosmetics::Execute function
	}
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