/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#include "PDRTSBaseSubsystem.h"
#include "RTSBase/Classes/PDRTSCommon.h"


void UPDRTSBaseSubsystem::ProcessTables()
{
	ProcessFailCounter++;
	if (WorkTables.IsEmpty())
	{
		FString BuildString = "UPDRTSBaseSubsystem::ProcessTables -- "
		+ FString::Printf(TEXT("\n 'WorkTables' array is empty. Is not able to process data"));
		UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);
		
		return;
	}
	
	for (const UDataTable* Table : WorkTables)
	{
		if (Table == nullptr
			|| Table->IsValidLowLevelFast() == false
			|| Table->RowStruct != FPDWorkUnitDatum::StaticStruct())
		{
			continue;
		}

		TArray<FPDWorkUnitDatum*> Rows;
		Table->GetAllRows("", Rows);
		
		TArray<FName> RowNames = Table->GetRowNames();
		for (const FName& Name : RowNames)
		{
			const FPDWorkUnitDatum* DefaultDatum = Table->FindRow<FPDWorkUnitDatum>(Name,"");
			check(DefaultDatum != nullptr) // This should never be nullptr

			const FGameplayTag& JobTag = DefaultDatum->JobTag;

			if (JobTag.IsValid() == false)
			{
				FString BuildString = "UPDRTSBaseSubsystem::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add work/job on row (%s) Which does not have a valid gameplay tag. Skipping processing entry"), *Name.ToString());
				UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			
			// @note If duplicates, ignore duplicate and output errors to screen and to log
			if (TagToJobMap.Contains(JobTag))
			{
				const UDataTable* RetrievedTable = TagToTable.FindRef(JobTag);
				
				FString BuildString = "UPDInventorySubsystem::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add item(%s) which has already been added by previous table(%s)."),
						*JobTag.GetTagName().ToString(), RetrievedTable != nullptr ? *RetrievedTable->GetName() : *FString("INVALID TABLE"));
				UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			
			TagToJobMap.Emplace(DefaultDatum->JobTag) = DefaultDatum;
			NameToTagMap.Emplace(Name) = JobTag;
			TagToNameMap.Emplace(JobTag) = Name;
			TagToTable.Emplace(JobTag) = Table;
		}
	}
	ProcessFailCounter = 0;
	bHasProcessedTables = true;
}

const FPDWorkUnitDatum* UPDRTSBaseSubsystem::GetWorkEntry(const FGameplayTag& JobTag)
{
	if (bHasProcessedTables == false && ProcessFailCounter < 2) { ProcessTables(); }
	
	return TagToJobMap.Contains(JobTag) ? TagToJobMap.FindRef(JobTag) : nullptr;
}

const FPDWorkUnitDatum* UPDRTSBaseSubsystem::GetWorkEntry(const FName& JobRowName)
{
	if (bHasProcessedTables == false && ProcessFailCounter < 2) { ProcessTables(); }
	
	const FGameplayTag& JobTag = NameToTagMap.Contains(JobRowName) ? NameToTagMap.FindRef(JobRowName) : FGameplayTag::EmptyTag;
	return GetWorkEntry(JobTag);
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
