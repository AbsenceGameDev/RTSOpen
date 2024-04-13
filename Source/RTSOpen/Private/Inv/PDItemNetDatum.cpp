/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "Inv/PDItemNetDatum.h"

#include "Inv/PDBuildCommons.h"
#include "Inv/PDInventoryComponent.h"
#include "Inv/PDInventorySubsystem.h"

static TMap<int32,int32> ConstructTMapInPlace(int32 StackIdx, int32 Count)
{
	TMap<int32,int32> ConstructedMap;
	ConstructedMap.Emplace(StackIdx) = Count;
	return ConstructedMap;
}

FPDItemNetDatum::FPDItemNetDatum(const FGameplayTag& InItemTag, int32 StackIndex, int32 InCount)
	: ItemTag(InItemTag), LastEditedStackIndex(StackIndex), Stacks(ConstructTMapInPlace(StackIndex, InCount))
{
}

FPDItemNetDatum::FPDItemNetDatum(const FGameplayTag& InItemTag, int32 InCount)
	: ItemTag(InItemTag), Stacks(ConstructTMapInPlace(0, InCount))
{
}

void FPDItemNetDatum::PreReplicatedRemove(const FPDItemList& OwningList)
{
	check(OwningList.OwningInventory)
	OwningList.OwningInventory->OnDatumUpdated(this, EPDItemNetOperation::REMOVE);
}

void FPDItemNetDatum::PostReplicatedAdd(const FPDItemList& OwningList)
{
	check(OwningList.OwningInventory != nullptr)
	OwningList.OwningInventory->OnDatumUpdated(this, EPDItemNetOperation::ADD);
}

void FPDItemNetDatum::PostReplicatedChange(const FPDItemList& OwningList)
{
	check(OwningList.OwningInventory != nullptr)
	OwningList.OwningInventory->OnDatumUpdated(this, EPDItemNetOperation::CHANGE);
}

bool FPDItemList::NetSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FPDItemNetDatum, FPDItemList>(Items, DeltaParams, *this);
}

void FPDItemList::RemoveItem(FGameplayTag& ItemToRemove)
{
}

void FPDItemList::RemoveStack(FGameplayTag& ItemToRemove, int32 StackIdx)
{
}

void FPDItemList::AddItem(FGameplayTag& ItemToUpdate, int32 AmountToAdd)
{
	// @todo when adding an item, map it's tag to array IDX for fast editing of the array after, without having to search for it
	UPDInventorySubsystem* InvSubsystem = GEngine->GetEngineSubsystem<UPDInventorySubsystem>();
	check(InvSubsystem != nullptr); // Should never be nullptr

	if (InvSubsystem->TagToItemMap.Contains(ItemToUpdate) == false)
	{
		const FString BuildString = "FPDItemList::AddItem -- "
		+ FString::Printf(TEXT("\n Trying to add item with invalid tag (%s). No given ItemTables have this tag on any given entry "), *ItemToUpdate.GetTagName().ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);

		return;
	}
	
	// Check if item already exists, if true check its' last edited stack
	if (ItemToIndexMapping.Contains(ItemToUpdate))
	{
		int32 ItemsInLastEditedStack = 0;
		MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
		const FPDItemNetDatum& Item = Items[ItemToIndexMapping.FindRef(ItemToUpdate)]; // Stacks;
		if (Item.LastEditedStackIndex != INDEX_NONE)
		{
			ItemsInLastEditedStack = Item.Stacks[Item.LastEditedStackIndex];
		}

		if (ItemsInLastEditedStack == INDEX_NONE)
		{
			// @todo handle
		}

		
		const FPDItemDefaultDatum* Datum =InvSubsystem->TagToItemMap.FindRef(ItemToUpdate);
		const int32 Space = Datum->StackLimit - ItemsInLastEditedStack;
		const bool bFitsInCurrentStack = Space >= AmountToAdd;
		
		TMap<int32, int32>& CurrentStacks = Items[(ItemToIndexMapping.FindRef(ItemToUpdate))].Stacks;
		if (bFitsInCurrentStack)
		{
			MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
			const FPDItemNetDatum NetDatum{ItemToUpdate, Item.LastEditedStackIndex, AmountToAdd};
			*(CurrentStacks.Find(Item.LastEditedStackIndex)) += AmountToAdd;
		}
		else
		{
			// split
			AmountToAdd -= Space;

			MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
			const FPDItemNetDatum NetDatum{ItemToUpdate, Item.LastEditedStackIndex, AmountToAdd};

			*(CurrentStacks.Find(Item.LastEditedStackIndex)) += Space;
			CurrentStacks.Emplace(Item.LastEditedStackIndex + 1) = AmountToAdd;
		}
		
		
	}
	else
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
		const FPDItemNetDatum NetDatum{ItemToUpdate, 0, AmountToAdd};
		ItemToIndexMapping.Emplace(ItemToUpdate) = Items.Add(NetDatum);
	}

}

void FPDItemList::UpdateItem(FGameplayTag& ItemToRemove, int32 AmountToAdd)
{
	
}

void FPDItemList::UpdateStack(FGameplayTag& ItemToUpdate, int32 StackIdx, int32 AmountToAdd)
{
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
