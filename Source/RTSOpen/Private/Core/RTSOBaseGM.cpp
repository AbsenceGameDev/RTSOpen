/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "Core/RTSOBaseGM.h"
#include "Core/RTSOBaseGI.h"

#include "RTSOpenCommon.h"
#include "PDInteractSubsystem.h"
#include "PDRTSBaseSubsystem.h"
#include "Actors/GodHandPawn.h"
#include "Actors/RTSOController.h"
#include "Actors/Interactables/ConversationHandlers/RTSOInteractableConversationActor.h"
#include "Widgets/RTSOMainMenuBase.h"

#include "MassCommonFragments.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

const FString ARTSOBaseGM::ROOTSAVE = "ROOTSAVE" ; 

ARTSOBaseGM::ARTSOBaseGM(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = AGodHandPawn::StaticClass();
	PlayerControllerClass = ARTSOController::StaticClass();
	// HUDClass;
	// GameStateClass;
	// PlayerStateClass;
	// SpectatorClass;
}


void ARTSOBaseGM::BeginPlay()
{
	Super::BeginPlay();
	
	GameSave = Cast<URTSOpenSaveGame>(UGameplayStatics::CreateSaveGameObject(URTSOpenSaveGame::StaticClass()));
	
	URTSOBaseGI* GI = Cast<URTSOBaseGI>(GetWorld()->GetGameInstance());
	if (GI == nullptr) { return; }
	// GI->StartTransition();

	MARK_PROPERTY_DIRTY_FROM_NAME(URTSOBaseGI, bGMReady, GI);
	GI->bGMReady = true;
}

// @todo expand this into a real autosave function, remove any dependency on tickrate
constexpr int32 INT32_HALFMAX = INT32_MAX / 2; 
constexpr int32 StepTickLimit = 60 /*~tickspersecond*/ * 60 /*secondsperminute*/ * 60 /*approx. minutes*/;

void ARTSOBaseGM::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);

	static int32 CheapMansDelay = 0;
	++CheapMansDelay %= INT32_HALFMAX;

	if ((CheapMansDelay % StepTickLimit) == 0)
	{
		SaveConversationActorStates();
		SaveInteractables();

		// @todo finish SaveResources & SaveUnits and then call them here as-well
	}
}

void ARTSOBaseGM::OnGeneratedLandscapeReady_Implementation()
{
	// Load file data if it exists
}

void ARTSOBaseGM::LoadGame_Implementation(const FString& Slot, const bool bDummyParam)
{
	const FString SelectedSlot = Slot == FString() ? ARTSOBaseGM::ROOTSAVE : Slot;
	
	const bool bDoesExist = UGameplayStatics::DoesSaveGameExist(SelectedSlot,0);
	if (bDoesExist)
	{
 		GameSave = Cast<URTSOpenSaveGame>(UGameplayStatics::LoadGameFromSlot(SelectedSlot,0));
		return;
	}
	
	GameSave = Cast<URTSOpenSaveGame>(UGameplayStatics::CreateSaveGameObject(URTSOpenSaveGame::StaticClass()));
}


void ARTSOBaseGM::ClearSave_Implementation(const bool bRefreshSeed)
{
	check(GameSave != nullptr)

	GameSave->GameTime = 0;
	GameSave->Interactables.Empty();
	GameSave->Inventories.Empty();

	if (bRefreshSeed)
	{
		GameSave->Seeder = UKismetMathLibrary::MakeRandomStream(FMath::RandRange(0, INT_MAX));
	}
	
	bHasStartedSaveData = false;
}

void ARTSOBaseGM::SaveGame_Implementation(const FString& Slot, const bool bAllowOverwrite)
{
	check(GameSave != nullptr)

	const FString SelectedSlot = Slot == FString() ? ARTSOBaseGM::ROOTSAVE : Slot;
	
	const bool bDoesExist = UGameplayStatics::DoesSaveGameExist(SelectedSlot,0);
	if (bDoesExist && bAllowOverwrite == false) { return; }
	
	bHasSavedDataAsync = false;
	FAsyncSaveGameToSlotDelegate Dlgt = FAsyncSaveGameToSlotDelegate::CreateLambda(
		[This = this](const FString&, int, bool bResult)
		{
			if (This == nullptr) { return; }
			This->bHasSavedDataAsync = bResult; 
		});
	
	UGameplayStatics::AsyncSaveGameToSlot(GameSave, SelectedSlot, 0, Dlgt);
	bHasStartedSaveData = true;
}

void ARTSOBaseGM::ProcessChangesAndSaveGame_Implementation(const FString& Slot, const bool bAllowOverwrite)
{
	// Async call might be needed here and then after it has finished call SaveGame()
	SaveConversationActorStates();
	SaveInteractables();
	SaveAllPlayerStates();
	SaveEntities();

	// @todo finish SaveResources & SaveUnits and then call them here as-well
	
	// @todo @note SaveConversationProgression(); is not needed as we update it directly anytime a player makes conversation progress on the server


	// Final step, call to write this data to disk
	SaveGame(Slot, bAllowOverwrite);
}

// Not needed, data updated directly after each choice injunction,
// keep here and reserve it for possible use if the data-structure we want to store data from grows larger/more complex
void ARTSOBaseGM::SaveConversationProgression_Implementation()
{
}

void ARTSOBaseGM::SaveConversationActorStates_Implementation()
{
	check(GameSave != nullptr)

	URTSOConversationActorTrackerSubsystem* Tracker =GetWorld()->GetSubsystem<URTSOConversationActorTrackerSubsystem>();
	check(Tracker != nullptr)

	for (ARTSOInteractableConversationActor* ConversationActor : Tracker->TrackedConversationActors)
	{
		if (ConversationActor == nullptr) { continue; }
		
		FPDPersistentID IDStruct =
			ConversationActor->ConversationActorPersistentID.IsValidID()
			? ConversationActor->ConversationActorPersistentID.GetID()
			: FPDPersistentID::GenerateNewPersistentID();
		
		FRTSSavedConversationActorData& State = GameSave->ConversationActorState.FindOrAdd(IDStruct.GetID());
		State.Health = 1.0; // Force full health for now
		State.Location = ConversationActor->GetActorLocation();
		State.ActorClassType = TSoftClassPtr<ARTSOInteractableConversationActor>(ConversationActor->GetClass());
		
		State.ProgressionPerPlayer.Empty();
		for (TTuple<int32/*PlayerID*/, int32/**/> PlayerInstanceData : ConversationActor->InstanceData.ProgressionPerPlayer)
		{
			State.ProgressionPerPlayer.FindOrAdd(PlayerInstanceData.Key) = PlayerInstanceData.Value;
		}
	}

	// @todo: store current active savegame slot index and use here, remove placeholder of 'INDEX_NONE'
	SaveGame(FString::FromInt(INDEX_NONE));	
}

void ARTSOBaseGM::SaveAllPlayerStates()
{
	URTSOConversationActorTrackerSubsystem* Tracker =GetWorld()->GetSubsystem<URTSOConversationActorTrackerSubsystem>();
	check(Tracker != nullptr)

	for (ARTSOController* Controller : Tracker->TrackedPlayerControllers)
	{
		SavePlayerState(Controller);
	}
}

void ARTSOBaseGM::SavePlayerState_Implementation(ARTSOController* PlayerController)
{
	check(GameSave != nullptr)

	// todo store player stats here as-well when I've finished writing the progression system 
	GameSave->PlayerLocations.FindOrAdd(PlayerController->GetActorID()) = PlayerController->GetPawn()->GetActorLocation();
}

void ARTSOBaseGM::SaveInteractables_Implementation()
{
	check(GameSave != nullptr)
	check(GEngine->GetEngineSubsystem<UPDInteractSubsystem>() != nullptr)
	GameSave->Interactables.Empty();

	FPDArrayListWrapper& SaveInfo = *GEngine->GetEngineSubsystem<UPDInteractSubsystem>()->WorldInteractables.Find(GetWorld());
	for (FRTSSavedInteractables& _Actor: SaveInfo.ActorInfo)
	{
		// Update class if needed
		if (_Actor.ActorInWorld == nullptr)
		{
			_Actor.Usability = 0.0;
			continue;
		}

		// Update class if needed
		if (_Actor.ActorClass == nullptr)
		{
			_Actor.ActorClass = _Actor.ActorInWorld->GetClass();
		}
		
		// update location
		_Actor.Location = _Actor.ActorInWorld->GetActorLocation();
	}
	
	GameSave->Interactables.Append(SaveInfo.ActorInfo);

	// @todo: store current active savegame slot index and use here, remove placeholder of 'INDEX_NONE'
	SaveGame(FString::FromInt(INDEX_NONE));
}

void ARTSOBaseGM::SaveResources_Implementation(const TMap<int32, FRTSSavedItems>& DataRef)
{
	check(GameSave != nullptr)
	GameSave->Inventories.Append(DataRef); // overwrite any previous value, this is low so only allow saving resources every 3 seconds

	const FLatentActionInfo DelayInfo{0,0, TEXT("SaveGame"), this};
	UKismetSystemLibrary::Delay(GetOwner(), 4.0f, DelayInfo);
}

/* Save current units, their locations and actor classes*/
void ARTSOBaseGM::SaveEntities_Implementation()
{
	check(GameSave != nullptr)

	UPDRTSBaseSubsystem* RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	FMassEntityManager* EntityManager = RTSSubsystem != nullptr ? RTSSubsystem->EntityManager : nullptr; 
	if (EntityManager == nullptr) { return; }

	// 1. get all our entities
	RTSSubsystem->WorldOctree.Lock();
	RTSSubsystem->WorldOctree.FindAllElements
	(
		[&](const FPDEntityOctreeCell& Cell)
		{
			// 2. Store entities per active player
			if (EntityManager->IsEntityValid(Cell.EntityHandle)) { return; }

			// @todo (might not be needed) 3. Store entity config type 
			FRTSSavedWorldUnits UnitDatum{};
			const FPDMFragment_RTSEntityBase& EntityBaseFragment = EntityManager->GetFragmentDataChecked<FPDMFragment_RTSEntityBase>(Cell.EntityHandle);

			// 4. Store entities active task/job data and stats
			UnitDatum.CurrentAction = EntityManager->GetFragmentDataChecked<FPDMFragment_Action>(Cell.EntityHandle);
			UnitDatum.OwnerID = EntityBaseFragment.OwnerID;
			UnitDatum.SelectionIndex = EntityBaseFragment.SelectionGroupIndex;
			UnitDatum.Location = EntityManager->GetFragmentDataChecked<FTransformFragment>(Cell.EntityHandle).GetTransform().GetLocation();
			GameSave->EntityUnits.Emplace(UnitDatum);
		}
	);

	
	RTSSubsystem->WorldOctree.Unlock();

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