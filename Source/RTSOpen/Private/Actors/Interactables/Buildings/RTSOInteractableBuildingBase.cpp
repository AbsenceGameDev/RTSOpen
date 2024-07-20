﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "Actors/Interactables/Buildings/RTSOInteractableBuildingBase.h"

#include "MassEntitySubsystem.h"
#include "PDBuildCommon.h"
#include "PDBuilderSubsystem.h"
#include "PDInventorySubsystem.h"
#include "PDRTSCommon.h"
#include "PDRTSPingerSubsystem.h"
#include "RTSOpenCommon.h"
#include "Actors/GodHandPawn.h"
#include "AI/Mass/RTSOMassFragments.h"
#include "Components/BoxComponent.h"
#include "Components/PDInventoryComponent.h"
#include "Widgets/Slate/SRTSOActionLog.h"

ARTSOInteractableBuildingBase::ARTSOInteractableBuildingBase()
{
	PrimaryActorTick.bCanEverTick = true;

	JobTag = TAG_AI_Job_WalkToTarget;
}

void ARTSOInteractableBuildingBase::RefreshStaleSettings_Ghost()
{
	if (GhostCollisionSettings.bIsSet == false)
	{
		GhostCollisionSettings.bIsSet = true;
		GhostCollisionSettings.ComponentCollisionEnabledSettings.FindOrAdd(MeshName) = ECollisionEnabled::NoCollision;
		GhostCollisionSettings.ComponentCollisionEnabledSettings.FindOrAdd(BoxcompName) = ECollisionEnabled::NoCollision;
		GhostCollisionSettings.bIsActorCollisionEnabled = false;	
	}
}

void ARTSOInteractableBuildingBase::RefreshStaleSettings_Main()
{
	if (BuildableCollisionSettings.bIsSet == false)
	{
		BuildableCollisionSettings.bIsSet = true;
		BuildableCollisionSettings.ComponentCollisionEnabledSettings.FindOrAdd(MeshName) = Mesh->GetCollisionEnabled();
		BuildableCollisionSettings.ComponentCollisionEnabledSettings.FindOrAdd(BoxcompName) = Boxcomp->GetCollisionEnabled();
		BuildableCollisionSettings.bIsActorCollisionEnabled = GetActorEnableCollision();
	}
}

void ARTSOInteractableBuildingBase::BeginPlay()
{
	Super::BeginPlay();
	
	RefreshStaleSettings<true>(); // Refresh ghost
	RefreshStaleSettings<false>(); // Refresh main
}

void ARTSOInteractableBuildingBase::BeginDestroy()
{
	OnBuildingDestroyed();
	Super::BeginDestroy();
}

void ARTSOInteractableBuildingBase::AddTagToCaller_Implementation(AActor* Caller, const FGameplayTag& NewTag)
{
	IRTSOMissionProgressor::AddTagToCaller_Implementation(Caller, NewTag);
}

FGameplayTagContainer ARTSOInteractableBuildingBase::SelectorTagToTagContainer_Implementation(AActor* Caller, const FGameplayTag& SelectorTag)
{
	return MissionProgressionTagsGrantedUponSuccessfulBuild;
}

void ARTSOInteractableBuildingBase::OnBuildSuccessful(AActor* InstigatorActor) const
{
	if (InstigatorActor->GetClass()->ImplementsInterface(UPDRTSBuilderInterface::StaticClass()) )
	{
		const int32 InstigatorID = IPDRTSBuilderInterface::Execute_GetBuilderID(InstigatorActor);
		
		const FRTSOActionLogEvent NewActionEvent{
			FString::Printf(TEXT("OwnerID(%i) -- Successfully built %s "), InstigatorID, *GetName())}; 
		URTSActionLogSubsystem::DispatchEvent(InstigatorID, NewActionEvent);		
	}
	
	if (InstigatorActor == nullptr
		|| InstigatorActor->GetClass()->ImplementsInterface(URTSOConversationInterface::StaticClass()) == false)
	{
		return;
	}

	ARTSOInteractableBuildingBase* MutableThis = const_cast<ARTSOInteractableBuildingBase*>(this);
	IRTSOMissionProgressor::Execute_AddTagContainerToCallerFromSelectorTag(MutableThis, InstigatorActor, FGameplayTag());
}

FGameplayTagContainer ARTSOInteractableBuildingBase::GetGenericTagContainer_Implementation() const
{
	FGameplayTagContainer GeneratedTags;
	GeneratedTags.AddTag(JobTag);

	for (const FGameplayTagContainer& GhostStageTags : MissionProgressionTagsGrantedPerGhostStage)
	{
		GeneratedTags.AppendTags(GhostStageTags);
	}
	
	GeneratedTags.AppendTags(MissionProgressionTagsGrantedUponSuccessfulBuild);
	return GeneratedTags;
}

FGameplayTag ARTSOInteractableBuildingBase::GetInstigatorTag_Implementation() const
{
	return InstigatorBuildableTag;
}

bool ARTSOInteractableBuildingBase::GetCanInteract_Implementation() const
{
	// don't interact with ghosts via the interaction system, ghost state is meant for visualizing
	return (bIsGhost_noSerialize == false || bIsPreviewGhost == false) && Super::GetCanInteract_Implementation();
}

void ARTSOInteractableBuildingBase::OnInteract_Implementation(
	const FPDInteractionParamsWithCustomHandling& InteractionParams,
	EPDInteractResult& InteractResult) const
{
	// If ghost, we can only supply resources if we have any 
	if (bIsGhost_noSerialize)
	{
		const int32& ImmutableStage = CurrentTransitionState.CurrentStageIdx;
		const UMassEntitySubsystem& EntitySubsystem = *GetWorld()->GetSubsystem<UMassEntitySubsystem>();
		const bool bEntityValid = EntitySubsystem.GetEntityManager().IsEntityValid(InteractionParams.InstigatorEntity);

		FRTSOLightInventoryFragment* EntityInv = bEntityValid ? EntitySubsystem.GetEntityManager().GetFragmentDataPtr<FRTSOLightInventoryFragment>(InteractionParams.InstigatorEntity) : nullptr;
		UPDInventoryComponent* Bank = InteractionParams.InstigatorActor != nullptr ? InteractionParams.InstigatorActor->GetComponentByClass<UPDInventoryComponent>() : nullptr;

		ARTSOInteractableBuildingBase* MutableSelf = const_cast<ARTSOInteractableBuildingBase*>(this); // this will bite me in the ass
		// if it enters then it means the stage is finished, 
		if (MutableSelf->WithdrawRecurringCostFromBankOrEntity(Bank, EntityInv, ImmutableStage))
		{
			// Here
			AsyncTask(ENamedThreads::GameThread,
				[&]()
				{
					// if we are false here, then we have not progressed to the end
					if (MutableSelf->CurrentStateFinishedProgressing == false && (MutableSelf->RunningStateProgressFunction == false || ImmutableStage == 0))
					{
						MutableSelf->Internal_ProgressGhostStage(true, false);
					}
				});			
		}
		InteractResult = EPDInteractResult::INTERACT_SUCCESS;
		return;
	}

	Super::OnInteract_Implementation(InteractionParams, InteractResult);
}

void ARTSOInteractableBuildingBase::Internal_ProgressGhostStage(const bool bForceProgressThroughStage, const bool bChainAll)
{
	CurrentStateFinishedProgressing = false;
	RunningStateProgressFunction = true;

	const int32 CachedStageIdx = CurrentTransitionState.CurrentStageIdx;
	
	const auto CallbackEndOfGhostStage =
		[&]()
		{
			// Second dispatch, called within the first dispatch
			// Async task, end stage, max duration based solution: 
			AsyncTask(ENamedThreads::GameThread,
				[&]()
				{
					RunningStateProgressFunction = false;

					// ensure this hasn't been called manually already
					if (CachedStageIdx != CurrentTransitionState.CurrentStageIdx) { return; }

					UPDBuilderSubsystem::ProcessGhostStage(this, InstigatorBuildableTag,  CurrentTransitionState, false);
					CurrentStateFinishedProgressing = true;

					if (GetOwner()->GetClass()->ImplementsInterface(UPDRTSBuilderInterface::StaticClass()))
					{
						const int32 InstigatorID = IPDRTSBuilderInterface::Execute_GetBuilderID(GetOwner());
						
						const FRTSOActionLogEvent NewActionEvent{
							FString::Printf(TEXT("OwnerID(%i) -- Successfully Progressed Build to Stage %i,  of actor %s "),
								InstigatorID, CurrentTransitionState.CurrentStageIdx, *GetName())};
						URTSActionLogSubsystem::DispatchEvent(InstigatorID, NewActionEvent);		
					}
					
					if (bChainAll && UPDBuilderSubsystem::IsPastFinalIndex(InstigatorBuildableTag,CurrentTransitionState) == false)
					{
						Internal_ProgressGhostStage(bForceProgressThroughStage, bChainAll);
					}
				}
			);
		};
	
	// start stage, max duration based solution:
	UPDBuilderSubsystem::ProcessGhostStage(this, InstigatorBuildableTag,  CurrentTransitionState, true);
	
	// Cache to make comparison,
	// if something finishes the stage before the timer runs out then use this to avoid calling 'ProcessGhostStage' with 'bStateOfStage == false' twice
	const double MaxDuration = UPDBuilderSubsystem::GetMaxDurationGhostStage(InstigatorBuildableTag,  CurrentTransitionState);

	// If duration is approx. 0.0 and not forcing progression, rely on handling/triggering the end of stage manually
	if (bForceProgressThroughStage == false && MaxDuration < SMALL_NUMBER)
	{
		RunningStateProgressFunction = false;
		return;
	}
	
	// If duration is approx. 0.0 or below, while forcing, call right away
	if (bForceProgressThroughStage && MaxDuration < SMALL_NUMBER)
	{
		CallbackEndOfGhostStage();
		return;
	}
			
	// If duration is -1, handle right away
	if (MaxDuration <= (-1.0 + SMALL_NUMBER))
	{
		CallbackEndOfGhostStage();
		return;
	}
			
	FTimerHandle ThrowawayHandle{};
	const FTimerDelegate Delegate = FTimerDelegate::CreateLambda(CallbackEndOfGhostStage);
	GetWorld()->GetTimerManager().SetTimer(ThrowawayHandle, Delegate, MaxDuration, false);
}

void ARTSOInteractableBuildingBase::TransitionFromGhostToMain_Implementation()
{
	IPDRTSBuildableGhostInterface::TransitionFromGhostToMain_Implementation();

	IPDRTSBuildableGhostInterface::Execute_ProgressGhostStage(this, true);
}

void ARTSOInteractableBuildingBase::ProgressGhostStage_Implementation(const bool bChainAll)
{
	IPDRTSBuildableGhostInterface::ProgressGhostStage_Implementation(bChainAll);

	AsyncTask(ENamedThreads::GameThread,
		[&]()
		{
			Internal_ProgressGhostStage(false, bChainAll);
		});
}

bool ARTSOInteractableBuildingBase::AttemptFinalizeGhost_Implementation()
{
	const AGodHandPawn* AsGodhand = Cast<AGodHandPawn>(GetOwner());
	const ARTSOController* PC = AsGodhand != nullptr ? AsGodhand->GetController<ARTSOController>() : nullptr;
	if (AsGodhand == nullptr || PC == nullptr)
	{
		UE_LOG(PDLog_RTSO, Error, TEXT("ARTSOInteractableBuildingBase(%s)::ProcessIfWorkersNotRequired -- No valid owner -- AsGodhand == nullptr || PC == nullptr"), *GetName())
		return false;
	}
	
	UPDInventorySubsystem* InventorySubsystem = UPDInventorySubsystem::Get();
	UPDBuilderSubsystem* BuilderSubsystem = UPDBuilderSubsystem::Get();
	ensure(InventorySubsystem != nullptr);
	ensure(BuilderSubsystem != nullptr);
	const int32& ImmutableStage = CurrentTransitionState.CurrentStageIdx;

	BuilderSubsystem->GetBuildableTagFromData(AsGodhand->CurrentBuildableData);
	const FPDItemDefaultDatum* BuildableResourceDatum = InventorySubsystem->GetDefaultDatum(BuilderSubsystem->GetBuildableTagFromData(AsGodhand->CurrentBuildableData)); // this is backwards, redesign, redesign, redesign!
	if (BuildableResourceDatum == nullptr)
	{
		UE_LOG(PDLog_RTSO, Error, TEXT("ARTSOInteractableBuildingBase::ProcessIfWorkersNotRequired -- Did not find any entry in UPDInventorySubsystem for the given resource/building"))
		return false;
	}
	
	/// @note using the 'initial cost' data for complete build
	const bool bCanAfford = UPDInventorySubsystem::CanInventoryAffordItem(AsGodhand->InventoryComponent->ItemList, *BuildableResourceDatum, true, false, ImmutableStage);
	if (bCanAfford == false)
	{
		UE_LOG(PDLog_RTSO, Warning, TEXT("ARTSOInteractableBuildingBase::ProcessIfWorkersNotRequired -- Could not afford, setting to queue"))

		// FILO
		BuilderSubsystem->BuildQueues.FindOrAdd(PC->GetActorID()).ActorInWorld.EmplaceFirst(this);
		AsGodhand->InventoryComponent->OnItemUpdated.AddDynamic(AsGodhand, &AGodHandPawn::OnItemUpdate);
		return false;
	}

	for (TTuple<FGameplayTag, FPDItemCosts> CraftingCost : BuildableResourceDatum->CraftingCosts)
	{
		AsGodhand->InventoryComponent->RequestUpdateItem(EPDItemNetOperation::CHANGE, CraftingCost.Key, -CraftingCost.Value.InitialCost);
	}

	// Progress from current stage to finish
	IPDRTSBuildableGhostInterface::Execute_TransitionFromGhostToMain(this);
	return true;
}

FRTSOBuildableInventories& ARTSOInteractableBuildingBase::ReturnBuildableInventories()
{
	return BuildableInventories;
}

template<bool TIsGhost>
void ARTSOInteractableBuildingBase::ProcessSpawn()
{
	RefreshStaleSettings<TIsGhost>();

	UMaterialInstance* SelectedInstancedMat = TIsGhost ? GhostMat : MainMat;
	const FPDRTSBuildableCollisionSettings& SelectedSettings = TIsGhost ? GhostCollisionSettings : BuildableCollisionSettings; 

	Mesh->SetMaterial(0, SelectedInstancedMat);
	Mesh->SetCollisionEnabled(SelectedSettings.ComponentCollisionEnabledSettings.FindRef(MeshName));
	Boxcomp->SetCollisionEnabled(SelectedSettings.ComponentCollisionEnabledSettings.FindRef(BoxcompName));
	SetActorEnableCollision(SelectedSettings.bIsActorCollisionEnabled);

	bIsGhost_noSerialize = TIsGhost;
	if constexpr (TIsGhost == false)
	{
		OnBuildSuccessful(GetOwner());
	}
}

bool ARTSOInteractableBuildingBase::WithdrawRecurringCostFromBankOrEntity(UPDInventoryComponent* Bank, FRTSOLightInventoryFragment* EntityInv, const int32& ImmutableStage)
{
	bool bCanAfford = false;
			
	UPDInventorySubsystem* InventorySubsystem = UPDInventorySubsystem::Get();
	ensure(InventorySubsystem != nullptr);
	
	const FPDItemDefaultDatum* BuildableResourceDatum = InventorySubsystem->GetDefaultDatum(InstigatorBuildableTag);
	if (BuildableResourceDatum != nullptr)
	{
		// Pay the recurring cost
		if (Bank != nullptr)
		{
			// In this context CanInventoryAffordItem returns true only if the full amount is afforded right away
			bCanAfford = UPDInventorySubsystem::CanInventoryAffordItem(Bank->ItemList, *BuildableResourceDatum, true, true, ImmutableStage);
			if (bCanAfford)
			{
				for (const TTuple<FGameplayTag, FPDItemCosts>& ItemCostsTuple : BuildableResourceDatum->CraftingCosts)
				{
					const TArray<int32>& RecurringCostPerPhase = ItemCostsTuple.Value.RecurringCostPerPhase;
					
					// Deduct from player
					Bank->RequestUpdateItem(EPDItemNetOperation::CHANGE, BuildableResourceDatum->ItemTag, -RecurringCostPerPhase[ImmutableStage]);

					// Insert into buildable inv
					if (ImmutableStage > BuildableInventories.LightInventoriesPerGhostStage.Num())
					{
						BuildableInventories.LightInventoriesPerGhostStage.SetNum(ImmutableStage + 1);
					}
					BuildableInventories.LightInventoriesPerGhostStage[ImmutableStage].Handler.AddItem(BuildableResourceDatum->ItemTag, RecurringCostPerPhase[ImmutableStage]);
				}
			}
		}
		else if (EntityInv != nullptr)
		{
			// Pay the recurring cost, don't fail if we can't afford all at once, this is meant for incremental progress
			// bCanAfford = UPDInventorySubsystem::CanInventoryAffordItem(EntityInv->Inner, *BuildableResourceDatum, true, true, ImmutableStage);
			bCanAfford = true;
			for (const TTuple<FGameplayTag, FPDItemCosts>& ItemCostsTuple : BuildableResourceDatum->CraftingCosts)
			{
				const TArray<int32>& RecurringCostPerPhase = ItemCostsTuple.Value.RecurringCostPerPhase;
			
				// Deduct from entity, @note 'AddItems' Clamps to zero so won't offset into negative
				const int32 CurrentCount = EntityInv->Handler.GetItemCount(ItemCostsTuple.Key);
				EntityInv->Handler.AddItem(ItemCostsTuple.Key, -RecurringCostPerPhase[ImmutableStage]);
				
				// Insert into buildable inv
				if (ImmutableStage > BuildableInventories.LightInventoriesPerGhostStage.Num())
				{
					BuildableInventories.LightInventoriesPerGhostStage.SetNum(ImmutableStage + 1);
				}
				FRTSOLightInventoryFragmentHandler& BuildableInvHandler = BuildableInventories.LightInventoriesPerGhostStage[ImmutableStage].Handler;
				BuildableInvHandler.AddItem(BuildableResourceDatum->ItemTag, CurrentCount);

				// past stage requirements
				if (BuildableInvHandler.GetItemCount(BuildableResourceDatum->ItemTag) >= RecurringCostPerPhase[ImmutableStage])
				{
					bCanAfford = true;
				}

			}
		}
	}

	// 
	if (bCanAfford == false)
	{
		UE_LOG(PDLog_RTSO, Error, TEXT("ARTSOInteractableBuildingBase::ProcessIfWorkersRequired -- Tried withdrawing from player bank but player bank was short on resources needed"))
		return false;
	}	

	// @done BuildableInventories keeps track of already received resources, make use of later when loading/saving a buildable to file 
	return true;
}

void ARTSOInteractableBuildingBase::ProcessIfWorkersRequired()
{
	const UPDBuilderSubsystemSettings* DefaultSubsystemSetting = GetDefault<UPDBuilderSubsystemSettings>();
	const PD::Build::Behaviour::Cost BuildCostBehaviour = DefaultSubsystemSetting->DefaultBuildSystemBehaviours.Cost;

	const int32& ImmutableStage = CurrentTransitionState.CurrentStageIdx;
	
	check(GetOwner() != nullptr) // This function should never be called on a non-owning pawn 
	UPDInventoryComponent* PlayerBank = GetOwner()->GetComponentByClass<UPDInventoryComponent>();
	check(PlayerBank != nullptr) // This function should never be called on a pawn that does not have a inventory 
	
	switch (BuildCostBehaviour)
	{
	case PD::Build::Behaviour::Cost::EPlayerBank:
		// Withdraw immediately, exit function it fails
		if (WithdrawRecurringCostFromBankOrEntity(PlayerBank, nullptr, ImmutableStage) == false)
		{
			return;
		}
		break;
	case PD::Build::Behaviour::Cost::EBaseBank:
		// Withdraw immediately, exit function it fails
		{
			UPDInventoryComponent* AssignedBaseBank = nullptr; // @todo write some system to handle bases and make us of it here
			if (WithdrawRecurringCostFromBankOrEntity(AssignedBaseBank,nullptr, ImmutableStage) == false)
			{
				return;
			}
		}
		break;
	case PD::Build::Behaviour::Cost::EStandBy:
		// @done Withdraw from workers when they arrive
		break;
		
	case PDBuildBehaviourCost::EFree:
		break;
	}

	// Start the current stage, it's settings decide when effects are applied and if workers should end stage or if it ends automatically
	IPDRTSBuildableGhostInterface::Execute_ProgressGhostStage(this, false);
	
	const PD::Build::Behaviour::Progress BuildProgressionBehaviour = DefaultSubsystemSetting->DefaultBuildSystemBehaviours.Progression;
	switch (BuildProgressionBehaviour)
	{
	case PDBuildBehaviourProgress::EImmediate: // Immediate does not matter in this function, if immediate it will not hit this function
	case PD::Build::Behaviour::Progress::EStandBy:
		// just wait
		break;
	case PD::Build::Behaviour::Progress::ECallForWorkers:
		{
			constexpr double PingInterval = 10.0; // @todo make configurable or use passthrough value
			const FGameplayTagContainer TagContainer = Execute_GetGenericTagContainer(this);
			const FGameplayTag SelectedTag = TagContainer.IsEmpty() ? JobTag : TagContainer.GetByIndex(0);
			const FPDEntityPingDatum ConstructedDatum = {this, SelectedTag, INDEX_NONE, PingInterval};

			UPDEntityPinger::EnablePingStatic(ConstructedDatum);
		}
		break;
	}
}

void ARTSOInteractableBuildingBase::ProcessIfWorkersNotRequired()
{
	IPDRTSBuildableGhostInterface::Execute_AttemptFinalizeGhost(this);
}

void ARTSOInteractableBuildingBase::OnBuildingDestroyed_Implementation()
{
	UPDBuilderSubsystem* BuilderSubsystem = UPDBuilderSubsystem::Get();
	ensure(BuilderSubsystem != nullptr);
	BuilderSubsystem->QueueRemoveFromWorldBuildTree(GetUniqueID());

	const AGodHandPawn* AsGodhand = Cast<AGodHandPawn>(GetOwner());
	const ARTSOController* PC = AsGodhand != nullptr ? AsGodhand->GetController<ARTSOController>() : nullptr;
	if (AsGodhand == nullptr || PC == nullptr)
	{
		UE_LOG(PDLog_RTSO, Error, TEXT("ARTSOInteractableBuildingBase(%s)::ProcessIfWorkersNotRequired -- No valid owner -- AsGodhand == nullptr || PC == nullptr"), *GetName())
		return;
	}
}

void ARTSOInteractableBuildingBase::OnSpawnedAsGhost_Implementation(const FGameplayTag& BuildableTag, bool bInIsPreviewGhost, bool bInRequiresWorkersToBuild)
{
	IPDRTSBuildableGhostInterface::OnSpawnedAsGhost_Implementation(BuildableTag, bInIsPreviewGhost, bInRequiresWorkersToBuild);
	InstigatorBuildableTag = BuildableTag;

	// Update material
	if (GhostMat == nullptr || GhostMat->IsValidLowLevelFast() == false)
	{
		// Output error level log
		UE_LOG(PDLog_RTSO, Error, TEXT("ARTSOInteractableBuildingBase::OnSpawnedAsGhost -- Material Instance member 'GhostMat' is not set"))
		
		return;
	}
	
	ProcessSpawn<true>();

	bIsPreviewGhost = bInIsPreviewGhost;
	if (bIsPreviewGhost)  { return; };
	
	if (bInRequiresWorkersToBuild)
	{
		ProcessIfWorkersRequired();
	}
	else
	{
		ProcessIfWorkersNotRequired();
	}
}

void ARTSOInteractableBuildingBase::OnSpawnedAsMain_Implementation(const FGameplayTag& BuildableTag)
{
	IPDRTSBuildableGhostInterface::OnSpawnedAsMain_Implementation(BuildableTag);
	InstigatorBuildableTag = BuildableTag;
	ProcessSpawn<false>();
	
	// Update material
	if (MainMat == nullptr || MainMat->IsValidLowLevelFast() == false)
	{
		// Output error level log
		UE_LOG(PDLog_RTSO, Error, TEXT("ARTSOInteractableBuildingBase::OnSpawnedAsMain -- Material Instance member 'MainMat' is not set"))
		
		return;
	}
}

void ARTSOInteractableBuildingBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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