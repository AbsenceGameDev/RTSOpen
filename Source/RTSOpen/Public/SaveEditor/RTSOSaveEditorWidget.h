/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once
#include "CoreMinimal.h"
#include "RTSOpenCommon.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "Components/WidgetSwitcher.h"
#include "Components/CircularThrobber.h"
#include "Widgets/RTSOActiveMainMenu.h"
#include "Widgets/Layout/SWrapBox.h"
#include "RTSOSaveEditorWidget.generated.h"

class SRTSOSaveEditorBase;
struct FUserMissionTagsStruct;
struct FConversationStateStruct;
struct FUserInventoriesStruct;
struct FPlayerLocationStruct;
class URTSOpenSaveGame;
class SRTSOSaveEditor;

UCLASS(Blueprintable)
class URTSOSaveEditorInnerWidget : public UWidget
{
	GENERATED_BODY()

public:
	void CopyData(URTSOpenSaveGame* InSaveGame);
	void OnFailedCopyData();
	void OnCompletedCopyData();
	void UpdateInnerEditor();
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION() void SelectEditor(EPDSaveDataThreadSelector NewEditorType);
	void ResetFieldData(EPDSaveDataThreadSelector SaveDataGroupSelector);

	TSharedPtr<SWrapBox> InnerSlateWrapbox;
	TSharedPtr<SRTSOSaveEditorBase> SharedExistingSaveEditor;

	EPDSaveDataThreadSelector EditorType = EPDSaveDataThreadSelector::EPlayers;
	
	TArray<TSharedPtr<FPlayerLocationStruct>>    LocationsAsSharedTupleArray;
	TArray<TSharedPtr<FRTSSavedInteractable>>    InteractableAsSharedArray;
	TArray<TSharedPtr<FRTSSavedWorldUnits>>      EntitiesAsSharedArray;
	TArray<TSharedPtr<FUserInventoriesStruct>>   AllUserInventoriesAsSharedTupleArray;
	TArray<TSharedPtr<FConversationStateStruct>> ConversationStatesAsSharedArray;
	TArray<TSharedPtr<FUserMissionTagsStruct>>   UserMissionTagsAsSharedArray;

	UPROPERTY()
	URTSOpenSaveGame* SaveGamePtr = nullptr;
	UPROPERTY()
	FRTSSaveData CopiedSaveData;
};

UCLASS(Blueprintable)
class URTSOSaveEditorUserWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void BindButtonDelegates(AActor* ActorToBindAt);

	UFUNCTION()
	void LoadSlotData(int32 SlotIdx);
	
	UFUNCTION()
	void LoadSlotData0();
	UFUNCTION()
	void LoadSlotData1();	
	UFUNCTION()
	void LoadSlotData2();
	UFUNCTION()
	void LoadSlotData3();
	UFUNCTION()
	void LoadSlotData4();

	UButton* GetCategoryButton(EPDSaveDataThreadSelector Button) const;
	void Category_ResetButtonState(EPDSaveDataThreadSelector PreviousButton) const;
	void Category_ActivateButtonState(EPDSaveDataThreadSelector NewButton) const;
	void SelectCategory(EPDSaveDataThreadSelector CategoryButton) const;
	void SelectCategoryAndInvalidateCache(EPDSaveDataThreadSelector CategoryButton);
	UFUNCTION() void Category_WorldBaseData();
	UFUNCTION() void Category_PlayerBaseData();
	UFUNCTION() void Category_InteractableData();
	UFUNCTION() void Category_EntityData();
	UFUNCTION() void Category_PlayerInventoriesData();
	UFUNCTION() void Category_ConversationStateData();
	UFUNCTION() void Category_MissionProgressTagsData();

	virtual void OnAnimationStarted_Implementation(const UWidgetAnimation* Animation) override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UCanvasPanel* BaseCanvas = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	URTSOMenuButton* ExitButton = nullptr;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	URTSOSaveEditorInnerWidget* Inner = nullptr;

	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite, Meta = (BindWidgetAnim))
	UWidgetAnimation* CategoryLoadingAnimation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UWidgetSwitcher* LoadViewWidgetSwitch = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UWidgetSwitcher* TabsLoadViewWidgetSwitch = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UOverlay* DataViewLoaderAnimationLayer = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UOverlay* TabsLoaderAnimationLayer = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UOverlay* CategoryButtonOverlay = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UOverlay* DataViewOverlay = nullptr;
	

	/** @defgroup SaveDataEditorTabButtons  */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget)) class URTSOMenuButton* Btn_WorldBaseData = nullptr; /**< @brief Button that invoked the WorldBase Data view. @ingroup SaveDataEditorTabButtons */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget)) class URTSOMenuButton* Btn_PlayerBaseData = nullptr; /**< @brief Button that invoked the WorldBase Data view. @ingroup SaveDataEditorTabButtons */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget)) class URTSOMenuButton* Btn_InteractableData = nullptr; /**< @brief Button that invoked the WorldBase Data view. @ingroup SaveDataEditorTabButtons */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget)) class URTSOMenuButton* Btn_EntityData = nullptr; /**< @brief Button that invoked the WorldBase Data view. @ingroup SaveDataEditorTabButtons */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget)) class URTSOMenuButton* Btn_PlayerInventoriesData = nullptr; /**< @brief Button that invoked the WorldBase Data view. @ingroup SaveDataEditorTabButtons */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget)) class URTSOMenuButton* Btn_ConversationStateData = nullptr; /**< @brief Button that invoked the WorldBase Data view. @ingroup SaveDataEditorTabButtons */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget)) class URTSOMenuButton* Btn_MissionProgressTagsData = nullptr; /**< @brief Button that invoked the WorldBase Data view. @ingroup SaveDataEditorTabButtons */
	

	/** @defgroup MenuButtonGroup
	 *  @todo replace with ulistview of available slots */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot0 = nullptr; /**< @brief Self-explanatory. @ingroup MenuButtonGroup */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot1 = nullptr; /**< @brief Self-explanatory. @ingroup MenuButtonGroup */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot2 = nullptr; /**< @brief Self-explanatory. @ingroup MenuButtonGroup */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot3 = nullptr; /**< @brief Self-explanatory. @ingroup MenuButtonGroup */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot4 = nullptr; /**< @brief Self-explanatory. @ingroup MenuButtonGroup */

	UPROPERTY(VisibleInstanceOnly)
	URTSOpenSaveGame* LoadedGameSaveForModification = nullptr;
	
};

/*
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
