/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "PDInteractCommon.h"

#include "PDInteractInterface.generated.h"


/** @brief Boilerplate */
UINTERFACE(MinimalAPI) class UPDInteractInterface : public UInterface { GENERATED_BODY() };

/**
 * @brief This interface will be placed on actors we want to be able to interact with.
 */
class PDINTERACTION_API IPDInteractInterface
{
	GENERATED_BODY()

public:	
	/** @brief This function handles acknowledging and handling interactions. @return true|false based on if the interaction failed or succeeded */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Action|Interface", Meta = (ExpandEnumAsExecs="InteractResult"))
	void OnInteract(FPDInteractionParams& InteractionParams, EPDInteractResult& InteractResult) const;
	virtual void OnInteract_Implementation(FPDInteractionParams& InteractionParams, EPDInteractResult& InteractResult) const;

	/** @brief This function handles returning a max interaction value. */
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Action|Interface")
	double GetMaxInteractionDistance() const;
	virtual double GetMaxInteractionDistance_Implementation() const;

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
