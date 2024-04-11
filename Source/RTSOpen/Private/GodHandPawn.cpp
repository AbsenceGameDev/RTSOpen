// Fill out your copyright notice in the Description page of Project Settings.


#include "GodHandPawn.h"

#include "Components/PDInteractComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
AGodHandPawn::AGodHandPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneRoot  = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	InteractComponent = CreateDefaultSubobject<UPDInteractComponent>(TEXT("InteractComponent"));
	
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
	Springarm->SetRelativeRotation(FRotator{FMath::Lerp(-35,-55, Alpha),0,0});
	PawnMovement->MaxSpeed =  FMath::Lerp(1000, 5000, Alpha);

	// depth of field when magnifying
	FPostProcessSettings PPSettings{};
	PPSettings.DepthOfFieldFocalDistance = Springarm->TargetArmLength;
	PPSettings.DepthOfFieldSensorWidth = 175.f;
	Camera->PostProcessSettings = PPSettings;

	Camera->SetFieldOfView(FMath::Lerp(25.0f, 15.0f, Alpha));
}

// Called when the game starts or when spawned
void AGodHandPawn::BeginPlay()
{
	Super::BeginPlay();

	UpdateMagnification();
 
	APlayerController* PC = GetController<APlayerController>();
	if (PC == nullptr) { return; }

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
	
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

