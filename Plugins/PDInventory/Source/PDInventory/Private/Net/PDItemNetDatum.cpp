/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "Net/PDItemNetDatum.h"
#include "Components/PDInventoryComponent.h"

#include "PDItemCommon.h"
#include "PDInventorySubsystem.h"

static TMap<int32,int32> ConstructTMapInPlace(int32 StackIdx, int32 Count)
{
	TMap<int32,int32> ConstructedMap;
	ConstructedMap.Emplace(StackIdx) = Count;
	return ConstructedMap;
}

FPDItemNetDatum::FPDItemNetDatum(const FGameplayTag& InItemTag, int32 StackIndex, int32 InCount)
	: ItemTag(InItemTag), LastEditedStackIndex(StackIndex), TotalItemCount(InCount), Stacks(ConstructTMapInPlace(StackIndex, InCount))
{
}

FPDItemNetDatum::FPDItemNetDatum(const FGameplayTag& InItemTag, int32 InCount)
	: ItemTag(InItemTag), LastEditedStackIndex(0), TotalItemCount(InCount), Stacks(ConstructTMapInPlace(0, InCount))
{
}

void FPDItemNetDatum::PreReplicatedRemove(const FPDItemList& OwningList)
{
	check(OwningList.GetOwningInventory() != nullptr)
	OwningList.GetOwningInventory()->OnDatumUpdated(this, EPDItemNetOperation::REMOVEALL);
}

void FPDItemNetDatum::PostReplicatedAdd(const FPDItemList& OwningList)
{
	check(OwningList.GetOwningInventory() != nullptr)
	OwningList.GetOwningInventory()->OnDatumUpdated(this, EPDItemNetOperation::ADDNEW);
}

void FPDItemNetDatum::PostReplicatedChange(const FPDItemList& OwningList)
{
	check(OwningList.GetOwningInventory() != nullptr)
	OwningList.GetOwningInventory()->OnDatumUpdated(this, EPDItemNetOperation::CHANGE);
}

bool FPDItemList::NetSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FFastArraySerializer::FastArrayDeltaSerialize<FPDItemNetDatum, FPDItemList>(Items, DeltaParams, *this);
}

bool FPDItemList::RemoveAllItemsOfType(FGameplayTag& ItemToRemove)
{
	if (ItemToIndexMapping.Contains(ItemToRemove) == false) { return false; }

	MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
	FPDItemNetDatum& NetDatum = Items[ItemToIndexMapping.FindRef(ItemToRemove)];
	NetDatum.Stacks.Empty(); // clear data
	NetDatum.LastEditedStackIndex = 0;
	NetDatum.TotalItemCount = 0;
	
	return true;
}

bool FPDItemList::RemoveStack(FGameplayTag& ItemToRemove, int32 StackIdx)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
	FPDItemNetDatum& NetDatum = Items[ItemToIndexMapping.FindRef(ItemToRemove)];

	const int32 CopyOldValue = NetDatum.Stacks[StackIdx];
	NetDatum.Stacks[StackIdx] = 0; // clear data
	NetDatum.LastEditedStackIndex = StackIdx;
	NetDatum.TotalItemCount -= CopyOldValue;
	
	return true;
}

bool FPDItemList::UpdateItem(FGameplayTag& ItemToUpdate, int32 AmountToAdd)
{
	if (ItemToIndexMapping.Contains(ItemToUpdate))
	{
		const FPDItemNetDatum& Item = Items[ItemToIndexMapping.FindRef(ItemToUpdate)]; // Stacks;
		return UpdateItemAtStackIdx(ItemToUpdate, Item.LastEditedStackIndex, AmountToAdd);
	}

	return UpdateItemAtStackIdx(ItemToUpdate, INDEX_NONE, AmountToAdd);
}

bool FPDItemList::_Remove(FGameplayTag& ItemToUpdate, int32& AmountToAdd)
{
	FPDItemNetDatum& Item = Items[ItemToIndexMapping.FindRef(ItemToUpdate)]; // Stacks;
	// remove all items if that is the request
	if( Item.TotalItemCount <= FMath::Abs(AmountToAdd))
	{
		return RemoveAllItemsOfType(ItemToUpdate);
	}

	// Tally how many stack we otherwise might need to remove
	TArray<int32> StackIndicesToRemove{};
	for (const TPair<int32,int32>& Stack : Item.Stacks)
	{
		if (Stack.Value <= FMath::Abs(AmountToAdd))
		{
			AmountToAdd += Stack.Value; // AmountToAdd is negative at this point
			StackIndicesToRemove.Emplace(Stack.Key);
			continue;
		}
		Item.LastEditedStackIndex = Stack.Key;
		break;
	}

	// Removed tallied stacks
	for (int32 StackIdxToRemove : StackIndicesToRemove)
	{
		RemoveStack(ItemToUpdate, StackIdxToRemove);
	}
			
	MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
	int32& StackValue = Item.Stacks[Item.LastEditedStackIndex];
	if (Item.Stacks[Item.LastEditedStackIndex] > FMath::Abs(AmountToAdd))
	{
		StackValue += AmountToAdd;
	}
	return true;
}

bool FPDItemList::_Add(FGameplayTag& ItemToUpdate, int32 StackIdx, int32& AmountToAdd)
{
	MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
	FPDItemNetDatum& Item = Items[ItemToIndexMapping.FindRef(ItemToUpdate)]; // Stacks;
	const UPDInventorySubsystem* InvSubsystem = GEngine->GetEngineSubsystem<UPDInventorySubsystem>();
	check(InvSubsystem != nullptr); // Should never be nullptr
	
	int32 ItemsInLastEditedStack = 0;
	if (Item.Stacks.Contains(StackIdx))
	{
		ItemsInLastEditedStack = Item.Stacks[Item.LastEditedStackIndex];
	}
		
	const FPDItemDefaultDatum* Datum = InvSubsystem->TagToItemMap.FindRef(ItemToUpdate);
	const int32 Space = Datum->StackLimit - ItemsInLastEditedStack;
		
	TMap<int32, int32>& CurrentStacks = Item.Stacks;
	const bool bFitsInCurrentStack = Space >= AmountToAdd;
	if (bFitsInCurrentStack)
	{
		Item.TotalItemCount += AmountToAdd; 
		*(CurrentStacks.Find(StackIdx)) += AmountToAdd;
		if (Space == AmountToAdd)// next add should target a new stack
		{
			Item.LastEditedStackIndex++; // increment last edited stack and use it for the new key
			const FPDItemNetDatum NetDatum{ItemToUpdate, Item.LastEditedStackIndex, 0};
			ItemToIndexMapping.Emplace(ItemToUpdate) = Items.Add(NetDatum);
		}
			
		return true;
	}
	
	// split
	AmountToAdd -= Space;
	*(CurrentStacks.Find(StackIdx)) += Space;
	OwningInventory->Stacks.Current++;
			

	// Final added count in last index
	const int32 AppendedStackRemainder = AmountToAdd != Datum->StackLimit ? AmountToAdd % Datum->StackLimit : AmountToAdd;
	// find how many stacks we need to create
	const int32 AppendedStackCount = AmountToAdd / Datum->StackLimit;
	for (int32 Step = 0; Step < AppendedStackCount; Step++)
	{
		Item.LastEditedStackIndex++;
		CurrentStacks.Emplace(Item.LastEditedStackIndex) = AmountToAdd;
		OwningInventory->Stacks.Current++;
	}
	Item.LastEditedStackIndex++;
	CurrentStacks.Emplace(Item.LastEditedStackIndex) = AppendedStackRemainder;
	return true;
}

bool FPDItemList::UpdateItemAtStackIdx(FGameplayTag& ItemToUpdate, int32 StackIdx, int32 AmountToAdd)
{
	// @todo when adding an item, map it's tag to array IDX for fast editing of the array after, without having to search for it
	const UPDInventorySubsystem* InvSubsystem = GEngine->GetEngineSubsystem<UPDInventorySubsystem>();
	check(InvSubsystem != nullptr); // Should never be nullptr

	if (InvSubsystem->TagToItemMap.Contains(ItemToUpdate) == false)
	{
		const FString BuildString = "FPDItemList::AddItem -- "
		+ FString::Printf(TEXT("\n Trying to add item with invalid tag (%s). No given ItemTables have this tag on any given entry "), *ItemToUpdate.GetTagName().ToString());
		UE_LOG(LogTemp, Warning, TEXT("%s"), *BuildString);

		return false;
	}

	// Check if item already exists, if true check its' last edited stack
	if (ItemToIndexMapping.Contains(ItemToUpdate))
	{

		//
		// Removing or Adding items
		const bool bSubtraction = AmountToAdd < 0; // Want to remove items, possibly full stacks
		 return bSubtraction
			? _Remove(ItemToUpdate, AmountToAdd)
			: _Add(ItemToUpdate, StackIdx, AmountToAdd);
		
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(UPDInventoryComponent, ItemList, OwningInventory)
	const FPDItemNetDatum NetDatum{ItemToUpdate, 0, AmountToAdd};
	ItemToIndexMapping.Emplace(ItemToUpdate) = Items.Add(NetDatum);
	return true;	
}
// @todo when Item.LastEditedStackIndex reaches close to it's max representation, wrap it around and make sure there are no collisions on the way

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
