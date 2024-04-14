/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "PDInventorySubsystem.h"

#include "PDItemCommon.h"

void UPDInventorySubsystem::ProcessTables()
{
	for (const UDataTable* Table : ItemTables)
	{
		if (Table == nullptr
			|| Table->IsValidLowLevelFast() == false
			|| Table->RowStruct != FPDItemDefaultDatum::StaticStruct())
		{
			continue;
		}

		TArray<FPDItemDefaultDatum*> Rows;
		Table->GetAllRows("", Rows);
		
		TArray<FName> RowNames = Table->GetRowNames();
		for (const FName& Name : RowNames)
		{
			const FPDItemDefaultDatum* DefaultDatum = Table->FindRow<FPDItemDefaultDatum>(Name,"");
			check(DefaultDatum != nullptr) // This should never be nullptr

			const FGameplayTag& ItemTag = DefaultDatum->ItemTag;

			if (ItemTag.IsValid() == false)
			{
				FString BuildString = "UPDInventorySubsystem::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add item on row (%s) Which does not have a valid gameplay tag. Skipping processing entry"), *Name.ToString());
				UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			
			// @note If duplicates, ignore duplicate and output errors to screen and to log
			if (TagToItemMap.Contains(ItemTag))
			{
				const UDataTable* RetrievedTable = TagToTable.FindRef(ItemTag);
				
				FString BuildString = "UPDInventorySubsystem::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add item(%s) which has already been added by previous table(%s)."),
						*ItemTag.GetTagName().ToString(), RetrievedTable != nullptr ? *RetrievedTable->GetName() : *FString("INVALID TABLE"));
				UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			
			NameToItemMap.Emplace(Name) = DefaultDatum;
			TagToItemMap.Emplace(DefaultDatum->ItemTag) = DefaultDatum;
			NameToTagMap.Emplace(Name) = ItemTag;
			TagToNameMap.Emplace(ItemTag) = Name;
			TagToTable.Emplace(ItemTag) = Table;
		}
	}
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
