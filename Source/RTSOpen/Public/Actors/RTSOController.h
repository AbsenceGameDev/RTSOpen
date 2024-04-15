// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RTSOController.generated.h"

/**
 * 
 */
UCLASS()
class RTSOPEN_API ARTSOController : public APlayerController
{
	GENERATED_UCLASS_BODY()

	virtual void SetupInputComponent() override;
};
