﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "PDInteractCommon.h"
#include "Blueprint/UserWidget.h"
#include "Containers/Deque.h"
#include "PDInteractSharedUI.generated.h"

class UButton;
class UTextBlock;
class UImage;

/**
 * @brief Base interactable widget. Has events and data related to interaction messages/popups
 */
UCLASS(BlueprintType, Blueprintable)
class UPDWInteractable_Base : public UUserWidget
{
	GENERATED_BODY()
public:
	/** @brief Does nothing significant for now ( calls into super ). Reserved for later use */
	virtual void NativeConstruct() override;
	/** @brief Does nothing significant for now ( calls into super ). Reserved for later use */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	/** @brief Sets the current message cache with an optional parameter to allow to apply it immediately to the widget */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	FORCEINLINE void SetCurrentMessage(const FPDInteractMessage& InMessage, const bool bApplyImmediate = true)
	{
		CurrentMessage = InMessage;

		if (bApplyImmediate) { ApplyMessageChanges(); }
	}

	/** @brief Apply cached message data to the relevant widget text object */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	void ApplyMessageChanges();

	/** @brief Sets the current tracked actor */
	UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
	void SetTrackedActor(AActor* InActor) { TrackedActor = InActor;}
	/** @brief Gets the current tracked actor */
	AActor* GetTrackedActor() const { return TrackedActor;}
	/** @brief Event call when an interact widget pop-up is spawned */
	void OnSpawnWidgetPopup(class UPDWRadialInteract_HUD* RadialInteract_HUD);
	
protected:
	/** @brief The currently represented interaction message */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPDInteractMessage CurrentMessage;
	
	/** @brief The current interaction trace type. Radial trace or Shape trace along line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPDTickTraceType TraceType = EPDTickTraceType::TRACE_RADIAL;
	
	/** @brief Title textblock widget. Usually represents the interactable actor name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UTextBlock* InteractableTitle = nullptr;
	/** @brief KeyIndicator textblock widget. Represents the key(s) needing to be pressed to perform the interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UTextBlock* KeyIndicator_Action = nullptr;
	/** @brief KeyIndicator image widget. Background for the KeyIndicator textblock */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UImage* KeyIndicator_Image = nullptr;
	/** @brief GameMessage_Action textblock widget. Represents the action being performed with this interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UTextBlock* GameMessage_Action = nullptr;

	/** @brief Actual pressable UI button for performing the interaction,
	 * as an alternative way to interact outside of using the keyboard */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Meta = (BindWidget))
	UButton* OnScreenInteractButton = nullptr;
	
	/** @brief Actor being tracked for this widget. Will dictate where, on-screen, this widget will be at all times */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AActor* TrackedActor = nullptr;

};

/**
 * @brief Radial trace interact hud. Mainly meant to be used for representing the units surroundings when tracing radially 
 */
UCLASS(BlueprintType, Blueprintable)
class UPDWRadialInteract_HUD : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** @brief Ticks the onscreen interactables, widgets that represent interactables we see on screen within the interaction range */
	void TickInteractables();

	/** @brief override GetOverridePawn_Implementation in a child class, for projects where the pawn in question is controlled but not owned by the controller */
	UFUNCTION(BlueprintNativeEvent)
	APawn* GetOverridePawn();
	
	/** @brief Spawns a new interactable widget, given a message and an actor to track*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SpawnNewInteractableWidget(const FPDInteractMessage& Message, AActor* ActorToTrack);
	
	/** @brief Map to keep track of the onscreen interactables.
	 * Map is keyed by the actor pointer and value is the actual base widget that is being displayed for it */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<AActor*, UPDWInteractable_Base*> OnScreenInteractables;

	/** @brief The max amount of on-screen interactables the user can have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxOnScreenInteractables = 50;

	/** @brief The widget class for the tracked interactables */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UPDWInteractable_Base> SpawnWidgetInteractableClass;

protected:

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
