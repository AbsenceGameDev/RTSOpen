// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/PDInteractActor.h"
#include "RTSOInteractableResourceBase.generated.h"

UCLASS()
class RTSOPEN_API ARTSOInteractableResourceBase : public APDInteractActor
{
	GENERATED_BODY()

public:
	ARTSOInteractableResourceBase();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnInteract_Implementation(const FPDInteractionParamsWithCustomHandling& InteractionParams, EPDInteractResult& InteractResult) const override;
	virtual FGameplayTagContainer GetGenericTagContainer_Implementation() const override;
	
	virtual double GetInteractionTime_Implementation() const override { return HarvestTime; };

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	FGameplayTag JobTag{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess="true"))
	double HarvestTime = 0.0;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess="true"))
	double RefreshInterval = 20.0;
	double RefreshTickAcc = 0.0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (AllowPrivateAccess="true", RowType="/Script/PDInventory.PDItemDefaultDatum"))
	TMap<FGameplayTag /*resource/item tag*/, int32 /*count*/> LinkedItemResources;
};
