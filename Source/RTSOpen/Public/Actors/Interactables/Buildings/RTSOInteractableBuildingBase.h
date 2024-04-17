// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/PDInteractActor.h"
#include "RTSOInteractableBuildingBase.generated.h"

UCLASS()
class RTSOPEN_API ARTSOInteractableBuildingBase : public APDInteractActor
{
	GENERATED_BODY()

public:
	ARTSOInteractableBuildingBase();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	

	virtual FGameplayTagContainer GetGenericTagContainer_Implementation() const override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	FGameplayTag JobTag{};	
};
