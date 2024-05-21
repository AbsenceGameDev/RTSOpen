/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "ConversationTypes.h"

#include "NativeGameplayTags.h"
#include "PDConversationCommons.h"
#include "Actors/PDInteractActor.h"
#include "Interfaces/PDPersistenceInterface.h"
#include "Interfaces/RTSOConversationInterface.h"
#include "RTSOInteractableConversationActor.generated.h"

class ARTSOController;
class UCameraComponent;
class UPDConversationInstance;
class URTSOConversationInstance;

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

/** @brief */
USTRUCT(Blueprintable)
struct FRTSOConversationRules
{
	GENERATED_BODY()
	
	/** @brief Tag for entry data that these rules pertain */
	UPROPERTY(EditAnywhere)
	FGameplayTag EntryTag;
	
	/** @brief StartingState and ActiveState for this conversation instance */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ERTSOConversationState> State = ERTSOConversationState::CurrentStateInactive;

	/** @brief Tags required for this instance to be loaded */
	UPROPERTY(EditAnywhere)
	TArray<FGameplayTag> RequiredTags;

	/** @brief Flag to tell if this conversation is repeatable*/
	UPROPERTY(EditAnywhere)
	bool bCanRepeatConversation = true;
};

/** @brief */
USTRUCT(Blueprintable)
struct FRTSOConversationMetaProgressionDatum : public FTableRowBase
{
	GENERATED_BODY()
	
	/**< @brief Starting progression */
	UPROPERTY(EditAnywhere)
	int32 BaseProgression = 0;
	
	/** @brief Required tags to progress through the mission phases */
	UPROPERTY(EditAnywhere)
	TArray<FRTSOConversationRules> PhaseRequiredTags;
};

/** @brief */
USTRUCT(Blueprintable)
struct FRTSOConversationMetaState
{
	GENERATED_BODY()

	/** @brief Sets PhaseRequiredTags state value from ConversationProgressionEntry.PhaseRequiredTags */
	void ApplyValuesFromProgressionTable(FRTSOConversationMetaProgressionDatum& ConversationProgressionEntry);
	
	/** @brief Progression map, keyed by user (ID) and valued by progression level for the conversation.
	 * @todo I have made a mistake here as we need to know exactly which conversation the progression level relates to, not only the progression level and the userID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<int32/*ActorID*/, int32/*ProgressionLever*/> ProgressionPerPlayer;
	
	/** @brief Required tags to progress through the mission phases */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FRTSOConversationRules> PhaseRequiredTags;	
	
	/** @brief Actors that are interacting with the conversation actor. Used as storage */
	TDeque<AActor*> InteractingActors;

	/** @brief The currently active conversation instance. note@todo might need to map this, keyed by userID (players persistent ID/actorID, I really need to unify these names haha) */
	UPROPERTY(BlueprintReadOnly)
	URTSOConversationInstance* ActiveConversationInstance = nullptr;
};

/** @brief This instance adds some flags we want to use alongside with an overridden function so the system works as intended in singleplayer  */
UCLASS(BlueprintType)
class RTSOPEN_API URTSOConversationInstance : public UPDConversationInstance
{
	GENERATED_BODY()

public:
	/** @brief Sends choices to relevant listeners waiting delegate, logs when it fails. @todo Need to declare custom log categories, all over this project as matter of fact. LogTemp won't do anymore */
	virtual void PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& ClientMessage) override;

	/** @brief Appends tags to the gamesave upon having progressed a choice */
	virtual void OnChoiceNodePickedByUser(const FConversationContext& Context, const UConversationChoiceNode* ChoiceNode, const TArray<FConversationBranchPoint>& ValidDestinations) override;
};

/** @brief This class holds some helper and possibly debug functions we might want to use */
UCLASS()
class RTSOPEN_API URTSOConversationBPFL : public UPDConversationLibrary
{
	GENERATED_BODY()

public:

	/** @brief Send a request for the conversation to advance/resume */
	UFUNCTION(BlueprintCallable)
	static void RequestToAdvance(
		UPDConversationInstance* Conversation,
		UConversationParticipantComponent* ConversationParticipantComponent,
		const FAdvanceConversationRequest& InChoicePicked);

	/** @brief Starts the conversation, makes participants and sets up a conversation callback */
	UFUNCTION()
	static URTSOConversationInstance* StartConversation(
		FGameplayTag ConversationEntryTag,
		AActor* Instigator,
		FGameplayTag InstigatorTag,
		AActor* Target,
		FGameplayTag TargetTag,
		const TSubclassOf<UConversationInstance> ConversationInstanceClass);
};

/** @brief */
UCLASS()
class RTSOPEN_API ARTSOInteractableConversationActor
	: public APDInteractActor
	, public IRTSOConversationSpeakerInterface
{
	GENERATED_BODY()

public:
	/** @brief Creates the camera subobject */
	ARTSOInteractableConversationActor();

	/** @brief Empty for now. Reserved for later use */
	void ConversationStarted();
	/** @brief Empty for now. Reserved for later use */
	void ConversationTaskChoiceDataUpdated(const FConversationNodeHandle& NodeHandle, const FClientConversationOptionEntry& OptionEntry);
	/** @brief Empty for now. Reserved for later use */
	void ConversationUpdated(const FClientConversationMessagePayload& Payload);
	/** @brief Empty for now. Reserved for later use */
	void ConversationStatusChanged(bool bDidStatusChange);

	/** @brief Assigns 'TAG_AI_Job_GenericInteractCreates' to 'JobTag'.
	 * Sets up instance data, first sourcing the data from 'ConversationSettingsHandle'
	 * Creates the participant component and binds it's delegates.
	 * Also creates the async action UPDAsyncAction_ActivateFeature */
	virtual void BeginPlay() override;
	/** @brief Removes tracked conversation from URTSOConversationActorTrackerSubsystem */
	virtual void BeginDestroy() override;

	/** @brief adds 'JobTag' to a 'FGameplayTagContainer' and returns it */
	virtual FGameplayTagContainer GetGenericTagContainer_Implementation() const override;

	/** @brief Checks conversation mission phase progression for the interacting player, and picks up the conversation from there */
	virtual void OnInteract_Implementation(const FPDInteractionParamsWithCustomHandling& InteractionParams, EPDInteractResult& InteractResult) const override;
	
	/** @brief Begins the conversation if it was not active previously and otherwise it advances it 
	 *  @todo remove log output and screen prints  
	 */
	virtual void BeginWaitingForChoices_Implementation(int32 ActorID) override;
	/** @brief Requests to advance the conversation with the choice we've replied with */
	virtual void ReplyChoice_Implementation(AActor* Caller, int32 Choice) override;
	
	/** @brief Initializes progression to 0 for calling OnwerID */
	void TryInitializeOwnerProgression(int32 OwnerID) const;

	/** @brief Validates required tags by fetching the given conversation entry via
	 * the entrytag and comparing it's elements with those in the calling actor */
	virtual ERTSOConversationState ValidateRequiredTags(const FGameplayTag& EntryTag, AActor* CallingActor) const;
	/** @return InstanceDataPtr->PhaseRequiredTags[ConversationProgression].bCanRepeatConversation*/
	bool CanRepeatConversation(int32 ConversationProgression) const;
	/** @brief Resolves the entry conversation tag (that points to the new conversation instance), for the given conversation progression level */
	virtual const FGameplayTag& ResolveTagForProgressionLevel(const int32 ConversationProgression) const;
	/** @brief Empty for now. Reserved for later use */
	virtual void OnConversationPhaseStateChanged(const FGameplayTag& EntryTag, ERTSOConversationState NewState);
	
	/** @brief The handle to the settings row entry we want to apply to the mission. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/RTSOpen.RTSOConversationMetaProgressionDatum"))
	FDataTableRowHandle ConversationSettingsHandle;	

	/** @note This pointer is just so we can modify the underlying value without restrictions in const functions*/
	FRTSOConversationMetaState* InstanceDataPtr{};
	/** @brief The actual instance data */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	FRTSOConversationMetaState InstanceData{};

	/** @brief The owned participant component. */
	UPROPERTY(VisibleInstanceOnly)
	UConversationParticipantComponent* ParticipantComponent;

	/** @brief Interactable conversation actors name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ActorName{};
	
	/** @brief GameFeature name to load the conversation data from */
	UPROPERTY(EditAnywhere)
	FString GameFeatureName = "ConversationData";

	/** @brief Persistent IDs for the conversation actor */
	UPROPERTY(EditAnywhere)
	FPDPersistentID ConversationActorPersistentID;
private:
	/** @brief The JobTag related to this actor.*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	FGameplayTag JobTag{};

	/** @brief The Conversation camera, player sets this as viewtarget upon initiating a conversation*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	UCameraComponent* ConversationCamera = nullptr;	
};

/** @brief */
UCLASS()
class URTSOConversationActorTrackerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:

	// @todo start: make sure these actors copy their progression data before being destroyed
	// @todo cont:  if being destroyed by lets say an in-game event or via an intentional player interaction
	UPROPERTY()
	TSet<ARTSOInteractableConversationActor*> TrackedConversationActors{};

	/** @brief Tracked players controllers. Mainly meant for couch co-op and on the server where more than 10 defined instinct */
	UPROPERTY()
	TSet<ARTSOController*> TrackedPlayerControllers{};	
};


/**
 * Declaring the "Conversation.Entry" gameplay tags. to be defined in an object-file
 * @todo move to a 'conversation commons' file which I need to create
 */
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Entry_Intro);
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Entry_00);
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Entry_01);

/**
 * Declaring the "Conversation.Participant" gameplay tags. to be defined in an object-file
 * @todo move to a 'conversation commons' file which I need to create
 */
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Participant_Speaker);
RTSOPEN_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Conversation_Participant_Listener);


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