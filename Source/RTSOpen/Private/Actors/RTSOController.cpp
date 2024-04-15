// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RTSOController.h"

ARTSOController::ARTSOController(const FObjectInitializer& ObjectInitializer)
{
	bShowMouseCursor = true;
	bEnableMouseOverEvents = true;
}

void ARTSOController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// InputComponent->BindAction(); // Bind touch detection, dispatch delegate to notify input mode typ has changed to Gamepad
	// InputComponent->BindAction(); // Bind touch detection, dispatch delegate to notify input mode typ has changed to touchscreen 
	// InputComponent->BindAxis(); // Bind MouseMove, dispatch delegate to notify input mode typ has changed to keyboard + mouse
}
