/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "PDInteractCommon.h"
#include "PDWorldManagementInterface.h"

#include "PDInteractInterface.generated.h"


/** @brief Boilerplate */
UINTERFACE(MinimalAPI) class UPDInteractInterface : public UInterface { GENERATED_BODY() };

/**
 * @brief This interface will be placed on actors we want to be able to interact with.
 */
class PDINTERACTION_API IPDInteractInterface : public IPDWorldManagementInterface
{
	GENERATED_BODY()

public:	
	/** @brief This function handles acknowledging and handling interactions. @return true|false based on if the interaction failed or succeeded */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Interact|Interface", Meta = (ExpandEnumAsExecs="InteractResult"))
	void OnInteract(FPDInteractionParams& InteractionParams, EPDInteractResult& InteractResult) const;
	virtual void OnInteract_Implementation(FPDInteractionParams& InteractionParams, EPDInteractResult& InteractResult) const;

	/** @brief This function handles returning a max interaction value. */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Interact|Interface")
	double GetMaxInteractionDistance() const;
	virtual double GetMaxInteractionDistance_Implementation() const;

	/** @brief This function handles returning a max interaction value. */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Interact|Interface")
	double GetCurrentUsability() const;
	virtual double GetCurrentUsability_Implementation() const;

	/* @brief usage will be game implementation specific but make sense for many different types of games with interactable to allow a tag to be returned, possibly related to AI jobs as in my case */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Interact|Interface")
	FGameplayTagContainer GetGenericTagContainer() const;
	virtual FGameplayTagContainer GetGenericTagContainer_Implementation() const;
	
	virtual void RegisterWorldInteractable_Implementation(UWorld* SelectedWorld, AActor* SelectedInteractable) override;

	bool bHasBeenRegisteredWithCurrentWorld = false;
};

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
