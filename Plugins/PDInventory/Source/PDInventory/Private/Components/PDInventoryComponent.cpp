﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "Components/PDInventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UPDInventoryComponent::UPDInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	ItemList.SetOwningInventory(this);

	// @todo set polling to 0 times per second to enforce stateful replication, do this by setting up a inventory network manager and route all inventory networking calls through it, 1 connection for every x-hundred actors  
}

void UPDInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams Parameters{};
	Parameters.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UPDInventoryComponent, ItemList, Parameters);
}

void UPDInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ItemList.SetOwningInventory(this);
}

void UPDInventoryComponent::RequestUpdateItem(TEnumAsByte<EPDItemNetOperation> RequestedOperation, const FGameplayTag& ItemTag, int32 Count)
{
	// @todo write some validation here so we don't run any code before validating that we are even allowed to make the requested change
	// If dedicated server, allow if possible
	// GetOwner()->GetNetMode() == NM_DedicatedServer


	// Relevant authoritative code
	switch(RequestedOperation) {
	case REMOVEALL:
		ItemList.RemoveAllItemsOfType(const_cast<FGameplayTag&>(ItemTag)); // Actually removing is expensive
		break;
	case ADDNEW:
	case CHANGE:
		ItemList.UpdateItem(const_cast<FGameplayTag&>(ItemTag), Count); // update, applies either addition or subtraction
		break;
		
	default:
		UE_LOG(PDLog_Inventory, Error, TEXT("Called 'UPDInventoryComponent::RequestedUpdateItem', on ItemTag(%s) with an invalid 'RequestedOperation' parameter"), *ItemTag.GetTagName().ToString())
		return; 
	}
	
	GetOwner()->ForceNetUpdate();
}

void UPDInventoryComponent::RequestTradeItems(
	UPDInventoryComponent* Caller,
	const TMap<FGameplayTag, int32>& OfferedItems,
	const TMap<FGameplayTag, int32>& RequestedItems)
{
	if (Caller == nullptr || Caller->IsValidLowLevelFast() == false) { return; }

	// Offer from caller to called
	for (const TPair<FGameplayTag, int32> Item : OfferedItems)
	{
		if (Caller->ItemList.ItemToIndexMapping.Contains(Item.Key) == false) { continue; }
		
		const int32 TotalItemCount = Caller->ItemList.Items[*Caller->ItemList.ItemToIndexMapping.Find(Item.Key)].TotalItemCount;
		if (Item.Value <= TotalItemCount)
		{
			const int32 Final = FMath::Clamp(Item.Value, 0, TotalItemCount);
			Caller->RequestUpdateItem(EPDItemNetOperation::CHANGE, Item.Key, - Final);
			RequestUpdateItem(EPDItemNetOperation::CHANGE, Item.Key, Final);
				
		}
	}
	
	// Request from caller to called
	for (const TPair<FGameplayTag, int32> Item : RequestedItems)
	{
		if (ItemList.ItemToIndexMapping.Contains(Item.Key) == false) { continue; }
		
		const int32 TotalItemCount = ItemList.Items[*ItemList.ItemToIndexMapping.Find(Item.Key)].TotalItemCount;
		if (Item.Value <= TotalItemCount)
		{
			const int32 Final = FMath::Clamp(Item.Value, 0, TotalItemCount);
			RequestUpdateItem(EPDItemNetOperation::CHANGE, Item.Key, - Final);
			Caller->RequestUpdateItem(EPDItemNetOperation::CHANGE, Item.Key, Final);
		}
	}	
}

void UPDInventoryComponent::OnDatumUpdated(FPDItemNetDatum* ItemNetDatum, EPDItemNetOperation Operation)
{
	OnItemUpdated.Broadcast(ItemNetDatum != nullptr ? *ItemNetDatum : FPDItemNetDatum{});
}

bool UPDInventoryComponent::IsAtLastAvailableStack() const
{
	return Stacks.Max != INDEX_NONE && Stacks.Max == Stacks.Current;
}

bool UPDInventoryComponent::CanAfford(const TMap<FGameplayTag, int32> RequestedItems)
{
	return CanAfford(RequestedItems, 1);
}

bool UPDInventoryComponent::CanAfford(const TMap<FGameplayTag, int32> RequestedItems, int32 CountMultiplier)
{
	for (const TTuple<FGameplayTag, int32>& ItemRequest : RequestedItems)
	{
		const FPDItemNetDatum* FoundItem = ItemList.Items.FindByKey(ItemRequest.Key);
		if (FoundItem == nullptr || FoundItem->TotalItemCount < (ItemRequest.Value * CountMultiplier))
		{
			return false;
		}
	}

	return true;	
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
                      4. Must not be resold or repackaged or redistributed as another product, is only allowed to be used within a commercial or non-commercial game project.
                      5. Teams with yearly budgets larger than 100000 USD must contact the owner for a custom license or buy the framework from a marketplace it has been made available on.

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
