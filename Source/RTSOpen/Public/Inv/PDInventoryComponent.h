/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "PDBuildCommons.h"
#include "PDItemNetDatum.h"
#include "Components/ActorComponent.h"
#include "PDInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPDOnItemUpdated, FPDItemNetDatum&, UpdatedDatum);

/**
 * @brief A simple but capable inventory component
 * @note Stateful networking + fastarrayserializers
 * 
 */
UCLASS()
class UPDInventoryComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void BeginPlay() override;
public:
	UFUNCTION(BlueprintCallable)
	void RequestUpdateItem(TEnumAsByte<EPDItemNetOperation> RequestedOperation, FGameplayTag& ItemTag, int32 Count);
	
	void OnDatumUpdated(FPDItemNetDatum* ItemNetDatum, EPDItemNetOperation Operation);

	// @todo GOALS
	//
	// 1. Handle managing inventory data
	//
	// 2. Handle replication

	// Variables
public:

	// @todo Move to settings table, keep here for now
	/** @brief Is in cubic centimetres, if INDEX_NONE (-1) or below, then the volume is unlimited */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxVolume = INDEX_NONE;

	/** @brief Hidden from bp but cached and serialized if the option gets toggled on and off */
	UPROPERTY()
	int32 OldMaxVolume = INDEX_NONE;
	
	// Called on replicated talent level update
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