/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#pragma once

#include "CoreMinimal.h"
#include "Interfaces/RTSOInputInterface.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"

#include "ConversationTypes.h"
#include "GameplayTagContainer.h"
#include "InputModifiers.h"
#include "MassEntityTypes.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/PDPersistenceInterface.h"
#include "Widgets/RTSOMainMenuBase.h"


#include "RTSOController.generated.h"

class URTSOInputStackSubsystem;

/** @brief */
UENUM()
enum class EMarqueeSelectionEvent : uint8
{
	STARTMARQUEE,
	HOLDMARQUEE,
	RELEASEMARQUEE,
};

/** @brief */
USTRUCT(Blueprintable)
struct FRTSOMouseEventDelegateWrapper
{
	GENERATED_BODY()

	/** @brief */
	UPROPERTY(EditAnywhere, Category = "Events", Meta = (IsBindableEvent="True"))
	UWidget::FOnPointerEvent OnMouseMoveEvent;
	/** @brief */
	UPROPERTY(EditAnywhere, Category = "Events", Meta = (IsBindableEvent="True"))
	UWidget::FOnPointerEvent OnMouseButtonDownEvent;
	/** @brief */
	UPROPERTY(EditAnywhere, Category = "Events", Meta = (IsBindableEvent="True"))
	UWidget::FOnPointerEvent OnMouseButtonUpEvent;
	/** @brief */
	UPROPERTY(EditAnywhere, Category = "Events", Meta = (IsBindableEvent="True"))
	UWidget::FOnPointerEvent OnMouseDoubleClickEvent;
};

// consider moving away from using tileviews in the future
UCLASS(Blueprintable)
class URTSOStructWrapper : public UObject
{
	GENERATED_BODY()
public:
	
	/** @brief */
	void AssignData(
		const FText& InSelectionEntry,
		const int32 InChoiceIndex,
		UUserWidget* InDirectParentReference)
	{
		SelectionEntry = InSelectionEntry;
		ChoiceIndex = InChoiceIndex;
		DirectParentReference = InDirectParentReference;
	}

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn=true))
	FText SelectionEntry{};
	
	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn=true))
	int32 ChoiceIndex = INDEX_NONE;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn=true))
	UUserWidget* DirectParentReference = nullptr;
};

/** @brief */
UCLASS(Abstract)
class URTSOModularTile : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief */
	virtual void NativePreConstruct() override;

	/** @brief */
	UFUNCTION(BlueprintCallable)
	virtual void Refresh();
	
	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class USizeBox* SizeBoxContainer = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UImage* ImageShadow = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UNamedSlot* NamedSlot = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UTextBlock* TextName = nullptr;

	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText TileText{};

	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Height{0};		

	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Width{0};
};

UCLASS(Abstract)
class URTSOConversationSelectionEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
protected:
	
	// IUserObjectListEntry
	/** @brief */
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	// IUserObjectListEntry

	/** @brief */
	UFUNCTION() FEventReply MouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	/** @brief */
	UFUNCTION() FEventReply MouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	/** @brief */
	UFUNCTION() FEventReply MouseButtonUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	/** @brief */
	UFUNCTION() FEventReply MouseDoubleClick(FGeometry MyGeometry, const FPointerEvent& MouseEvent);	

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOModularTile* Tile = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UBorder* TextContentBorder = nullptr;	
	
	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UCommonTextBlock* TextContent = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn=true))
	int32 ChoiceIndex = INDEX_NONE;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn=true))
	UUserWidget* DirectParentReference = nullptr;	
};

/** @brief */
UCLASS(BlueprintType, Blueprintable)
class URTSOConversationMessageWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** @brief */
	UFUNCTION(BlueprintNativeEvent) FEventReply MouseMove(int32 ChoiceIdx, FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	/** @brief */
	UFUNCTION(BlueprintNativeEvent) FEventReply MouseButtonDown(int32 ChoiceIdx, FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	/** @brief */
	UFUNCTION(BlueprintNativeEvent) FEventReply MouseButtonUp(int32 ChoiceIdx, FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	/** @brief */
	UFUNCTION(BlueprintNativeEvent) FEventReply MouseDoubleClick(int32 ChoiceIdx, FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	
	/** @brief */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPayload(const FClientConversationMessagePayload& Payload, AActor* PotentialCallbackActor);
	/** @brief */
	virtual void SetPayload_Implementation(const FClientConversationMessagePayload& Payload, AActor* PotentialCallbackActor);

	/** @brief */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SelectChoice(int32 ChoiceSelection);
	/** @brief */
	virtual void SelectChoice_Implementation(int32 ChoiceSelection);

	/** @brief */
	virtual void NativeDestruct() override;
	

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	class UTileView* ConversationSelectors = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadOnly)
	AActor* CurrentPotentialCallbackActor;

	/** @brief */
	UPROPERTY(BlueprintReadWrite)
	TArray<URTSOStructWrapper*> InstantiatedEntryObjects{};

	/** @brief */
	int32 LatestInteractedChoice = INDEX_NONE;
};

/** @brief */
UCLASS(BlueprintType, Blueprintable)
class URTSOConversationWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	/** @brief */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetPayload(const FClientConversationMessagePayload& Payload, AActor* PotentialCallbackActor);
	/** @brief */
	virtual void SetPayload_Implementation(const FClientConversationMessagePayload& Payload, AActor* PotentialCallbackActor);

	/** @brief */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SelectChoice(int32 ChoiceSelection);
	/** @brief */
	virtual void SelectChoice_Implementation(int32 ChoiceSelection);
	
	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	URTSOConversationMessageWidget* ConversationMessageWidget = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	URTSOModularTile* Tile = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	UCommonTextBlock* TextContent = nullptr;

	/** @brief */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* ExitConversationButton = nullptr;
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

	/** @brief */
	virtual void BeginPlay() override;
	/** @brief */
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

	/** @brief De-activates a mapping context if it exists in the mapping context settings, keyed by a NativeGameplayTag*/
	bool IsMappingContextActive(const FNativeGameplayTag& ContextTag);
	/** @brief De-activates a mapping context if it exists in the mapping context settings, keyed by a regular GameplayTag*/
	bool IsMappingContextActive(const FGameplayTag& ContextTag);	
	
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
	virtual void ActionExitConversation_Implementation(const FInputActionValue& Value) override;
	virtual void ActionToggleMainMenu_Implementation(const FInputActionValue& Value) override;	
	void HandleConversationChoiceInput(int32 ChoiceSelection) const;
	/* RTSO Input Interface - End */
	

	/* RTSO Menu interface - Start, @todo move to actual interface if we start making more menus*/
	UFUNCTION() void OnReleased_Resume();
	UFUNCTION() void OnReleased_QuitGame();	
	/*Handled via the buttons target widget class*/ // UFUNCTION() void OnReleased_Settings(); 
	/*Handled via the buttons target widget class*/ // UFUNCTION() void OnReleased_Save();
	/*Handled via the buttons target widget class*/ // UFUNCTION() void OnReleased_Load();
	/* RTSO Menu interface - End, @todo move to actual interface if we start making more menus*/
	

	
	/* RTSO Mouse projections - Start */
	/** @brief */
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

	/** @brief */
	UFUNCTION() FORCEINLINE bool IsDrawingMarquee() const { return bIsDrawingMarquee; }
	/** @brief */
	UFUNCTION() FORCEINLINE FVector2D GetStartMousePositionMarquee() const {return StartMousePositionMarquee;}

	/** @brief Range loops from 1-10 */
	UFUNCTION() FORCEINLINE int32 GeneratedGroupID()
	{
		RollbackSelectionID = CurrentSelectionID;
		
		// this is the buffer-space, when we want to map it to a hotkey we move it to a key within the range of 0-9 so we can easily access it
		const int32 NewStep = ++SelectionBufferIdx %= SelectionBufferSize;
		CurrentSelectionID = MaxSelectionHotkeyIndex + NewStep;
		
		return CurrentSelectionID;
	}
	
	/** @brief */
	UFUNCTION() FORCEINLINE int32 GetLatestGroupID() const { return CurrentSelectionID; }
	/** @brief */
	UFUNCTION() FORCEINLINE int32 GetCurrentGroupID() const { return CurrentSelectionID; }
	/** @brief */
	UFUNCTION() FORCEINLINE FVector2D GetCurrentMousePositionMarquee() const {return CurrentMousePositionMarquee;}
	/** @brief */
	UFUNCTION(BlueprintImplementableEvent) void OnMarqueeSelectionUpdated(int32 SelectionGroup, const TArray<int32>& NewSelection) const;
	/** @brief */
	UFUNCTION(BlueprintCallable) void MarqueeSelection(EMarqueeSelectionEvent SelectionEvent);
	/** @brief */
	UFUNCTION() void OnBeginConversation(const FClientConversationMessagePayload& Payload, AActor* PotentialCallbackActor);
	/** @brief */
	UFUNCTION() void OnAdvanceConversation(const FClientConversationMessagePayload& Payload, AActor* PotentialCallbackActor);
	/** @brief */
	UFUNCTION() void OnEndConversation(const FClientConversationMessagePayload& Payload, AActor* PotentialCallbackActor);
	/** @brief */
	UFUNCTION(BlueprintCallable) int32 GetActorID() { return ActorID.GetID(); };
	/** @brief */
	static void DrawBoxAndTextChaos(const FVector& BoundsCenter, const FQuat& Rotation, const FVector& DebugExtent, const FString& DebugBoxTitle, FColor LineColour = FColor::Black);
	/** @brief */
	static void AdjustMarqueeHitResultsToMinimumHeight(FHitResult& StartHitResult, FHitResult& CenterHitResult, FHitResult& EndHitResult);
	/** @brief */
	UFUNCTION() void GetEntitiesOrActorsInMarqueeSelection();
	/** @brief */
	UFUNCTION() void ReorderGroupIndex(const int32 OldID, const int32 NewID);
	/** @brief */
	const TMap<int32, TMap<int32, FMassEntityHandle>>& GetMarqueeSelectionMap() { return MarqueeSelectedHandles; }
	/** @brief */
	UFUNCTION() FORCEINLINE URTSOConversationWidget* GetConversationWidget() const { return ConversationWidget;};

	/** @brief */
	UFUNCTION() FORCEINLINE FHitResult GetLatestStartHitResult()  { return LatestStartHitResult;};
	/** @brief */
	UFUNCTION() FORCEINLINE FHitResult GetLatestCenterHitResult() { return LatestCenterHitResult;};
	/** @brief */
	UFUNCTION() FORCEINLINE FHitResult GetLatestEndHitResult()    { return LatestEndHitResult;};	
	
protected:
	/** @brief */
	void OnSelectionChange(bool bClearSelection);
	/* RTSO Marquee selection - End */

private:
	/** @brief */
	UFUNCTION()
	void Internal_ExitConversation();
public:

	/** @brief */
	int32 LatestMenuButtonIdx = INDEX_NONE;
	
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTS|Cursor|Settings")
	int32 ScreenEdgeMovementPadding = 0;

	/** @brief */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "RTS|Cursor|State")
	TEnumAsByte<ECollisionChannel> DedicatedLandscapeTraceChannel = ECollisionChannel::ECC_GameTraceChannel13;	
	
	/* Input Actions */
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionMove = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionMagnify = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionRotate = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionDragMove = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionWorkerUnit = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionClearSelection = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionMoveSelection = nullptr;

	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionHotkeySelection = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionAssignSelectionToHotkey = nullptr;
	
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionChordedBase = nullptr;
	
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionExitConversation = nullptr;	

	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionToggleMainMenu = nullptr;	
	
	/* Input Actions - If I have time to implement */
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	class UInputAction* CtrlActionBuildMode = nullptr;
	
	/** @brief Mapping context settings, keyed by FGameplayTags. @todo Set default entries via a developer settings structure */
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RTS|Input")
	TMap<FGameplayTag, UInputMappingContext*> MappingContexts{};

protected:
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Startscreen")
	TSubclassOf<URTSOMainMenuBase> MMWidgetClass{};
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Menu|Startscreen")
	URTSOMainMenuBase* MainMenuWidget = nullptr;
	
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	URTSOConversationWidget* ConversationWidget = nullptr;
	/** @brief */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<URTSOConversationWidget> ConversationWidgetClass = nullptr;
	
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	FHitResult LatestStartHitResult{};
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	FHitResult LatestCenterHitResult{};
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	FHitResult LatestEndHitResult{};

	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	FPDPersistentID ActorID{};
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 RollbackSelectionID = INDEX_NONE;
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 LatestSelectionID = INDEX_NONE;
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 SelectionBufferIdx = INDEX_NONE;	
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 SelectionBufferSize = 20;
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 MaxSelectionHotkeyIndex = 9;
	/** @brief */
	UPROPERTY(VisibleInstanceOnly)
	int32 CurrentSelectionID = INDEX_NONE;  /**< @brief Need function to switch selection group */
	
	/** @brief Marquee - active state */
	uint8 bIsDrawingMarquee : 1;
	/** @brief Marquee - (Start) screen position */
	FVector2D StartMousePositionMarquee{};
	/** @brief Marquee - (Current) screen position */
	FVector2D CurrentMousePositionMarquee{};

	// @todo add some mapping to selection groups
	/** @brief */
	TSet<int32> HotKeyedSelectionGroups{};	
	/** @brief */
	TMap<int32, TMap<int32, FMassEntityHandle>> MarqueeSelectedHandles{};
};


/**
 * @brief Scalar modifier passthrough hack. Scales input by a set factor per axis
 */
UCLASS(NotBlueprintable, MinimalAPI, meta = (DisplayName = "IntegerPassthrough"))
class UInputModifierIntegerPassthrough : public UInputModifier
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	/** @brief */
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
protected:
	/** @brief */
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

public:
	/** @brief */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category=Settings)
	int32 IntegerPassthrough = INDEX_NONE;
	
	/** @brief */
	UPROPERTY()
	URTSOInputStackSubsystem* InputStackWorkaround = nullptr;

	/** @brief */
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