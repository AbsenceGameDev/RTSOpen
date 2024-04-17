// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Interactables/Buildings/RTSOInteractableBuildingBase.h"


ARTSOInteractableBuildingBase::ARTSOInteractableBuildingBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARTSOInteractableBuildingBase::BeginPlay()
{
	Super::BeginPlay();
}

FGameplayTagContainer ARTSOInteractableBuildingBase::GetGenericTagContainer_Implementation() const
{
	FGameplayTagContainer GeneratedTags;
	GeneratedTags.AddTag(JobTag);
	return GeneratedTags;
}

void ARTSOInteractableBuildingBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

