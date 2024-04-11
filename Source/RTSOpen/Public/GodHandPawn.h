// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GodHandPawn.generated.h"

UCLASS()
class RTSOPEN_API AGodHandPawn : public APawn
{
	GENERATED_BODY()

public:
	AGodHandPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void UpdateMagnification();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="RTS|Pawn")
	class UPDInteractComponent* InteractComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	class USceneComponent* SceneRoot = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	class USpringArmComponent* Springarm = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	class UCameraComponent* Camera = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	class UStaticMeshComponent* CursorMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	class USphereComponent* Collision = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	class UFloatingPawnMovement* PawnMovement = nullptr;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	UCurveFloat* MagnificationCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	double MagnificationValue = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Player")
	double MagnificationStrength = 0.0;
};
