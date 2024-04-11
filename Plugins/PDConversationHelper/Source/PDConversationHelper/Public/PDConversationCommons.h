/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "ConversationParticipantComponent.h"
#include "GameFeaturesSubsystem.h"
#include "ConversationInstance.h"

#include "GameFeaturePluginOperationResult.h"
#include "Kismet/BlueprintAsyncActionBase.h"

#include "PDConversationCommons.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCompleted_ActivateFeature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBeginWaitingForChoicesDlgt);

UENUM(Blueprintable, BlueprintType)
enum class EPDKeyProcessResult : uint8 
{
	KEY_SUCCESS UMETA(DisplayName="Valid or whitelisted Key"), 
	KEY_FAIL    UMETA(DisplayName="Invalid or blacklisted Key"),
};

/** @brief This instance adds some flags we want to use alongside with an overridden function so the system works as intended in singleplayer  */
UCLASS(BlueprintType)
class PDCONVERSATIONHELPER_API UPDConversationInstance : public UConversationInstance
{
	GENERATED_BODY()

	/** @brief */
	virtual void PauseConversationAndSendClientChoices(const FConversationContext& Context, const FClientConversationMessage& ClientMessage) override;
public:

	/** @brief Send a request for the conversation to advance/resume */
	UPROPERTY(BlueprintAssignable)
	FBeginWaitingForChoicesDlgt OnBegin_WaitingForChoices;
	
	/** @brief */
	UPROPERTY(BlueprintReadWrite)
	bool bWaitingForChoices = false;
};

/** @brief This class holds some helper and possibly debug functions we might want to use */
UCLASS()
class PDCONVERSATIONHELPER_API UPDConversationBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/** @brief Send a request for the conversation to advance/resume */
	UFUNCTION(BlueprintCallable)
	static void RequestToAdvance(UPDConversationInstance* Conversation, UConversationParticipantComponent* ConversationParticipantComponent, const FAdvanceConversationRequest& InChoicePicked);

	/** @brief Gets the previous/latest message that was sent */
	UFUNCTION(BlueprintCallable)
	static const FClientConversationMessagePayload& GetPreviousMessage(UConversationParticipantComponent* ConversationParticipantComponent);

	/** @brief Gets the conversation task node object related to the task node we are calling from */
	UFUNCTION(BlueprintCallable)
	static const UConversationTaskNode* GetTaskNode(const FConversationContext& Context);
	
	/** @brief Checks with teh game feature subsystem if the given plugin has been loaded and activated */
	UFUNCTION(BlueprintCallable)
	static bool IsGameFeaturePluginActive(FString PluginName);

	/** @brief Prints the message to screen */
	UFUNCTION(BlueprintCallable, Category = "Action|Interface", Meta = (ExpandEnumAsExecs="Results"))
	static int32 ProcessInputKey(const FKey& PressedKey, const TArray<FKey>& ValidKeys, EPDKeyProcessResult& Results);	

	/** @brief Prints the message to screen */
	UFUNCTION(BlueprintCallable)
	static void PrintConversationMessageToScreen(UObject* WorldContext, const FClientConversationMessage& Message, FLinearColor MessageColour);

	/** @brief Prints the message to screen, alternative input parameters */
	UFUNCTION(BlueprintCallable)
	static void PrintConversationTextToScreen(UObject* WorldContext, const FName& Participant, const FText& Text, FLinearColor MessageColour);		
};


/** @brief This class is an async latent action based class, deriving from UBlueprintAsyncActionBase.
 * We dispatch a call from at and it works much like a delay in the graph, as in it resumes the execution from the correct node when a reply has been received.  */
UCLASS()
class PDCONVERSATIONHELPER_API UPDAsyncAction_ActivateFeature : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UPDAsyncAction_ActivateFeature(const FObjectInitializer& ObjectInitializer);

	/** @brief Called when the action activates */
	virtual void Activate() override;
	/** @brief Called when the action completes */
	void Completed(const UE::GameFeatures::FResult& Result);
	
	/** @brief Static helper to create an action instance of this object */
	UFUNCTION(BlueprintCallable, Meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true" ))
	static UPDAsyncAction_ActivateFeature* CreateActionInstance(UObject* WorldContextObject, FString GameFeatureName);
public:
	/** @brief Cached name to the game feature data plugin */
	UPROPERTY()
	FString GameFeatureDataPluginName;
	
	/** @brief Delegate to (possibly) fire at end of the action*/
	UPROPERTY(BlueprintAssignable)
	FCompleted_ActivateFeature Succeeded;

	/** @brief Delegate to (possibly) fire at end of the action*/
	UPROPERTY(BlueprintAssignable)
	FCompleted_ActivateFeature Failed;
};

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