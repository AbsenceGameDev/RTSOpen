/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PDBuildCommons.generated.h"

UENUM()
enum EPDItemGroup
{
	CRAFTABLE,
	RESOURCE,
	OTHER,
};

USTRUCT(BlueprintType, Blueprintable)
struct RTSOPEN_API FPDItemCosts 
{
	GENERATED_BODY();

	bool operator==(FPDItemCosts& Other) const { return InitialCost == Other.InitialCost && RecurringCost == Other.RecurringCost && bApplyRecurringAtFirst == Other.bApplyRecurringAtFirst; }
	bool operator!=(FPDItemCosts& Other) const { return (*this == Other) == false; }

	int32 ApplyInitalCost(int32 InTotal);
	int32 ApplyRecurringCost(int32 InTotal);

	/** @brief Costs that will be checked against and deducted only the first usage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 InitialCost = 0;

	/** @brief Costs that will be checked against and deducted each usage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RecurringCost = 0;

	/** @brief If true, will apply the recurring cost on the initial usage as-well */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 bApplyRecurringAtFirst = false;	
};

/** @brief
 *  @property CraftingCosts, If empty, then this is not a craftable
 *  @property UsageCosts, If this and CraftingCosts are empty, then the item is a resource */
USTRUCT(BlueprintType, Blueprintable)
struct FPDItemDatum : public FTableRowBase
{
	GENERATED_BODY()

	virtual void OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems) override;
	virtual void OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName) override;
	void Refresh(const UDataTable* InDataTable, const FName InRowName);

	/** @brief Type of resource, is not editable. Is deduced at row creation/edit.
	 * @note Deduction is as noted in the brief of the struct this property is contained within */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TEnumAsByte<EPDItemGroup> Type = EPDItemGroup::RESOURCE;
	
	/** @brief Tag associated with this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ItemTag = FGameplayTag::EmptyTag;

	/** @brief Item costs to use this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FPDItemCosts> UsageCosts; 

	/** @brief Item costs to craft/build this item */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FPDItemCosts> CraftingCosts;

	/** @brief The actor class to spawn */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> ActorClass;
};


USTRUCT()
struct FPDBuildState 
{
	GENERATED_BODY();

	
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