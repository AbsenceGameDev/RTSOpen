// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactables/Resources/RTSOInteractableResourceBase.h"

#include "PDInventorySubsystem.h"
#include "PDItemCommon.h"
#include "PDRTSCommon.h"
#include "Components/PDInventoryComponent.h"


ARTSOInteractableResourceBase::ARTSOInteractableResourceBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	JobTag = TAG_AI_Job_GatherResource;
}

void ARTSOInteractableResourceBase::BeginPlay()
{
	Super::BeginPlay();
}

void ARTSOInteractableResourceBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RefreshTickAcc += DeltaTime;
}

void ARTSOInteractableResourceBase::OnInteract_Implementation(const FPDInteractionParamsWithCustomHandling& InteractionParams, EPDInteractResult& InteractResult) const
{
	Super::OnInteract_Implementation(InteractionParams, InteractResult);
	switch (InteractResult)
	{
	case EPDInteractResult::INTERACT_SUCCESS:
	case EPDInteractResult::INTERACT_DELAYED:
	case EPDInteractResult::INTERACT_FAIL:
		// Was handled already by a custom processing delegate
		return;
	case EPDInteractResult::INTERACT_UNHANDLED:
		break;
	}
	
	// InteractionParams.InteractionPercent;  // @todo, need an interaction funciton to actually call OnInteract form the interaction component 
	// if (InteractionParams.InteractionPercent) // @todo ^^^^^^^^^^^^^^^^ , after then the below code will make sense
	
	InteractResult = EPDInteractResult::INTERACT_FAIL;
	if (RefreshTickAcc < RefreshInterval) { return ; }
	(float&)RefreshTickAcc = 0.0;

	// has instigator inv component?
	const AActor* InstigatorActor = InteractionParams.InstigatorActor; 
	UPDInventoryComponent* InvComponent = InstigatorActor != nullptr ? Cast<UPDInventoryComponent>(InstigatorActor->GetComponentByClass(InteractionParams.InstigatorComponentClass)) : nullptr;
	if (InvComponent == nullptr) { return; }

	UPDInventorySubsystem* InvSubsystem = GEngine->GetEngineSubsystem<UPDInventorySubsystem>();
	check(InvSubsystem != nullptr)

	for (const TPair<FGameplayTag, int32 /*count*/>& ResourceReward : LinkedItemResources)
	{
		const FString CtxtString = FString::Printf(TEXT("Entry in LinkedItemResources in interactable(%s) is not pointing to a valid item/resource entry "), *GetName());
		const FPDItemDefaultDatum* DefaultDatum = InvSubsystem->GetDefaultDatum(ResourceReward.Key);
		if (DefaultDatum == nullptr) { continue; }

		InvComponent->RequestUpdateItem(EPDItemNetOperation::ADDNEW, DefaultDatum->ItemTag, ResourceReward.Value);
	}
	
	InteractResult = EPDInteractResult::INTERACT_SUCCESS;
}

FGameplayTagContainer ARTSOInteractableResourceBase::GetGenericTagContainer_Implementation() const
{
	FGameplayTagContainer GeneratedTags;
	GeneratedTags.AddTag(JobTag);
	return GeneratedTags;
}


