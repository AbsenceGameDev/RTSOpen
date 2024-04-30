﻿#include "PDInteractSharedUI.h"
#include "Components/TextBlock.h"

/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "PDInteractSubsystem.h"

#include "PDInteractCommon.h"
#include "Components/PDInteractComponent.h"
#include "Interfaces/PDInteractInterface.h"
#include "Kismet/GameplayStatics.h"


/* @todo rewrite the base/element widget as a slate widget instead */

void UPDWInteractable_Base::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPDWInteractable_Base::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UPDWInteractable_Base::ApplyMessageChanges()
{
	InteractableTitle->SetText(FText::FromString(CurrentMessage.ActorName));
	KeyIndicator_Action->SetText(FText::FromString(CurrentMessage.KeyActionMetadata.ConvertToString()));
	GameMessage_Action->SetText(FText::FromString(CurrentMessage.GameAction));
}

void UPDWInteractable_Base::OnSpawnWidgetPopup(UPDWRadialInteract_HUD* UpdwRadialInteract_HUD)
{
}

void UPDWRadialInteract_HUD::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPDWRadialInteract_HUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const APawn* OwnerPawn = GetOwningPlayerPawn();
	const UPDInteractComponent* InteractComponent = OwnerPawn != nullptr ?
		OwnerPawn->GetComponentByClass<UPDInteractComponent>() : nullptr;
	if (InteractComponent == nullptr || InteractComponent->IsValidLowLevelFast() == false)
	{
		return;
	}
	TArray<AActor*> Incoming = InteractComponent->GetRadialTraceResults();


	TArray<AActor*> KeysToRemove{};
	for (const TPair<AActor*, UPDWInteractable_Base*> ActorPair : OnScreenInteractables)
	{
		UPDWInteractable_Base* Widget = ActorPair.Value;
		if (Incoming.Contains(ActorPair.Key) == false || Widget == nullptr)
		{
			KeysToRemove.Emplace(ActorPair.Key);
			continue;
		}
		
		if (Widget->GetTrackedActor() == nullptr)
		{
			KeysToRemove.Emplace(ActorPair.Key);
			Widget->SetVisibility(ESlateVisibility::Hidden);
			Widget->RemoveFromParent();
			continue;
		}
		
		FWidgetTransform Transform = Widget->GetRenderTransform();
		UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), Widget->GetTrackedActor()->GetActorLocation(),Transform.Translation);
		Widget->SetRenderTransform(Transform);	
	}
	for (AActor* Actor : KeysToRemove)
	{
		OnScreenInteractables.Remove(Actor);
	}
	
	
	// Register remaining actors
	for (AActor* InteractActor : Incoming)
	{
		if (InteractActor != nullptr && InteractActor->Implements<UPDInteractInterface>())
		{
			SpawnNewInteractableWidget(IPDInteractInterface::Execute_GetInteractionMessage(InteractActor), InteractActor);
		}

		if (OnScreenInteractables.Num() >= 50) { break; }
	}


	TickInteractables();	
}

void UPDWRadialInteract_HUD::TickInteractables()
{
	TArray<AActor*> KeysToRemove{};
	for (const TPair<AActor*, UPDWInteractable_Base*> ActorPair : OnScreenInteractables)
	{
		UPDWInteractable_Base* Widget = ActorPair.Value;
		if (Widget == nullptr)
		{
			KeysToRemove.Emplace(ActorPair.Key);
			continue;
		}
		
		if (Widget->GetTrackedActor() == nullptr)
		{
			KeysToRemove.Emplace(ActorPair.Key);
			Widget->SetVisibility(ESlateVisibility::Hidden);
			Widget->RemoveFromParent();
			continue;
		}
		
		FWidgetTransform Transform = Widget->GetRenderTransform();
		UGameplayStatics::ProjectWorldToScreen(GetOwningPlayer(), Widget->GetTrackedActor()->GetActorLocation(),Transform.Translation);
		Widget->SetRenderTransform(Transform);	
	}

	for (AActor* Actor : KeysToRemove)
	{
		OnScreenInteractables.Remove(Actor);
	}
}

APawn* UPDWRadialInteract_HUD::GetOverridePawn_Implementation()
{
	return GetOwningPlayerPawn();
}

void UPDWRadialInteract_HUD::SpawnNewInteractableWidget_Implementation(const FPDInteractMessage& Message, AActor* ActorToTrack)
{
	if (SpawnWidgetInteractableClass == nullptr && OnScreenInteractables.Contains(ActorToTrack)) { return; }
	
	UPDWInteractable_Base* NewBase = CreateWidget<UPDWInteractable_Base>(this, SpawnWidgetInteractableClass);
	NewBase->SetCurrentMessage(Message);
	NewBase->SetTrackedActor(ActorToTrack);
	NewBase->SetVisibility(ESlateVisibility::Visible);
	NewBase->AddToViewport(0);
	NewBase->OnSpawnWidgetPopup(this);
	
	OnScreenInteractables.Emplace(ActorToTrack,NewBase);
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
