/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#include "Inv/PDBuildCommons.h"

int32 FPDItemCosts::ApplyInitalCost(int32 InTotal)
{
	return InTotal - (bApplyRecurringAtFirst ? InitialCost + RecurringCost : InitialCost);
}

int32 FPDItemCosts::ApplyRecurringCost(int32 InTotal)
{
	return InTotal - RecurringCost;
}

void FPDItemDatum::OnPostDataImport(const UDataTable* InDataTable, const FName InRowName, TArray<FString>& OutCollectedImportProblems)
{
	FTableRowBase::OnPostDataImport(InDataTable, InRowName, OutCollectedImportProblems);
}

void FPDItemDatum::OnDataTableChanged(const UDataTable* InDataTable, const FName InRowName)
{
	FTableRowBase::OnDataTableChanged(InDataTable, InRowName);

	Refresh(InDataTable, InRowName);
}

void FPDItemDatum::Refresh(const UDataTable* InDataTable, const FName InRowName)
{
	bool bNoUsageCosts = true;
	for (const TPair<FGameplayTag, FPDItemCosts>& Usage : UsageCosts)
	{
		bNoUsageCosts = bNoUsageCosts || Usage.Key.IsValid();
	}

	bool bNoCraftingCosts = true;
	for (const TPair<FGameplayTag, FPDItemCosts>& Usage : CraftingCosts)
	{
		bNoCraftingCosts = bNoCraftingCosts || Usage.Key.IsValid();
	}

	Type =
		bNoUsageCosts && bNoCraftingCosts ? EPDItemGroup::RESOURCE
		: bNoCraftingCosts ? EPDItemGroup::OTHER
		: EPDItemGroup::CRAFTABLE;
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
