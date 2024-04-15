/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

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
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

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
	
	Springarm->TargetArmLength = FMath::Lerp(600, 30000, Alpha);
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
		TargetTransform = FTransform{FQuat{}, Origin, ScalarXY };
		
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

void AGodHandPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor == nullptr) { return; }

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
	Collision->GetOverlappingActors(Overlap, AActor::StaticClass());

	
	// Find interactables, could be items, could be other pawns
	AActor* ClosestActor = Overlap.IsEmpty() ? nullptr : Overlap[0];
	for (AActor* FoundActor : Overlap)
	{
		const IPDInteractInterface* FoundInterface = Cast<IPDInteractInterface>(FoundActor);
		if (FoundInterface == nullptr) { continue; }

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
	}
}


void AGodHandPawn::OverwriteMappingContext(APlayerController* PC, const FNativeGameplayTag& ContextTag, UInputMappingContext* NewContext)
{
	OverwriteMappingContext(PC, ContextTag.GetTag(), NewContext);
}

void AGodHandPawn::OverwriteMappingContext(APlayerController* PC, const FGameplayTag& ContextTag, UInputMappingContext* NewContext)
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


// Called when the game starts or when spawned
void AGodHandPawn::BeginPlay()
{
	Super::BeginPlay();
	
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

	// Lock projection to cetner screen
	int32 SizeX = 0, SizeY = 0;
	PC->GetViewportSize(SizeX, SizeY);
	FVector2D Size2D{static_cast<double>(SizeX), static_cast<double>(SizeY)};
	Size2D *= 0.5;
		
	
	// Mouse viewport coordinates
	float LocX = 0, LocY = 0;
	const bool bFoundMouse = PC->GetMousePosition(LocX, LocY);
	FVector2D Coords2D{LocX, LocY};

	FVector2D SelectedCoords = bFoundMouse ? Coords2D : Size2D;


	// PC->GetInputTouchState();


	FVector WorldLocation{}, WorldDirection{};
	PC->DeprojectScreenPositionToWorld(SelectedCoords.X, SelectedCoords.Y, WorldLocation, WorldDirection);

	IntersectionPoint = FMath::LinePlaneIntersection(
	WorldLocation, 
	WorldLocation + (WorldDirection * 10000.0),
	FVector{0.0, 0.0, 0.0},
	FVector{0.0, 0.0, 1.0});
	
	ScreenCoordinates = SelectedCoords;
	bFoundInputType = bFoundMouse;
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
	double VectorLength = GetActorLocation().Length();
	FVector NormalizedLocation = GetActorLocation();
	NormalizedLocation.Normalize(KINDA_SMALL_NUMBER);

	// Sync movement and collision placement
	AddMovementInput(
		NormalizedLocation * -1.0,
		 FMath::Max((VectorLength-9000.0)/5000.0, 0.0)
		);
	UpdateCursorLocation(DeltaTime);

	// Clamp mouse movement to screen edges	
	FVector MoveDirection{};
	double Strength = 0.0;
	ClampMovementToScreen(MoveDirection , Strength);
	AddMovementInput(MoveDirection , Strength);


	// Update collision placement
	FVector2D ScreenCoordinates;
	FVector IntersectionPoint;
	bool bFoundInputType;
	ProjectMouseToGroundPlane(ScreenCoordinates,IntersectionPoint, bFoundInputType);
	Collision->SetWorldLocation(IntersectionPoint + FVector{0.0, 0.0, 10.0});
	
}

// Called every frame
void AGodHandPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TrackMovement(DeltaTime);

	if (bUpdatePathOnTick) { RefreshPathingEffects(); }
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
	AddMovementInput(GetActorForwardVector(), ImmutableMoveInput.Y);
	AddMovementInput(GetActorRightVector(), ImmutableMoveInput.X);
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
	const bool ImmutableMoveInput = Value.Get<bool>(); 
	SelectedWorkerUnitTarget = HoveredActor; 
}

void AGodHandPawn::ActionWorkerUnit_Started(const FInputActionValue& Value)
{
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
		if (NS_WorkerPath)
		{
			NC_WorkerPath = UNiagaraFunctionLibrary::SpawnSystemAttached(NS_WorkerPath, SceneRoot, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::SnapToTarget, false);
			// bUpdatePathOnTick = true; // @todo enable when this has actually been implemented
		}
		return;
	}
	// else
	AddMappingContext(GetController<APlayerController>(), TAG_CTRL_Ctxt_DragMove);
}

void AGodHandPawn::ActionWorkerUnit_Cancelled(const FInputActionValue& Value)
{
	const bool ImmutableMoveInput = Value.Get<bool>(); 
}

void AGodHandPawn::ActionWorkerUnit_Completed(const FInputActionValue& Value)
{
	const bool ImmutableMoveInput = Value.Get<bool>(); 
}

void AGodHandPawn::ActionBuildMode(const FInputActionValue& Value)
{
	const bool ImmutableMoveInput = Value.Get<bool>(); 
}

void AGodHandPawn::RefreshPathingEffects()
{
	// @todo
}

void AGodHandPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* AsEnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (AsEnhancedInput == nullptr) { return; }
	AsEnhancedInput->BindAction(CtrlActionMove, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionMove);
	AsEnhancedInput->BindAction(CtrlActionMagnify, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionMagnify);	
	AsEnhancedInput->BindAction(CtrlActionRotate, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionRotate);	
	// @todo // AsEnhancedInput->BindAction(CtrlActionDragMove, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionDragMove);	

	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionWorkerUnit_Triggered);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Started, this, &AGodHandPawn::ActionWorkerUnit_Started);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Canceled, this, &AGodHandPawn::ActionWorkerUnit_Cancelled);	
	AsEnhancedInput->BindAction(CtrlActionWorkerUnit, ETriggerEvent::Completed, this, &AGodHandPawn::ActionWorkerUnit_Completed);	

	AsEnhancedInput->BindAction(CtrlActionBuildMode, ETriggerEvent::Triggered, this, &AGodHandPawn::ActionBuildMode);	
	
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

