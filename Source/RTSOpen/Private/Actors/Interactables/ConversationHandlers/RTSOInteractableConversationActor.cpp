/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "Actors/Interactables/ConversationHandlers/RTSOInteractableConversationActor.h"
#include "ConversationInstance.h"
#include "ConversationLibrary.h"
#include "ConversationParticipantComponent.h"
#include "MassEntitySubsystem.h"
#include "PDConversationCommons.h"
#include "PDRTSBaseSubsystem.h"
#include "AI/Mass/PDMassFragments.h"
#include "Interfaces/RTSOConversationInterface.h"
#include "Kismet/KismetSystemLibrary.h"

/** Declaring the "Conversation.Entry" gameplay tags. to be defined in an object-file
 * @todo move to a 'conversation commons' file which I need to create */
UE_DEFINE_GAMEPLAY_TAG(TAG_Conversation_Entry_Intro, "Conversation.Entry.Intro");
UE_DEFINE_GAMEPLAY_TAG(TAG_Conversation_Entry_00, "Conversation.Entry.00");
UE_DEFINE_GAMEPLAY_TAG(TAG_Conversation_Entry_01, "Conversation.Entry.01");


/** Declaring the "Conversation.Participants" gameplay tags. to be defined in an object-file
 * @todo move to a 'conversation commons' file which I need to create */
UE_DEFINE_GAMEPLAY_TAG(TAG_Conversation_Participants_Speaker, "Conversation.Participants.Speaker");
UE_DEFINE_GAMEPLAY_TAG(TAG_Conversation_Participants_Listener, "Conversation.Participants.Listener");

void FRTSOConversationMetaState::ApplyValuesFromProgressionTable(FRTSOConversationMetaProgressionDatum& ConversationProgressionEntry)
{
	Datum.BaseProgression = ConversationProgressionEntry.BaseProgression;
	Datum.PhaseRequiredTags = ConversationProgressionEntry.PhaseRequiredTags;
	
	
}

void ARTSOInteractableConversationActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);


	// @todo handle loading progression from server or game-save here in OnConstruction

	if (ConversationSettingsHandle.IsNull()) { return; }
	const FString Context = FString::Printf(TEXT("ARTSOInteractableConversationActor(%s)::OnConstruction -- Attempting to access ConversationSettingsHandle.GetRow<FRTSOConversationMetaProgressionDatum>() "), *GetName());

	InstanceData.ApplyValuesFromProgressionTable(*ConversationSettingsHandle.GetRow<FRTSOConversationMetaProgressionDatum>(Context));
	InstanceDataPtr = &InstanceData;
}

void ARTSOInteractableConversationActor::ConversationStarted()
{
}
void ARTSOInteractableConversationActor::ConversationTaskChoiceDataUpdated(const FConversationNodeHandle& NodeHandle, const FClientConversationOptionEntry& OptionEntry)
{
}
void ARTSOInteractableConversationActor::ConversationUpdated(const FClientConversationMessagePayload& Payload)
{
}
void ARTSOInteractableConversationActor::ConversationStatusChanged(bool bDidStatusChange)
{
}

void ARTSOInteractableConversationActor::BeginPlay()
{
	Super::BeginPlay();

	ParticipantComponent =
		Cast<UConversationParticipantComponent>(AddComponentByClass(UConversationParticipantComponent::StaticClass(), false, FTransform::Identity, false));

	ParticipantComponent->ConversationStarted.AddUObject(this, &ARTSOInteractableConversationActor::ConversationStarted);
	ParticipantComponent->ConversationTaskChoiceDataUpdated.AddUObject(this, &ARTSOInteractableConversationActor::ConversationTaskChoiceDataUpdated);
	ParticipantComponent->ConversationUpdated.AddUObject(this, &ARTSOInteractableConversationActor::ConversationUpdated);
	ParticipantComponent->ConversationStatusChanged.AddUObject(this, &ARTSOInteractableConversationActor::ConversationStatusChanged);
	
	UPDAsyncAction_ActivateFeature* ActiveFeature = UPDAsyncAction_ActivateFeature::CreateActionInstance(this, GameFeatureName);
	ActiveFeature->Activate();
}

void ARTSOInteractableConversationActor::OnInteract_Implementation(
	const FPDInteractionParamsWithCustomHandling& InteractionParams,
	EPDInteractResult& InteractResult) const
{
	Super::OnInteract_Implementation(InteractionParams, InteractResult);

	IPDInteractInterface::OnInteract_Implementation(InteractionParams, InteractResult);
	switch (InteractResult)
	{
	case EPDInteractResult::INTERACT_SUCCESS:
	case EPDInteractResult::INTERACT_FAIL:
	case EPDInteractResult::INTERACT_DELAYED:
		return; // was handled by the delegate in InteractionParams
	case EPDInteractResult::INTERACT_UNHANDLED:
		break; // was not handled by a supplied delegate
	}
	
	if (InteractionParams.InteractionPercent <= .85) { return; }

	const TMap<int32, AActor*>& IDToActorMap =  GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>()->SharedOwnerIDMappings;
	const UMassEntitySubsystem& EntitySubsystem = *GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	const FPDMFragment_RTSEntityBase* EntityBase = EntitySubsystem.GetEntityManager().GetFragmentDataPtr<FPDMFragment_RTSEntityBase>(InteractionParams.InstigatorEntity);

	AActor* SelectedActor = EntityBase != nullptr && IDToActorMap.Contains(EntityBase->OwnerID) ?
		IDToActorMap.FindRef(EntityBase->OwnerID) : InteractionParams.InstigatorActor;

	if (SelectedActor == nullptr) { return; }

	
	const int32 CurrentProgression = InstanceDataPtr->Datum.BaseProgression;
	ERTSOConversationState ConversationState =
		ValidateRequiredTags(ResolveTagForProgressionLevel(CurrentProgression), SelectedActor);
	switch (ConversationState)
	{
	case CurrentStateCompleted:
		// handle based on conversation rules
		if (CanRepeatConversation(CurrentProgression) == false) { return; }
		break;
	case Invalid:
		// @todo fail log
		return;
	case Valid:
		// Continue on as normal 
		break;
	default: return; // ValidateRequiredTags Never returns anything outside of the three above states
	}

	// open conversation here with instigator
	InstanceDataPtr->ActiveConversationInstance =
		Cast<UPDConversationInstance>(
			UConversationLibrary::StartConversation(
			InstanceDataPtr->Datum.PhaseRequiredTags[CurrentProgression].EntryTag,
			(AActor*)this, 
			TAG_Conversation_Participants_Speaker, 
			SelectedActor,
			TAG_Conversation_Participants_Listener,
			UPDConversationInstance::StaticClass()));

	if (InstanceDataPtr->ActiveConversationInstance == nullptr) { return; }
	
	InstanceDataPtr->ActiveConversationInstance->OnBegin_WaitingForChoices.AddDynamic(this, &ARTSOInteractableConversationActor::BeginWaitingForChoices);
	if (InstanceDataPtr->ActiveConversationInstance->bWaitingForChoices)
	{
		// Nasty, I know, but right now I can't mark the function as const and be able to bind it to the dynamic delegate 
		((ARTSOInteractableConversationActor*)this)->BeginWaitingForChoices();
	}
}

void ARTSOInteractableConversationActor::BeginWaitingForChoices() 
{
	const FClientConversationMessagePayload& Payload = ParticipantComponent->GetLastMessage();
	UPDConversationBPFL::PrintConversationMessageToScreen(this, Payload.Message, FLinearColor::Blue);

	for (const FClientConversationOptionEntry& Opt : Payload.Options)
	{
		FName Participant{};
		switch (Opt.ChoiceType)
		{
		case EConversationChoiceType::ServerOnly:
			break;
		case EConversationChoiceType::UserChoiceAvailable:
			break;
		case EConversationChoiceType::UserChoiceUnavailable:
			Participant = FName("(Can't select)");
			break;
		}

		UPDConversationBPFL::PrintConversationTextToScreen(this, Participant, Opt.ChoiceText,FLinearColor::Blue);
	}
}

void ARTSOInteractableConversationActor::ReplyChoice(AActor* Caller, int32 Choice)
{
	const FClientConversationMessagePayload& Payload = ParticipantComponent->GetLastMessage();
	if (Payload.Options.IsValidIndex(Choice)) { return; }

	const FClientConversationOptionEntry& OptionEntry = Payload.Options[Choice];
	
	FAdvanceConversationRequest AdvanceRequest{OptionEntry.ChoiceReference};
	AdvanceRequest.UserParameters = OptionEntry.ExtraData;
	
	UPDConversationBPFL::RequestToAdvance(InstanceDataPtr->ActiveConversationInstance, ParticipantComponent, AdvanceRequest);

	const FString BuildString = "Selected{" + FString::FromInt(Choice) + "}";
	UKismetSystemLibrary::PrintString(this, BuildString, true, true, FLinearColor::Blue, 50);
}

ERTSOConversationState ARTSOInteractableConversationActor::ValidateRequiredTags(const FGameplayTag& EntryTag, AActor* CallingActor) const
{
	ERTSOConversationState ValidatedTagState = ERTSOConversationState::Invalid;
	if (CallingActor == nullptr || CallingActor->GetClass()->ImplementsInterface(URTSOConversationInterface::StaticClass()) == false)
	{
		return ValidatedTagState;
	}
	
	const TSet<FGameplayTag>& ActorTagSet = Cast<IRTSOConversationInterface>(CallingActor)->GetProgressionTagSet();

	const int32 LastConversationProgressionIndex = InstanceDataPtr->Datum.PhaseRequiredTags.Num();
	int32 ConversationProgressionLevel = 0;
	
	for (; ConversationProgressionLevel < LastConversationProgressionIndex ;)
	{
		if (ConversationProgressionLevel > InstanceDataPtr->Datum.BaseProgression) { break; }
		
		const FRTSOConversationRules& RequiredRules =
				InstanceDataPtr->Datum.PhaseRequiredTags[ConversationProgressionLevel];
		ConversationProgressionLevel++;

		// Invalid entry tag, might be too early in that conversation 'tree'
		if (RequiredRules.EntryTag != EntryTag) { continue; }

		// Already completed
		if(RequiredRules.State == ERTSOConversationState::CurrentStateCompleted)
		{
			return ERTSOConversationState::CurrentStateCompleted;
		}

		// Wish epic had implemented some form of set comparator,
		// but in-case it would be an mmo with lets say 5-10 thousand players at most we can expect that many conversations to be held as a max limit,
		// and should be easy for the servers to handle.
		// Realistically at a given moment the amount of validation calls will likely by much fewer
		bool bFoundAllRequiredTags = true; // Default to true if there are no requirements
		for (const FGameplayTag& RequiredTag : RequiredRules.RequiredTags)
		{
			bFoundAllRequiredTags &= ActorTagSet.Contains(RequiredTag); // Needs them all to exist in the actor tag set
		}
		ValidatedTagState = bFoundAllRequiredTags ?
			ERTSOConversationState::Valid : ERTSOConversationState::Invalid; 
	}

	return ValidatedTagState;
}

bool ARTSOInteractableConversationActor::CanRepeatConversation(const int32 ConversationProgression) const
{
	const bool bIsValidIndex = InstanceDataPtr->Datum.PhaseRequiredTags.IsValidIndex(ConversationProgression);
	if (bIsValidIndex == false) { return false; }

	return InstanceDataPtr->Datum.PhaseRequiredTags[ConversationProgression].bCanRepeatConversation;
}

const FGameplayTag& ARTSOInteractableConversationActor::ResolveTagForProgressionLevel(const int32 ConversationProgression) const
{
	const bool bIsValidIndex = InstanceDataPtr->Datum.PhaseRequiredTags.IsValidIndex(ConversationProgression);
	if (bIsValidIndex == false) { return FGameplayTag::EmptyTag; }

	return InstanceDataPtr->Datum.PhaseRequiredTags[ConversationProgression].EntryTag;
}

void ARTSOInteractableConversationActor::OnConversationPhaseStateChanged(const FGameplayTag& EntryTag, ERTSOConversationState NewState)
{
}

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
