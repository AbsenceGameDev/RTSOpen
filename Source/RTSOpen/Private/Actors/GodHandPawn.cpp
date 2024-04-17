/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

/* Permadev - Actors */
#include "Actors/GodHandPawn.h"
#include "Actors/PDInteractActor.h"
#include "Actors/RTSOController.h"
#include "Pawns/PDRTSBaseUnit.h"
#include "PDRTSCommon.h"

/* Permadev - Components */
#include "Components/PDInteractComponent.h"
#include "Components/PDInventoryComponent.h"

/* Engine - Components */
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"

/* Engine - Input */
#include "CommonInputTypeEnum.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

/* Engine - Effects */
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

#include "Kismet/KismetMathLibrary.h"

// Sets default values
AGodHandPawn::AGodHandPawn() : bTickHover(false)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneRoot  = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	InteractComponent  = CreateDefaultSubobject<UPDInteractComponent>(TEXT("InteractComponent"));
	InventoryComponent = CreateDefaultSubobject<UPDInventoryComponent>(TEXT("InventoryComponent"));
	
	Springarm  = CreateDefaultSubobject<USpringArmComponent>(TEXT("Springarm"));
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
	CursorMesh->SetCollisionProfileName("NoCollision");

	Collision  = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->SetupAttachment(SceneRoot);

	// Enforce these entries as default, need to actually be linked with mapping context assets in editor 
	MappingContexts.Emplace(TAG_CTRL_Ctxt_BaseInput);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_ConversationMode);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_WorkerUnitMode);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_DragMove);
	MappingContexts.Emplace(TAG_CTRL_Ctxt_BuildMode);
	
	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
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

	if (HoveredActor == nullptr)
	{
		TargetTransform = Collision->GetComponentTransform();
		TargetTransform.SetScale3D(FVector{2.0, 2.0,1.0});
	}
	else
	{
		FVector Origin;
		FVector Extent;
		HoveredActor->GetActorBounds(true, Origin, Extent);
		Extent.Z = 0;
		Origin.Z += 20.0;
	
		const double CursorTargetScalar = Extent.GetAbsMax() / TargetPadding;
		const double CursorScalarOffset = FMath::Sin(AccumulatedPlayTime * CursorSelectedScalePhaseSpeed) * CursorSelectedSinScaleIntensity;
	
		const double CursorScalarFinalXY = CursorTargetScalar + CursorScalarOffset;
		const FVector ScalarXY = FVector{CursorScalarFinalXY, CursorScalarFinalXY, 1.0};

		const FQuat OldQuat = CursorMesh->GetComponentRotation().Quaternion();
		TargetTransform = FTransform{OldQuat, Origin, ScalarXY };
		
	}

	const FTransform InterpTransform = UKismetMathLibrary::TInterpTo(CursorMesh->GetComponentTransform(), TargetTransform, DeltaTime, SelectionRescaleSpeed);
	CursorMesh->SetWorldTransform(InterpTransform);
}

void AGodHandPawn::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	//
	// @todo At this line: Bind input switch function
	
}

void AGodHandPawn::OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	// neither an ai or any interactabke
	if (Cast<APDRTSBaseUnit>(OtherActor) == nullptr && Cast<IPDInteractInterface>(OtherActor) == nullptr)
	{
		return; 
	}

	HoveredActor = OtherActor;
	bTickHover = true;
}

void AGodHandPawn::OnCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TArray<AActor*> Overlap;
	GetOverlappingActors(Overlap, AActor::StaticClass());
	HoveredActor = Overlap.IsEmpty() ? nullptr : HoveredActor; 
}	

void AGodHandPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	// neither an ai or any interactabke
	if (Cast<APDRTSBaseUnit>(OtherActor) == nullptr && Cast<IPDInteractInterface>(OtherActor) == nullptr)
	{
		return; 
	}

	HoveredActor = OtherActor;
	bTickHover = true;
}

void AGodHandPawn::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	TArray<AActor*> Overlap;
	GetOverlappingActors(Overlap, AActor::StaticClass());
	HoveredActor = Overlap.IsEmpty() ? nullptr : HoveredActor; 
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

void AGodHandPawn::HoverTick(const float DeltaTime)
{
	if (bTickHover == false) { return; }
	
	TArray<AActor*> Overlap;
	Collision->GetOverlappingActors(Overlap);
	bTickHover = Overlap.IsEmpty() == false; // keep ticking until this is not true anymore
	
	// Find interactables, could be items, could be other pawns
	AActor* ClosestActor = bTickHover ? Overlap[0] : nullptr;
	for (AActor* FoundActor : Overlap)
	{

		const IPDInteractInterface* AsInterface = Cast<IPDInteractInterface>(FoundActor);
		const APDRTSBaseUnit* AsWorker    = Cast<APDRTSBaseUnit>(FoundActor);
		if (AsInterface == nullptr && AsWorker == nullptr) { continue; }

		// Check type of interactable here, is a unit/character or a world-item

		
		const float LengthToOldActor = (ClosestActor->GetActorLocation() - Collision->GetComponentLocation()).Length();
		const float LengthToNewActor = (FoundActor->GetActorLocation() - Collision->GetComponentLocation()).Length();
		if (LengthToNewActor < LengthToOldActor)
		{
			ClosestActor = FoundActor;
		}
		// ^ find closest actor and store in this
	}
	
	// Overwrite HoveredActor if they are not the same
	if (ClosestActor != nullptr && ClosestActor != HoveredActor)
	{
		HoveredActor = ClosestActor;
		UE_LOG(LogTemp, Warning, TEXT("HoverTick - Found New Hover Actor"))
	}
}


void AGodHandPawn::OverwriteMappingContextSettings(APlayerController* PC, const FNativeGameplayTag& ContextTag, UInputMappingContext* NewContext)
{
	OverwriteMappingContextSettings(PC, ContextTag.GetTag(), NewContext);
}

void AGodHandPawn::OverwriteMappingContextSettings(APlayerController* PC, const FGameplayTag& ContextTag, UInputMappingContext* NewContext)
{
	if (ContextTag.IsValid() == false)
	{
		FString BuildString = FString::Printf(TEXT("AGodHandPawn(%s)::OverwriteMappingContext -- "), *GetName())
		+ FString::Printf(TEXT("\n Input tag was invalid. Skipping processing entry "));
		UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);
		return;
	}
	
	MappingContexts.FindOrAdd(ContextTag) = NewContext;
}

void AGodHandPawn::AddMappingContext(APlayerController* PC, const FNativeGameplayTag& ContextTag)
{
	AddMappingContext(PC, ContextTag.GetTag());
}

void AGodHandPawn::AddMappingContext(APlayerController* PC, const FGameplayTag& ContextTag)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem == nullptr) { return; }

	if (MappingContexts.Contains(ContextTag) == false)
	{
		FString BuildString = FString::Printf(TEXT("AGodHandPawn(%s)::AddMappingContext -- "), *GetName())
		+ FString::Printf(TEXT("\n Failed to find a valid mapping context mapped to tag (%s). Skipping processing entry "), *ContextTag.GetTagName().ToString());
		UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);
		return;
	}
	
	const FModifyContextOptions ModifyOptions{};
	Subsystem->AddMappingContext(*MappingContexts.Find(ContextTag), 0, ModifyOptions);
}

void AGodHandPawn::RemoveMappingContext(APlayerController* PC, const FNativeGameplayTag& ContextTag)
{
	RemoveMappingContext(PC, ContextTag.GetTag());
}

void AGodHandPawn::RemoveMappingContext(APlayerController* PC, const FGameplayTag& ContextTag)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem == nullptr) { return; }

	if (MappingContexts.Contains(ContextTag) == false)
	{
		FString BuildString = FString::Printf(TEXT("AGodHandPawn(%s)::RemoveMappingContext -- "), *GetName())
		+ FString::Printf(TEXT("\n Failed to find a valid mapping context mapped to tag (%s). Skipping processing entry "), *ContextTag.GetTagName().ToString());
		UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);
		return;
	}

	Subsystem->RemoveMappingContext(*MappingContexts.Find(ContextTag));
}


// Called when the game starts or when spawned
void AGodHandPawn::BeginPlay()
{
	Super::BeginPlay();

	Collision->OnComponentBeginOverlap.AddDynamic(this, &AGodHandPawn::OnCollisionBeginOverlap);
	Collision->OnComponentEndOverlap.AddDynamic(this, &AGodHandPawn::OnCollisionEndOverlap);
	
	UpdateMagnification();
	APlayerController* PC = GetController<APlayerController>();
	if (PC == nullptr) { return; }

	AddMappingContext(PC, TAG_CTRL_Ctxt_BaseInput);
	AddMappingContext(PC, TAG_CTRL_Ctxt_WorkerUnitMode);
}

void AGodHandPawn::ProjectMouseToGroundPlane(FVector2D& ScreenCoordinates, FVector& IntersectionPoint, bool& bFoundInputType)
{
	ARTSOController* PC = GetController<ARTSOController>();

	if (PC == nullptr || PC->IsValidLowLevelFast() == false)
	{
		return;
	}

	// Lock projection to center screen
	int32 SizeX = 0, SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);
	FVector2D Size2D{static_cast<double>(SizeX), static_cast<double>(SizeY)};
	Size2D *= 0.5;
	
	// Mouse viewport coordinates
	float LocX = 0, LocY = 0;
	const bool bFoundMouse = PC->GetMousePosition(LocX, LocY);
	FVector2D Coords2D{LocX, LocY};
	ScreenCoordinates = bFoundMouse ? Coords2D : Size2D;
	bFoundInputType = bFoundMouse;
	

	FHitResult FloorHitResult{};
	FCollisionQueryParams Params;
	Params.MobilityType = EQueryMobilityType::Static;
	Params.AddIgnoredActor(this);

	FVector WorldLocation{}, WorldDirection{};
	FColor EndColour = FColor::Green;
	
	if (PC->GetHitResultAtScreenPosition(ScreenCoordinates, DedicatedLandscapeTraceChannel, Params, FloorHitResult))
	{
		WorldLocation = FloorHitResult.TraceStart;
		IntersectionPoint = FloorHitResult.Location;
		EndColour = FColor::Yellow;
		return;
	}
	PC->DeprojectScreenPositionToWorld(ScreenCoordinates.X, ScreenCoordinates.Y, WorldLocation, WorldDirection);
	IntersectionPoint = FMath::LinePlaneIntersection(
	WorldLocation, 
	WorldLocation + (WorldDirection * 10000000.0),
	GetActorLocation(),
	FVector{0.0, 0.0, 1.0});
}

void AGodHandPawn::DistanceFromViewportCenter(const FVector2D& InMoveDirection, FVector& Direction, double& Strength)
{
	// Viewport halfsize
	ARTSOController* PC = GetController<ARTSOController>();
	if (PC == nullptr) { return; }

	int32 iX=0,iY=0;
	PC->GetViewportSize(iX,iY);
	float fX=iX, fY=iY;

	const double XViewportHalfsize = (fX * 0.5) - ScreenEdgeMovementPadding;
	const double YViewportHalfsize = (fY * 0.5) - ScreenEdgeMovementPadding;
	
	const double DeadzoneX = FMath::Max((FMath::Abs(InMoveDirection.X) - XViewportHalfsize), 0) / ScreenEdgeMovementPadding;
	const double DeadzoneY = FMath::Max((FMath::Abs(InMoveDirection.Y) - YViewportHalfsize), 0) / ScreenEdgeMovementPadding;	
	
	Direction.X = FMath::Sign(InMoveDirection.Y) * -1 * DeadzoneY;
	Direction.Y = FMath::Sign(InMoveDirection.X) * DeadzoneX;
	Strength = 1.0f;
}

void AGodHandPawn::ClampMovementToScreen(FVector& MoveDirection, double& Strength)
{
	FVector2D ScreenCoordinates;
	FVector IntersectionPoint;
	bool bFoundInputType;
	ProjectMouseToGroundPlane(ScreenCoordinates,IntersectionPoint, bFoundInputType);

	int32 X=0,Y=0;
	GetController<ARTSOController>()->GetViewportSize(X, Y);
	FVector2D ViewportSize = FVector2D{static_cast<double>(X),static_cast<double>(Y)} * 2.0;

	FVector2D DeltaScreenCoords = ScreenCoordinates - ViewportSize;
	DistanceFromViewportCenter(DeltaScreenCoords, MoveDirection, Strength);

	MoveDirection = UKismetMathLibrary::TransformDirection(GetActorTransform(), MoveDirection);
}

void AGodHandPawn::TrackMovement(float DeltaTime)
{
	// double VectorLength = GetActorLocation().Length();
	// FVector NormalizedLocation = GetActorLocation();
	// NormalizedLocation.Normalize(KINDA_SMALL_NUMBER);
	//
	// // Sync movement and collision placement
	// AddMovementInput(
	// 	NormalizedLocation * -1.0,
	// 	 FMath::Max((VectorLength-9000.0)/5000.0, 0.0)
	// 	);
	
	// // Clamp mouse movement to screen edges	
	// FVector MoveDirection{};
	// double Strength = 0.0;
	// ClampMovementToScreen(MoveDirection , Strength);
	// AddMovementInput(MoveDirection , Strength);
	//
	// // Update collision and mesh placement
	FVector2D ScreenCoordinates;
	FVector IntersectionPoint;
	bool bFoundInputType;
	ProjectMouseToGroundPlane(ScreenCoordinates,IntersectionPoint, bFoundInputType);
	Collision->SetWorldLocation(IntersectionPoint + FVector{0.0, 0.0, 10.0});
	UpdateCursorLocation(DeltaTime);
	// CursorMesh->SetWorldLocation(IntersectionPoint + FVector{0.0, 0.0, 10.0});
}

// Called every frame
void AGodHandPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TrackMovement(DeltaTime);
	HoverTick(DeltaTime);

	if (bUpdatePathOnTick) { RefreshPathingEffects(); }

	UEnhancedInputComponent* AsEnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (AsEnhancedInput == nullptr) { return; }

	const bool IsPressed = AsEnhancedInput->GetBoundActionValue(CtrlActionWorkerUnit).Get<bool>();
	if (IsPressed == false)
	{
		
	}
}

void AGodHandPawn::TrackUnitMovement()
{
	FVector2D v2Dummy{};
	FVector MousePlaneIntersection{};
	bool bHasInput{};
	ProjectMouseToGroundPlane(v2Dummy, MousePlaneIntersection, bHasInput);
	if (bHasInput == false) { return; }
	
	const FVector CameraLagOffsetCompensation = FVector::ZeroVector; /**< @todo camera lag is controlled by the pd camera manager, might write a function there */
	const FVector FinalMove = WorkUnitTargetLocation - MousePlaneIntersection - CameraLagOffsetCompensation;
	AddActorWorldOffset(FVector{FinalMove.X, FinalMove.Y, 0.0});
}


// Called to bind functionality to input
void AGodHandPawn::ActionMove(const FInputActionValue& Value)
{
	const FVector2D& ImmutableMoveInput = Value.Get<FVector2D>();
	
	UE_LOG(LogTemp, Warning, TEXT("X:%f, Y:%f"), ImmutableMoveInput.X, ImmutableMoveInput.Y)
	AddMovementInput(GetActorForwardVector(), ImmutableMoveInput.Y * 100);
	AddMovementInput(GetActorRightVector(), ImmutableMoveInput.X * 100);	
}

void AGodHandPawn::ActionMagnify(const FInputActionValue& Value)
{
	const float ImmutableMoveInput = Value.Get<float>();
	MagnificationStrength = ImmutableMoveInput;
	UpdateMagnification();
}

void AGodHandPawn::ActionRotate(const FInputActionValue& Value)
{
	const float ImmutableMoveInput = Value.Get<float>(); // rotate yaw
	AddActorLocalRotation(FRotator{0,ImmutableMoveInput,0});
}

void AGodHandPawn::ActionDragMove(const FInputActionValue& Value)
{
	// Handles click&drag, does not handle touch&drag
	TrackUnitMovement();
}

void AGodHandPawn::ActionWorkerUnit_Triggered(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Triggered"))

	const bool ImmutableMoveInput = Value.Get<bool>(); 
	WorkerUnitActionTarget = HoveredActor; 
}

void AGodHandPawn::ActionWorkerUnit_Started(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Started"))
	
	const bool ImmutableMoveInput = Value.Get<bool>();

	FVector2D v2Dummy{};
	FVector v3Dummy{};
	bool bDummy{};
	ProjectMouseToGroundPlane(v2Dummy, v3Dummy, bDummy);
	WorkUnitTargetLocation = v3Dummy;

	TArray<AActor*> OverlapArray;
	GetOverlappingActors(OverlapArray, APDRTSBaseUnit::StaticClass());

	const bool bIsOverlappingWorkerUnit = OverlapArray.IsEmpty() == false && OverlapArray[0] != nullptr;
	if (bIsOverlappingWorkerUnit)
	{
		SelectedWorkerUnit = OverlapArray[0];
		if (NS_WorkerPath  != nullptr && NC_WorkerPath == nullptr)
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
	// else
	AddMappingContext(GetController<APlayerController>(), TAG_CTRL_Ctxt_DragMove);
}

void AGodHandPawn::ActionWorkerUnit_Cancelled(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Cancelled"))
	ActionWorkerUnit_Completed(Value);
}

void AGodHandPawn::ActionWorkerUnit_Completed(const FInputActionValue& Value)
{
	const bool ImmutableMoveInput = Value.Get<bool>();

	RemoveMappingContext(GetController<APlayerController>(), TAG_CTRL_Ctxt_DragMove);

	APDRTSBaseUnit* AsBaseUnit = SelectedWorkerUnit != nullptr ? Cast<APDRTSBaseUnit>(SelectedWorkerUnit) : nullptr;
	const FGameplayTagContainer AssociatedTags = WorkerUnitActionTarget != nullptr && WorkerUnitActionTarget->Implements<UPDInteractInterface>()
		? IPDInteractInterface::Execute_GetGenericTagContainer(WorkerUnitActionTarget)
		: FGameplayTagContainer{}; 
	if (AssociatedTags.IsEmpty() || AsBaseUnit == nullptr || AsBaseUnit->IsValidLowLevelFast() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Completed -- Unexpected error -- Clearing selected worker unit "))

		if (NC_WorkerPath != nullptr) { NC_WorkerPath->Deactivate(); }
		bUpdatePathOnTick = false;
		WorkUnitTargetLocation = FVector::ZeroVector;
		WorkerUnitActionTarget = nullptr;
		return;
	}
	
	AsBaseUnit->RequestAction(this, WorkerUnitActionTarget, AssociatedTags.GetByIndex(0));

	if (NC_WorkerPath != nullptr) { NC_WorkerPath->Deactivate(); }
	bUpdatePathOnTick = false;
	SelectedWorkerUnit = nullptr;
	UE_LOG(LogTemp, Warning, TEXT("ActionWorkerUnit_Completed -- Clearing selected worker unit "))
	
}

void AGodHandPawn::ActionBuildMode(const FInputActionValue& Value)
{
	const bool ImmutableMoveInput = Value.Get<bool>(); 
}

void AGodHandPawn::RefreshPathingEffects()
{
	if (SelectedWorkerUnit == nullptr || Collision == nullptr ) { return; }
	
	const FVector CollisionLocation = Collision->GetComponentLocation(); 
	const FVector TargetLocation = SelectedWorkerUnit->GetActorLocation();
	UNavigationPath* Navpath = UNavigationSystemV1::FindPathToLocationSynchronously(this, CollisionLocation, TargetLocation);
	if (Navpath == nullptr || Navpath->PathPoints.IsEmpty()) { return; }

	PathPoints = Navpath->PathPoints;
	PathPoints[0] = CollisionLocation;
	PathPoints.Last() = TargetLocation;
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(NC_WorkerPath, FName("TargetPath"), PathPoints);
}

void AGodHandPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* AsEnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (AsEnhancedInput == nullptr) { return; }
	AsEnhancedInput->BindAction(CtrlActionMove, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionMove);
	AsEnhancedInput->BindAction(CtrlActionMagnify, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionMagnify);	
	AsEnhancedInput->BindAction(CtrlActionRotate, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionRotate);	
	AsEnhancedInput->BindAction(CtrlActionDragMove, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionDragMove);	

	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionWorkerUnit_Triggered);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Started, this, &AGodHandPawn::ActionWorkerUnit_Started);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Canceled, this, &AGodHandPawn::ActionWorkerUnit_Cancelled);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Completed, this, &AGodHandPawn::ActionWorkerUnit_Completed);	

	AsEnhancedInput->BindAction(CtrlActionBuildMode, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionBuildMode);	
	
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
