/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "PDConversationCommons.h"
#include "GameFeaturesSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

//
// Conversation instance

void UPDConversationInstance::PauseConversationAndSendClientChoices(
	const FConversationContext& Context,
	const FClientConversationMessage& ClientMessage)
{
	Super::PauseConversationAndSendClientChoices(Context, ClientMessage);
	bWaitingForChoices = true;
	OnBegin_WaitingForChoices.Broadcast();
}

//
// Conversation function lib
void UPDConversationBPFL::RequestToAdvance(UPDConversationInstance* Conversation, UConversationParticipantComponent* ConversationParticipantComponent, const FAdvanceConversationRequest& InChoicePicked)
{
	if(Conversation == nullptr || ConversationParticipantComponent == nullptr) { return; }

	Conversation->bWaitingForChoices = false;
	ConversationParticipantComponent->RequestServerAdvanceConversation(InChoicePicked);
}

const FClientConversationMessagePayload& UPDConversationBPFL::GetPreviousMessage(UConversationParticipantComponent* ConversationParticipantComponent)
{
	check(ConversationParticipantComponent)
	return ConversationParticipantComponent->GetLastMessage();
}

const UConversationTaskNode* UPDConversationBPFL::GetTaskNode(const FConversationContext& Context)
{
	return Context.GetTaskBeingConsidered();
}

bool UPDConversationBPFL::IsGameFeaturePluginActive(FString PluginName)
{
	UGameFeaturesSubsystem* GameFeaturesSubsystem = GEngine ? GEngine->GetEngineSubsystem<UGameFeaturesSubsystem>() : nullptr;
	if (GameFeaturesSubsystem == nullptr) { return false; }
	
	FString PluginURL;
	const bool bFoundPlugin = UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName,  PluginURL);
	if (bFoundPlugin) { return GameFeaturesSubsystem->IsGameFeaturePluginActive(PluginURL); }

	return false;
}

int32 UPDConversationBPFL::ProcessInputKey(const FKey& PressedKey, const TArray<FKey>& ValidKeys, EPDKeyProcessResult& Results)
{
	Results = ValidKeys.Contains(PressedKey)
		? EPDKeyProcessResult::KEY_SUCCESS
		: EPDKeyProcessResult::KEY_FAIL;

	return ValidKeys.Find(PressedKey);
}

void UPDConversationBPFL::PrintConversationMessageToScreen(UObject* WorldContext, const FClientConversationMessage& Message, FLinearColor MessageColour)
{
	const FString BuildString = Message.ParticipantDisplayName.ToString() + " : " + Message.Text.ToString();
	UKismetSystemLibrary::PrintString(WorldContext, BuildString, true, true, MessageColour, 50);
}

void UPDConversationBPFL::PrintConversationTextToScreen(UObject* WorldContext, const FName& Participant, const FText& Text, FLinearColor MessageColour)
{
	const FString BuildString = Participant.ToString() + " : " + Text.ToString();
	UKismetSystemLibrary::PrintString(WorldContext, BuildString, true, true, MessageColour, 50);	
}


//
// Conversation async action
UPDAsyncAction_ActivateFeature::UPDAsyncAction_ActivateFeature(const FObjectInitializer& ObjectInitializer)
{
}

UPDAsyncAction_ActivateFeature* UPDAsyncAction_ActivateFeature::CreateActionInstance(UObject* WorldContextObject, FString GameFeatureName)
{
	UPDAsyncAction_ActivateFeature* Activator = NewObject<UPDAsyncAction_ActivateFeature>();
	Activator->GameFeatureDataPluginName = GameFeatureName;
	Activator->RegisterWithGameInstance(WorldContextObject);
	return Activator;
}

void UPDAsyncAction_ActivateFeature::Activate()
{
	Super::Activate();

	FString PluginURL;
	UGameFeaturesSubsystem* GameFeaturesSubsystem = GEngine ? GEngine->GetEngineSubsystem<UGameFeaturesSubsystem>() : nullptr;
	if (GameFeaturesSubsystem != nullptr && UGameFeaturesSubsystem::Get().GetPluginURLByName(GameFeatureDataPluginName,  PluginURL))
	{
		FGameFeaturePluginLoadComplete GameFeaturePluginLoadComplete;
		GameFeaturePluginLoadComplete.BindUObject(this, &UPDAsyncAction_ActivateFeature::Completed);
		GameFeaturesSubsystem->LoadAndActivateGameFeaturePlugin(PluginURL, GameFeaturePluginLoadComplete);
		return;
	}

	Failed.Broadcast();
	SetReadyToDestroy();
}

void UPDAsyncAction_ActivateFeature::Completed(const UE::GameFeatures::FResult& Result)
{
	Result.HasError() ?
		Failed.Broadcast()
		: Succeeded.Broadcast(); 
	
	SetReadyToDestroy();
}

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