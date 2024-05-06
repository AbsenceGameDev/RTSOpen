/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "ConversationTypes.h"
#include "NativeGameplayTags.h"
#include "Actors/PDInteractActor.h"
#include "Interfaces/RTSOConversationInterface.h"
#include "RTSOInteractableConversationActor.generated.h"

class UPDConversationInstance;
/**
 * @todo @note: If I have time then write a small mission system, but avoid putting to much time into it
 * @todo @cont: as I am working on a separate repo as-well with a full fledged mission system,
 * @todo @cont: so not worth making two fully fledged ones,
 * @todo @cont: this one well have to be made with some shortcuts in design and implementation
 */
UENUM()
enum ERTSOConversationState
{
	CurrentStateActive,
	CurrentStateInactive,
	CurrentStateCompleted,
	Invalid,
	Valid,
};
USTRUCT()
struct FRTSOConversationRules
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag EntryTag; /**< @brief Tag for entry data that these rules pertain */
	
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ERTSOConversationState> State = ERTSOConversationState::CurrentStateInactive; /**< @brief StartingState and ActiveState for this conversation instance */

	UPROPERTY(EditAnywhere)
	TArray<FGameplayTag> RequiredTags; /**< @brief Tags required for this instance to be loaded */

	UPROPERTY(EditAnywhere)
	bool bCanRepeatConversation = true;
};


USTRUCT()
struct FRTSOConversationMetaProgressionDatum : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	int32 BaseProgression = 0; /**< @brief Starting progression, @todo map to actual player/owner ID */
	
	UPROPERTY(EditAnywhere)
	TArray<FRTSOConversationRules> PhaseRequiredTags;
};

USTRUCT()
struct FRTSOConversationMetaState
{
	GENERATED_BODY()

	void ApplyValuesFromProgressionTable(FRTSOConversationMetaProgressionDatum& ConversationProgressionEntry);

	UPROPERTY(VisibleInstanceOnly)
	FRTSOConversationMetaProgressionDatum Datum;
	
	UPROPERTY(VisibleInstanceOnly)
	TDeque<AActor*> InteractingActors; // Used as storage

	UPROPERTY()
	UPDConversationInstance* ActiveConversationInstance = nullptr;
};

UCLASS()
class RTSOPEN_API ARTSOInteractableConversationActor
	: public APDInteractActor
	, public IRTSOConversationSpeakerInterface
{
	GENERATED_BODY()

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	void ConversationStarted();
	void ConversationTaskChoiceDataUpdated(const FConversationNodeHandle& NodeHandle, const FClientConversationOptionEntry& OptionEntry);
	void ConversationUpdated(const FClientConversationMessagePayload& Payload);
	void ConversationStatusChanged(bool bDidStatusChange);
	virtual void BeginPlay() override;

	virtual void OnInteract_Implementation(const FPDInteractionParamsWithCustomHandling& InteractionParams, EPDInteractResult& InteractResult) const override;
	
	UFUNCTION(BlueprintCallable)
	virtual void BeginWaitingForChoices() override;

	UFUNCTION(BlueprintCallable)
	virtual void ReplyChoice(AActor* Caller, int32 Choice) override;
	
	virtual ERTSOConversationState ValidateRequiredTags(const FGameplayTag& EntryTag, AActor* CallingActor) const;
	bool CanRepeatConversation(int32 ConversationProgression) const;
	virtual const FGameplayTag& ResolveTagForProgressionLevel(const int32 ConversationProgression) const;
	virtual void OnConversationPhaseStateChanged(const FGameplayTag& EntryTag, ERTSOConversationState NewState);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/RTSBase.RTSOConversationMetaProgressionDatum"))
	FDataTableRowHandle ConversationSettingsHandle;	

	/** @note This pointer is just so we can modify the underlying value without restrictions in const functions*/
	FRTSOConversationMetaState* InstanceDataPtr{};
	FRTSOConversationMetaState InstanceData{};

	UPROPERTY(VisibleInstanceOnly)
	UConversationParticipantComponent* ParticipantComponent;

	UPROPERTY(EditAnywhere)
	FString GameFeatureName = "ConversationData";
};

/**
 * Declaring the "Conversation.Entry" gameplay tags. to be defined in an object-file
 * @todo move to a 'conversation commons' file which I need to create
 */
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Entry_Intro);
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Entry_00);
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Entry_01);

/**
 * Declaring the "Conversation.Participants" gameplay tags. to be defined in an object-file
 * @todo move to a 'conversation commons' file which I need to create
 */
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Participants_Speaker);
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Participants_Listener);


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