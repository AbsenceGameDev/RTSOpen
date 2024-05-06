/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputModifiers.h"
#include "MassEntityTypes.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/RTSOInputInterface.h"
#include "RTSOController.generated.h"

class URTSOInputStackSubsystem;

UENUM()
enum class EMarqueeSelectionEvent : uint8
{
	STARTMARQUEE,
	HOLDMARQUEE,
	RELEASEMARQUEE,
};

class UInputMappingContext;
class FNativeGameplayTag;
struct FInputActionValue;

/**
 * @brief The players controller class. This sets up mapping contexts, bindings and such. Expects them to be assigned in a data-BP  
 */
UCLASS()
class RTSOPEN_API ARTSOController : public APlayerController, public IRTSOInputInterface
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** @brief Sets up bindings for enhanced input */
	virtual void SetupInputComponent() override;

	/** @brief Adds/overwrites a mapping context settings entry, keyed by a NativeGameplayTag*/
	void OverwriteMappingContextSettings(const FNativeGameplayTag& ContextTag, UInputMappingContext* NewContext);
	/** @brief Adds/overwrites a mapping context settings entry, keyed by a regular FGameplayTag*/
	void OverwriteMappingContextSettings(const FGameplayTag& ContextTag, UInputMappingContext* NewContext);
	
	/** @brief Activates a mapping context if it exists in the mapping context settings, keyed by a NativeGameplayTag*/
	void ActivateMappingContext(const FNativeGameplayTag& ContextTag);
	/** @brief Activates a mapping context if it exists in the mapping context settings, keyed by a regular GameplayTag*/
	void ActivateMappingContext(const FGameplayTag& ContextTag);
	/** @brief De-activates a mapping context if it exists in the mapping context settings, keyed by a NativeGameplayTag*/
	void DeactivateMappingContext(const FNativeGameplayTag& ContextTag);
	/** @brief De-activates a mapping context if it exists in the mapping context settings, keyed by a regular GameplayTag*/
	void DeactivateMappingContext(const FGameplayTag& ContextTag);	
	
	/* RTSO Input Interface - Start. Almost all of these only calls into the pawn in case it has the same interface */
	virtual void ActionMove_Implementation(const FInputActionValue& Value) override;
	virtual void ActionMagnify_Implementation(const FInputActionValue& Value) override;
	virtual void ActionRotate_Implementation(const FInputActionValue& Value) override;
	virtual void ActionDragMove_Implementation(const FInputActionValue& Value) override;
	virtual void ActionWorkerUnit_Triggered_Implementation(const FInputActionValue& Value) override;
	virtual void ActionWorkerUnit_Started_Implementation(const FInputActionValue& Value) override;
	virtual void ActionWorkerUnit_Cancelled_Implementation(const FInputActionValue& Value) override;
	virtual void ActionWorkerUnit_Completed_Implementation(const FInputActionValue& Value) override;
	virtual void ActionBuildMode_Implementation(const FInputActionValue& Value) override;
	virtual void ActionClearSelection_Implementation(const FInputActionValue& Value) override;
	virtual void ActionMoveSelection_Implementation(const FInputActionValue& Value) override;	
	virtual void ActionAssignSelectionToHotkey_Implementation(const FInputActionValue& Value) override;
	virtual void ActionHotkeySelection_Implementation(const FInputActionValue& Value) override;
	virtual void ActionChordedBase_Implementation(const FInputActionValue& Value) override;

	/* RTSO Input Interface - End */

	
	/* RTSO Mouse projections - Start */
	void ProjectMouseToGroundPlane(
		FVector2D& ScreenCoordinates,
		FVector&   IntersectionPoint,
		bool&      bFoundInputType) const;
	void ProjectMouseToGroundPlane(
		TEnumAsByte<ECollisionChannel> OverrideTraceChannel,
		FVector2D&                     ScreenCoordinates,
		FVector&                       IntersectionPoint,
		bool&                          bFoundInputType) const;
	void ProjectMouseToGroundPlane(
		FHitResult&                    HitResult,
		TEnumAsByte<ECollisionChannel> OverrideTraceChannel,
		FVector2D&                     ScreenCoordinates,
		FVector&                       IntersectionPoint,
		bool&                          bFoundInputType) const;
	FHitResult ProjectMouseToGroundPlane(
		TEnumAsByte<ECollisionChannel> OverrideTraceChannel,
		FVector2D&                     ScreenCoordinates,
		bool&                          bFoundInputType) const;
	void DistanceFromViewportCenter(
		const FVector2D& InMoveDirection,
		FVector&         Direction,
		double&          Strength) const;
	void ClampMovementToScreen(FVector& MoveDirection, double& Strength) const;
	/* RTSO Mouse projections - Start */

	/* RTSO Marquee selection - Start */
	UFUNCTION() FORCEINLINE bool IsDrawingMarquee() const { return bIsDrawingMarquee; }
	UFUNCTION() FORCEINLINE FVector2D GetStartMousePositionMarquee() const {return StartMousePositionMarquee;}
	UFUNCTION() FORCEINLINE int32 GeneratedGroupID()
	{
		RollbackSelectionID = CurrentSelectionID;
		
		// this is the buffer-space, when we want to map it to a hotkey we move it to a key within the range of 0-9 so we can easily access it
		const int32 NewStep = ++SelectionBufferIdx %= SelectionBufferSize;
		CurrentSelectionID = MaxSelectionHotkeyIndex + NewStep;
		
		return CurrentSelectionID;
	} /**<@brief Range loops from 1-10 */
	UFUNCTION() FORCEINLINE int32 GenerateActorID()
	{
		static int32 StaticGroupIDStart = 99;
		return ++StaticGroupIDStart;
	} /**<@brief Range loops from 1-10 */
	
	UFUNCTION() FORCEINLINE int32 GetLatestGroupID() const { return CurrentSelectionID; }
	UFUNCTION() FORCEINLINE int32 GetCurrentGroupID() const { return CurrentSelectionID; }
	UFUNCTION() FORCEINLINE FVector2D GetCurrentMousePositionMarquee() const {return CurrentMousePositionMarquee;}
	UFUNCTION(BlueprintImplementableEvent) void OnMarqueeSelectionUpdated(int32 SelectionGroup, const TArray<int32>& NewSelection) const;
	UFUNCTION(BlueprintCallable) void MarqueeSelection(EMarqueeSelectionEvent SelectionEvent);
	static void DrawBoxAndTextChaos(const FVector& BoundsCenter, const FQuat& Rotation, const FVector& DebugExtent, const FString& DebugBoxTitle, FColor LineColour = FColor::Black);
	static void AdjustMarqueeHitResultsToMinimumHeight(FHitResult& StartHitResult, FHitResult& CenterHitResult, FHitResult& EndHitResult);
	UFUNCTION() void GetEntitiesOrActorsInMarqueeSelection();
	UFUNCTION() void ReorderGroupIndex(const int32 OldID, const int32 NewID);
	const TMap<int32, TMap<int32, FMassEntityHandle>>& GetMarqueeSelectionMap() { return MarqueeSelectedHandles; }
	UFUNCTION() int32 GetActorID() { return ActorID; };

	UFUNCTION() FORCEINLINE FHitResult GetLatestStartHitResult()  { return LatestStartHitResult;};
	UFUNCTION() FORCEINLINE FHitResult GetLatestCenterHitResult() { return LatestCenterHitResult;};
	UFUNCTION() FORCEINLINE FHitResult GetLatestEndHitResult()    { return LatestEndHitResult;};	
	
protected:
	void OnSelectionChange(bool bClearSelection);
	/* RTSO Marquee selection - End */

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Cursor|Settings")
	int32 ScreenEdgeMovementPadding = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Cursor|State")
	TEnumAsByte<ECollisionChannel> DedicatedLandscapeTraceChannel = ECollisionChannel::ECC_GameTraceChannel13;	
	
	/* Input Actions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionMove = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionMagnify = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionRotate = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionDragMove = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionWorkerUnit = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionClearSelection = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionMoveSelection = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionHotkeySelection = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionAssignSelectionToHotkey = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionChordedBase = nullptr;
	
	/* Input Actions - If I have time to implement */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionBuildMode = nullptr;
	
	/** @brief Mapping context settings, keyed by FGameplayTags. @todo Set default entries via a developer settings structure */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	TMap<FGameplayTag, UInputMappingContext*> MappingContexts{};

protected:

	UPROPERTY(VisibleInstanceOnly)
	FHitResult LatestStartHitResult{};
	UPROPERTY(VisibleInstanceOnly)
	FHitResult LatestCenterHitResult{};
	UPROPERTY(VisibleInstanceOnly)
	FHitResult LatestEndHitResult{};

	UPROPERTY(VisibleInstanceOnly)
	int32 ActorID = INDEX_NONE;
	UPROPERTY(VisibleInstanceOnly)
	int32 RollbackSelectionID = INDEX_NONE; /**< @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 LatestSelectionID = INDEX_NONE;   /**< @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 SelectionBufferIdx = INDEX_NONE;   /**< @brief */	
	UPROPERTY(VisibleInstanceOnly)
	int32 SelectionBufferSize = 20;   /**< @brief */	
	UPROPERTY(VisibleInstanceOnly)
	int32 MaxSelectionHotkeyIndex = 9;   /**< @brief */		
	UPROPERTY(VisibleInstanceOnly)
	int32 CurrentSelectionID = INDEX_NONE;  /**< @brief Need function to switch selection group */
	
	/** @brief Marquee - active state */
	uint8 bIsDrawingMarquee : 1;
	/** @brief Marquee - (Start) screen position */
	FVector2D StartMousePositionMarquee{};
	/** @brief Marquee - (Current) screen position */
	FVector2D CurrentMousePositionMarquee{};

	// @todo add some mapping to selection groups
	TSet<int32> HotKeyedSelectionGroups{};	
	TMap<int32, TMap<int32, FMassEntityHandle>> MarqueeSelectedHandles{};
	
};


/** Scalar
	*  Scales input by a set factor per axis
	*/
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "IntegerPassthrough"))
class UInputModifierIntegerPassthrough : public UInputModifier
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
protected:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

public:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	int32 IntegerPassthrough = INDEX_NONE;
	
	UPROPERTY()
	URTSOInputStackSubsystem* InputStackWorkaround = nullptr;

	FInputActionValue CachedLastValue;
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