/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "PDItemCommon.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GameFramework/Pawn.h"
#include "GodHandPawn.generated.h"

class UInputMappingContext;
class APDInteractActor;
class UPDInventoryComponent;

UCLASS()
class RTSOPEN_API AGodHandPawn : public APawn
{
	GENERATED_BODY()

public:
	AGodHandPawn();
	virtual void Tick(float DeltaTime) override;
	void TrackUnitMovement();
	virtual void BeginPlay() override;

	void UpdateMagnification();
	void UpdateCursorLocation(float DeltaTime);
	virtual void PossessedBy(AController* NewController) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	void HoverTick(float DeltaTime);
	
	void OverwriteMappingContext(APlayerController* PC, const FNativeGameplayTag& ContextTag, UInputMappingContext* NewContext);
	void OverwriteMappingContext(APlayerController* PC, const FGameplayTag& ContextTag, UInputMappingContext* NewContext);

	void AddMappingContext(APlayerController* PC, const FNativeGameplayTag& ContextTag);
	void AddMappingContext(APlayerController* PC, const FGameplayTag& ContextTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void BeginBuild(TSubclassOf<AActor> TargetClass, TMap<FGameplayTag /*Resource tag*/, FPDItemCosts>& ResourceCost);

	/* Enhanced Input */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION()
	void ActionMove(const FInputActionValue& Value);
	UFUNCTION()
	void ActionMagnify(const FInputActionValue& Value);
	UFUNCTION()
	void ActionRotate(const FInputActionValue& Value);
	UFUNCTION()
	void ActionDragMove(const FInputActionValue& Value);
	
	UFUNCTION()
	void ActionWorkerUnit_Triggered(const FInputActionValue& Value);
	UFUNCTION()
	void ActionWorkerUnit_Started(const FInputActionValue& Value);
	UFUNCTION()
	void ActionWorkerUnit_Cancelled(const FInputActionValue& Value);
	UFUNCTION()
	void ActionWorkerUnit_Completed(const FInputActionValue& Value);

	UFUNCTION()
	void ActionBuildMode(const FInputActionValue& Value);	
protected:

private:
	void RefreshPathingEffects();
	void ProjectMouseToGroundPlane(FVector2D& ScreenCoordinates, FVector& IntersectionPoint, bool& bFoundInputType);
	void DistanceFromViewportCenter(const FVector2D& InMoveDirection, FVector& Direction, double& Strength);
	void ClampMovementToScreen(FVector& MoveDirection, double& Strength);
	void TrackMovement(float DeltaTime);

public:
	/* Gameplay system components */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="RTS|Pawn|GameplayComponents")
	class UPDInteractComponent* InteractComponent = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category ="RTS|Pawn|GameplayComponents")
	class UPDInventoryComponent* InventoryComponent = nullptr;

	/* Gameplay system components */
	UPROPERTY(EditAnywhere, Category = "RTS|Pawn|Effects")
	class UNiagaraSystem* NS_WorkerPath;
	UPROPERTY(VisibleInstanceOnly, Category = "RTS|Pawn|Effects")
	class UNiagaraComponent* NC_WorkerPath;
	
	/* Input Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	class UInputAction* CtrlActionMove = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	class UInputAction* CtrlActionMagnify = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	class UInputAction* CtrlActionRotate = nullptr;
	// @todo // UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	// @todo // class UInputAction* ControllerDragMove = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	class UInputAction* CtrlActionWorkerUnit = nullptr;

	/* Input Actions - If I have time to implement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	class UInputAction* CtrlActionBuildMode = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	TMap<FGameplayTag, UInputMappingContext*> MappingContexts{};
	
	/* Scene/actor components */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Base")
	class USceneComponent* SceneRoot = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Base")
	class USpringArmComponent* Springarm = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Base")
	class UCameraComponent* Camera = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Base")
	class UStaticMeshComponent* CursorMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Base")
	class USphereComponent* Collision = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Base")
	class UFloatingPawnMovement* PawnMovement = nullptr;

	/* State(s) - tracked components */	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RTS|Pawn|State")
	AActor* HoveredActor = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RTS|Pawn|State")
	TSubclassOf<AActor> TempSpawnClass;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RTS|Pawn|State")
	APDInteractActor* SpawnedInteractable = nullptr;
	
	/* State(s) - tracked values */	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RTS|Pawn|State")
	TMap<FGameplayTag, FPDItemCosts> CurrentResourceCost;
	
	/* Setting(s) - Cursor @todo move into a table row type and source from a datatable */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	UCurveFloat* MagnificationCurve = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	double MagnificationValue = 0.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	double MagnificationStrength = 0.01;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	double TargetPadding = 40.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	double CursorSelectedScalePhaseSpeed = 4.0;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	double CursorSelectedSinScaleIntensity = 0.3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	double SelectionRescaleSpeed = 10.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|Settings")
	int32 ScreenEdgeMovementPadding;
	
	/* State(s) - Cursor */	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|State")
	uint8 bTickHover : 1;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	double AccumulatedPlayTime = 0.0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	FTransform TargetTransform{};
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	FVector WorkUnitTargetLocation{};
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	AActor* SelectedWorkerUnitTarget;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	AActor* SelectedWorkerUnit;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	bool bUpdatePathOnTick = false;
};

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

