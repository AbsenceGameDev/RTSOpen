/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 2
#include "Net/Serialization/FastArraySerializer.h"
#else
#include "Engine/NetSerialization.h"
#endif

#include "PDItemNetDatum.generated.h"

class UPDInventoryComponent;
struct FPDItemList;


/*
 * DAtum szie: 112 bytes
 * Extreme case, 1000 players + 1000 NPCs, all with with inventory
 * Average TXrate: 4 updates per second per inventory
 * Average Total TX per second; (448.000 * 2) 896,000 bytes per second, or ~900 kilobytes = inventory data per connection
 * For any given player this will consist of about 112 bytes out per second, and incoming packets consist of 99% of the load
 */
USTRUCT(BlueprintType)
struct FPDItemNetDatum : public FFastArraySerializerItem 
{
	GENERATED_BODY();

	// ConstructInPlace
	FPDItemNetDatum() : ItemTag(), LastEditedStackIndex(INDEX_NONE), Stacks() {};
	FPDItemNetDatum(const FGameplayTag& InItemTag, int32 StackIndex, int32 InCount);
	FPDItemNetDatum(const FGameplayTag& InItemTag, int32 InCount);

	void PreReplicatedRemove(const FPDItemList& OwningList);
	void PostReplicatedAdd(const FPDItemList& OwningList);
	void PostReplicatedChange(const FPDItemList& OwningList);
	
	UPROPERTY()
	FGameplayTag ItemTag{};

	// @todo Might need to replace this, tmap is too weighty, an array could do with some index caching
	UPROPERTY()
	int32 LastEditedStackIndex = INDEX_NONE;
	
	UPROPERTY()
	TMap<int32 /* StackIndex */, int32 /* ItemCount */> Stacks{};
};

USTRUCT(BlueprintType)
struct FPDItemList : public FFastArraySerializer 
{
	GENERATED_USTRUCT_BODY()

	bool NetSerialize(FNetDeltaSerializeInfo& DeltaParams);
	
	void RemoveItem(FGameplayTag& ItemToRemove);
	void RemoveStack(FGameplayTag& ItemToRemove, int32 StackIdx);
	void AddItem(FGameplayTag& ItemToUpdate, int32 AmountToAdd);

	void UpdateItem(FGameplayTag& ItemToUpdate, int32 AmountToAdd);
	void UpdateStack(FGameplayTag& ItemToUpdate, int32 StackIdx, int32 AmountToAdd);
	
	UPROPERTY()
	TArray<FPDItemNetDatum> Items;

	UPROPERTY()
	UPDInventoryComponent* OwningInventory = nullptr;
	
	TMap<FGameplayTag, int32> ItemToIndexMapping; // do not replicate, this is local
};


template<>
struct TStructOpsTypeTraits<FPDItemList> : public TStructOpsTypeTraitsBase2<FPDItemList>
{
	enum
	{
		WithNetDeltaSerialize = true,
	};
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
