﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "Actors/Interactables/Resources/RTSOInteractableResourceBase.h"

#include "MassEntitySubsystem.h"
#include "PDInventorySubsystem.h"
#include "PDItemCommon.h"
#include "PDRTSCommon.h"
#include "AI/Mass/RTSOMassFragments.h"
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

	EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
	EntManager = &EntitySubsystem->GetEntityManager();
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
	
	InteractResult = EPDInteractResult::INTERACT_FAIL;
	if (RefreshTickAcc < RefreshInterval) { return ; }
	(float&)RefreshTickAcc = 0.0;

	// has instigator inv component?
	
	const AActor* InstigatorActor = InteractionParams.InstigatorActor;
	
	UPDInventoryComponent* InvComponent = InstigatorActor != nullptr ? Cast<UPDInventoryComponent>(InstigatorActor->GetComponentByClass(InteractionParams.InstigatorComponentClass)) : nullptr;
	FRTSOLightInventoryFragment* AsInventoryFragment = nullptr;// static_cast<FRTSOLightInventoryFragment*>(InteractionParams.InstigatorEntity);
	if (InvComponent == nullptr)
	{
		AsInventoryFragment = EntManager->GetFragmentDataPtr<FRTSOLightInventoryFragment>(InteractionParams.InstigatorEntity);
		if (AsInventoryFragment == nullptr)
		{
			return;
		}
	}

	UPDInventorySubsystem* InvSubsystem = UPDInventorySubsystem::Get();
	check(InvSubsystem != nullptr)

	for (const TPair<FGameplayTag, int32 /*count*/>& ResourceReward : LinkedItemResources)
	{
		const FString CtxtString = FString::Printf(TEXT("Entry in LinkedItemResources in interactable(%s) is not pointing to a valid item/resource entry "), *GetName());
		const FPDItemDefaultDatum* DefaultDatum = InvSubsystem->GetDefaultDatum(ResourceReward.Key);
		if (DefaultDatum == nullptr) { continue; }

		if (AsInventoryFragment != nullptr)
		{
			AsInventoryFragment->Handler.AddItem(DefaultDatum->ItemTag, ResourceReward.Value);
		}
		if (InvComponent != nullptr)
		{
			InvComponent->RequestUpdateItem(EPDItemNetOperation::ADDNEW, DefaultDatum->ItemTag, ResourceReward.Value);
		}
	}
	
	InteractResult = EPDInteractResult::INTERACT_SUCCESS;
}

FGameplayTagContainer ARTSOInteractableResourceBase::GetGenericTagContainer_Implementation() const
{
	FGameplayTagContainer GeneratedTags;
	GeneratedTags.AddTag(JobTag);
	return GeneratedTags;
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