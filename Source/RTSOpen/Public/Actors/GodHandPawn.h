/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

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

	UFUNCTION()
	void OnCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                             int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                           int32 OtherBodyIndex);
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
	void HoverTick(float DeltaTime);
	
	void OverwriteMappingContextSettings(APlayerController* PC, const FNativeGameplayTag& ContextTag, UInputMappingContext* NewContext);
	void OverwriteMappingContextSettings(APlayerController* PC, const FGameplayTag& ContextTag, UInputMappingContext* NewContext);

	void AddMappingContext(APlayerController* PC, const FNativeGameplayTag& ContextTag);
	void AddMappingContext(APlayerController* PC, const FGameplayTag& ContextTag);
	void RemoveMappingContext(APlayerController* PC, const FNativeGameplayTag& ContextTag);
	void RemoveMappingContext(APlayerController* PC, const FGameplayTag& ContextTag);

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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Pawn|Input")
	class UInputAction* CtrlActionDragMove = nullptr;
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
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "RTS|Pawn|Cursor|State")
	uint8 bTickHover : 1;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	double AccumulatedPlayTime = 0.0;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	FTransform TargetTransform{};
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	FVector WorkUnitTargetLocation{};
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	AActor* WorkerUnitActionTarget;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	AActor* SelectedWorkerUnit;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	bool bUpdatePathOnTick = false;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	TArray<FVector> PathPoints;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Pawn|Cursor|State")
	TEnumAsByte<ECollisionChannel> DedicatedLandscapeTraceChannel = ECollisionChannel::ECC_GameTraceChannel13;
};

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

