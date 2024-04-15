﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#include "PDItemCommon.h"

int32 FPDItemCosts::ApplyInitialCost(int32 InTotal)
{
	return InTotal - (bApplyRecurringAtFirst ? InitialCost + RecurringCost : InitialCost);
}

int32 FPDItemCosts::ApplyRecurringCost(int32 InTotal)
{
	return InTotal - RecurringCost;
}

void FPDItemDefaultDatum::OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems)
{
	FTableRowBase::OnPostDataImport(InDataTable, InRowName, OutCollectedImportProblems);
	Refresh(InDataTable, InRowName);
}

void FPDItemDefaultDatum::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	FTableRowBase::OnDataTableChanged(InDataTable, InRowName);
	Refresh(InDataTable, InRowName);
}

void FPDItemDefaultDatum::Refresh(const UDataTable* InDataTable, const FName InRowName)
{
	bool bHadUsageCosts = false;
	for (const TPair<FGameplayTag, FPDItemCosts>& Usage : UsageCosts)
	{
		bHadUsageCosts = bHadUsageCosts || Usage.Key.IsValid();
	}

	bool bHadCraftingCosts = false;
	for (const TPair<FGameplayTag, FPDItemCosts>& Usage : CraftingCosts)
	{
		bHadCraftingCosts = bHadCraftingCosts || Usage.Key.IsValid();
	}

	TEnumAsByte<EPDItemGroup> OldType = Type;
	
	Type =
		bHadUsageCosts == false && bHadCraftingCosts == false ? EPDItemGroup::RESOURCE
		: bHadCraftingCosts ? EPDItemGroup::CRAFTABLE
		: EPDItemGroup::OTHER;

	if (OldType == Type) { return; }

	// needed to update 
	const_cast<UDataTable*>(InDataTable)->HandleDataTableChanged(InRowName);
	
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