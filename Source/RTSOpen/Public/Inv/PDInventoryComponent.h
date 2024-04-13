/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "PDBuildCommons.h"
#include "PDItemNetDatum.h"
#include "Components/ActorComponent.h"
#include "PDInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPDOnItemUpdated, FPDItemNetDatum&, UpdatedDatum);

USTRUCT(Blueprintable)
struct FPDValueTracker
{
	GENERATED_BODY()

	/** @brief Current value */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Current = INDEX_NONE;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Old = INDEX_NONE;	
	
	/** @brief if INDEX_NONE (-1) or below, then there can exist an unlimited amount */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Max = INDEX_NONE;	
};

/**
 * @brief A simple but capable inventory component
 * @note Stateful networking + fastarrayserializers
 * 
 */
UCLASS(ClassGroup=(Custom), Meta=(BlueprintSpawnableComponent))
class UPDInventoryComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable)
	void RequestUpdateItem(TEnumAsByte<EPDItemNetOperation> RequestedOperation, FGameplayTag& ItemTag, int32 Count);
	
	void OnDatumUpdated(FPDItemNetDatum* ItemNetDatum, EPDItemNetOperation Operation);

	bool IsAtLastAvailableStack() const;
public:

	// @todo Move to settings table, keep here for now
	/** @brief Is in cubic centimetres, if INDEX_NONE (-1) or below, then the volume is unlimited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPDValueTracker Volume{};
	/** @brief Is in grams, if INDEX_NONE (-1) or below, then the weight is unlimited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPDValueTracker Weight{};
	/** @brief Is in total stacks the inventory systems has, if INDEX_NONE (-1) or below, then there can exist an unlimited amount of stacks */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPDValueTracker Stacks{};
	
	// Called on replicated item count update
	UPROPERTY(BlueprintAssignable)
	FPDOnItemUpdated OnItemUpdated;
	
	UPROPERTY(Replicated)
	FPDItemList ItemList;
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