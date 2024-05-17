/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonButtonBase.h"
#include "RTSOActiveMainMenu.generated.h"

class UCommonTextBlock;

/** @brief Save/load game delegate, used with the pop-up dialogs defined further down this file */
DECLARE_DELEGATE_TwoParams(FRTSOSavegameDelegate, const FString&, bool)

/** @brief Save type enum*/
UENUM()
enum ERTSOSaveType
{
	SAVE,
	LOAD
};

/** @brief */
UCLASS(Blueprintable)
class URTSOMenuButton : public UUserWidget
{
	GENERATED_BODY()

public:
	/** @brief Base/Root Overlay */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UOverlay* Overlay = nullptr;

	/** @brief Image (or material instance) for the buttons background*/
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UImage* Image_ButtonBackground = nullptr;

	/** @brief Widget border */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UBorder* Border = nullptr;

	/** @brief Button text block widget */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget, ExposeOnSpawn))
	class UTextBlock* TextBlock = nullptr;	
	
	/** @brief Hitbox, what our mouse events actually interacts with */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UButton* Hitbox = nullptr;

	/** @brief Target of the button press, add this class to the stack */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCommonActivatableWidget> PotentialTargetWidgetClass = nullptr;
	/** @brief Unused for now, reserve for later use */
	UPROPERTY()
	class URTSOMainMenuBase* OwningStack = nullptr;	
};

/** @brief */
UCLASS(Blueprintable)
class URTSOSaveGameDialog : public UUserWidget
{
	GENERATED_BODY()

public:

	/** @brief Calls SuccessCallback.Execute with the passthrough parameters*/
	UFUNCTION()
	void DialogReplyContinue();

	/** @brief Assign the yes/no button events to Reply_Yes & Reply_No */
	UFUNCTION()
	void SetupDelegates();	

	/** @brief Calls 'DialogReplyContinue' then calls 'RemoveFromParent' to remove self*/
	UFUNCTION()
	void Reply_Yes();	

	/** @brief Only Calls 'RemoveFromParent' to remove self*/
	UFUNCTION()
	void Reply_No();	

	/** @brief Tet block widget to contain the actual widget message*/
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UTextBlock* DialogContent = nullptr;	
	
	/** @brief Button to accept/reply yes*/
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* YesButton = nullptr;

	/** @brief Button to refuse/reply no*/
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* NoButton = nullptr;	
	
	/** @brief Message that the dialog should display */
	UPROPERTY(EditAnywhere)
	FText DialogMessage{};

	/** @brief Pass-through for the ERTSOSaveType */
	UPROPERTY()
	TEnumAsByte<ERTSOSaveType> Type = ERTSOSaveType::LOAD;

	/** @brief Pass-through for the slotidx */
	UPROPERTY()
	int32 SlotIdx = INDEX_NONE;

	/** @brief Callback that we want to fire on success (replying yes to the dialog)*/
	FRTSOSavegameDelegate SuccessCallback{};
};

/** @brief */
UCLASS(Blueprintable)
class URTSOMenuWidget_SaveGame : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/** @brief Unimplemented. Reserved for later use */
	void ClearDelegates() const {};

	/** @brief Only Calls super for now. Reserved for later use */
	virtual void NativePreConstruct() override;
	
	/** @brief (root) Base canvas panel */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UCanvasPanel* BaseCanvas = nullptr;

	/** @brief The box in which the banner and the inner box should contain */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UVerticalBox* MainBox = nullptr;

	/** @brief Canvas panel of the widget banner, for orienting elements within the banner */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UCanvasPanel* BannerCanvas = nullptr;

	/** @brief Text of the widget banner */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UTextBlock* BannerText = nullptr;

	/** @brief Image of the widget banner */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UImage* BannerImage = nullptr;

	/** @brief Button that is bound (in URTSOMainMenuBase::SaveTargetWidget & URTSOMainMenuBase::LoadTargetWidget) ot close the open 'URTSOMenuWidget_SaveGame'*/
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* ExitButton = nullptr;

	/** @brief Box that should hold all slots of type URTSOMenuButton. @todo replace with ulistview of slots */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UVerticalBox* InnerBox = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot0 = nullptr; /**< @brief Self-explanatory. @todo replace with ulistview of slots */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot1 = nullptr; /**< @brief Self-explanatory. @todo replace with ulistview of slots */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot2 = nullptr; /**< @brief Self-explanatory. @todo replace with ulistview of slots */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot3 = nullptr; /**< @brief Self-explanatory. @todo replace with ulistview of slots */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* Slot4 = nullptr; /**< @brief Self-explanatory. @todo replace with ulistview of slots */
	
	/** @brief Unused for now, reserve for later use */
	UPROPERTY()
	class URTSOMainMenuBase* OwningStack = nullptr;	
};

/** @brief */
UCLASS(Blueprintable)
class URTSOMenuWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	/** @brief Clears delegates on the buttons hitboxes */
	void ClearDelegates() const;
	
	/** @brief The resume button, is bound by the OwningMenuBaseWidget to to a function which removes the menu and resumes the game */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* ResumeButton = nullptr;

	/** @brief The settings button, will add the settings menu to the widget stack of the owning menu widget base */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* SettingsButton = nullptr;

	/** @brief The save-game button, will add the save menu to the widget stack of the owning menu widget base */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* SaveButton = nullptr;

	/** @brief The load-game button, will add the load menu to the widget stack of the owning menu widget base */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* LoadButton = nullptr;

	/** @brief The quit-game button quits the actual game @todo set up dialog widget to make sure the player wants to quit if they have not saved the game */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class URTSOMenuButton* QuitButton = nullptr;

	/** @brief Contains the buttons in a vertical direction */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UVerticalBox* VerticalBoxContainer = nullptr;

	/** @brief Unused for now, reserve for later use */
	UPROPERTY()
	class URTSOMainMenuBase* OwningMenuBaseWidget = nullptr;	
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
