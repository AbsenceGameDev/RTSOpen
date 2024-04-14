/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "Actors/GodHandPawn.h"

#include "Components/PDInteractComponent.h"
#include "Components/PDInventoryComponent.h"
#include "Actors/PDInteractActor.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"

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
	
	
	PawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("FloatingMovement"));
}

void AGodHandPawn::UpdateMagnification()
{
	MagnificationValue = FMath::Clamp((MagnificationStrength * 0.01) + MagnificationValue, 0.0, 1.0);

	const float Alpha = MagnificationCurve != nullptr
		? MagnificationCurve->GetFloatValue(MagnificationValue)
		: Springarm->TargetArmLength / (Springarm->TargetArmLength * MagnificationValue);
	
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

void AGodHandPawn::AddMappingContexts_Implementation(APlayerController* PC)
{
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	if (Subsystem == nullptr) { return; }

	const FModifyContextOptions ModifyOptions{};
	for (const UInputMappingContext* Ctxt : MappingContexts)
	{
		Subsystem->AddMappingContext(Ctxt, 0, ModifyOptions);
	}
}


// Called when the game starts or when spawned
void AGodHandPawn::BeginPlay()
{
	Super::BeginPlay();

	UpdateMagnification();

	//
	// @todo At this line: Start ticking MoveTracking()
	
 
	APlayerController* PC = GetController<APlayerController>();
	if (PC == nullptr) { return; }

	AddMappingContexts(PC);
}

// Called every frame
void AGodHandPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGodHandPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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

