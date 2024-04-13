// Fill out your copyright notice in the Description page of Project Settings.


#include "Inv/PDInventorySubsystem.h"

#include "Inv/PDBuildCommons.h"

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
