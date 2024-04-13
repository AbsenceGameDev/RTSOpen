// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/EngineSubsystem.h"
#include "PDInventorySubsystem.generated.h"

struct FPDItemDefaultDatum;
/**
 * 
 */
UCLASS()
class RTSOPEN_API UPDInventorySubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:	
	virtual void ProcessTables();


public:	
	UPROPERTY(EditAnywhere, Category = "Inventory Subsystem", Meta = (RequiredAssetDataTags="RowStructure=PDItemDefaultDatum"))
	TArray<UDataTable*> ItemTables;

	TMap<const FName, const FPDItemDefaultDatum*> NameToItemMap{};
	TMap<const FGameplayTag, const FPDItemDefaultDatum*> TagToItemMap{};
	TMap<const FName, FGameplayTag> NameToTagMap{};
	TMap<const FGameplayTag, FName> TagToNameMap{};
	TMap<const FGameplayTag, const UDataTable*> TagToTable{};
	
};
