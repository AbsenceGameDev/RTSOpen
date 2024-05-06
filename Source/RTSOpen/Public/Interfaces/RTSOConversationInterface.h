// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "RTSOConversationInterface.generated.h"

struct FGameplayTag;
// This class does not need to be modified.
UINTERFACE()
class URTSOConversationSpeakerInterface : public UInterface
{
	GENERATED_BODY()
};

class RTSOPEN_API IRTSOConversationSpeakerInterface
{
	GENERATED_BODY()

public:
	virtual void BeginWaitingForChoices() = 0;

	virtual void ReplyChoice(AActor* Caller, int32 Choice) = 0;
};

// This class does not need to be modified.
UINTERFACE()
class URTSOConversationInterface : public UInterface
{
	GENERATED_BODY()
};

class RTSOPEN_API IRTSOConversationInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddUniqueProgressionTag(const FGameplayTag& NewTag);
	virtual void AddUniqueProgressionTag_Implementation(const FGameplayTag& NewTag)
	{
		AcquiredConversationProgressionTags.FindOrAdd(NewTag);
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void RemoveProgressionTag(const FGameplayTag& TagToRemove);
	virtual void RemoveProgressionTag_Implementation(const FGameplayTag& TagToRemove)
	{
		if (AcquiredConversationProgressionTags.Contains(TagToRemove))
		{
			AcquiredConversationProgressionTags.Remove(TagToRemove);
		}
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AddUniqueProgressionTagSet(const TSet<FGameplayTag>& NewTags);
	virtual void AddUniqueProgressionTagSet_Implementation(const TSet<FGameplayTag>& NewTags)
	{
		AcquiredConversationProgressionTags.Append(NewTags);
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void RemoveProgressionTagSet(const TSet<FGameplayTag>& TagsToRemove);
	virtual void RemoveProgressionTagSet_Implementation(const TSet<FGameplayTag>& TagsToRemove)
	{
		for (const FGameplayTag& Tag : TagsToRemove)
		{
			AcquiredConversationProgressionTags.Remove(Tag);
		}
	}
	
	virtual const TSet<FGameplayTag>& GetProgressionTagSet()
	{
		return AcquiredConversationProgressionTags;
	}

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool HasProgressionTag(const FGameplayTag& CompareTag);
	virtual bool HasProgressionTag_Implementation(const FGameplayTag& CompareTag)
	{
		return AcquiredConversationProgressionTags.Contains(CompareTag);
	}
	
public:
	TSet<FGameplayTag> AcquiredConversationProgressionTags{};
};
