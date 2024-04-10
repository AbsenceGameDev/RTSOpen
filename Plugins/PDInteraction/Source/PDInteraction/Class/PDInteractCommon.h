/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "PDInteractCommon.generated.h"

UENUM(Blueprintable, BlueprintType)
enum EPDInteractResult
{
	InteractSuccess UMETA(DisplayName="Succeeded"),  /**< @brief Interaction attempt succeeded */
	InteractFail UMETA(DisplayName="Failed"),        /**< @brief Interaction attempt failed */ 
	InteractDelayed UMETA(DisplayName="Delayed"),    /**< @brief Interaction attempt delayed. Possibly networked call */
};

/**
 * @brief This structure is constructed by the interaction component and passed into an 'interactable' item at the point of interaction
 */
USTRUCT(Blueprintable, BlueprintType)
struct FPDInteractionParams
{
	GENERATED_BODY()

	FPDInteractionParams() = default;
	explicit FPDInteractionParams(double _InteractionPercent, APlayerController* _Instigator, const TSet<FGameplayTag>& _OptionalInteractionTags)
		: InteractionPercent(_InteractionPercent), Instigator(_Instigator), OptionalInteractionTags(_OptionalInteractionTags) {}

	/** @brief Will contain a normalized value representing progress [0.0, 1.0]*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	double InteractionPercent = 0.0;
	
	/** @brief Passing in the instigator, for some interactables this will be needed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	APlayerController* Instigator = nullptr;
	
	/** @brief A set of optional tags to be handled as seen fit by game module implementations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	TSet<FGameplayTag> OptionalInteractionTags{};
};

/**
 * @brief This structure is constructed by the interaction component and passed into an 'interactable' item at the point of interaction
 */
USTRUCT(Blueprintable, BlueprintType) 
struct FPDInteractionSettings
{
	GENERATED_BODY()

	FPDInteractionSettings() = default;
	explicit FPDInteractionSettings(double _InteractionTimeInSeconds, ECollisionChannel _InteractionCollisionChannel)
		: InteractionTimeInSeconds(_InteractionTimeInSeconds), InteractionCollisionChannel(_InteractionCollisionChannel){}
	
	/** @brief Represents teh full time, in seconds, it takes to complete the interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	double InteractionTimeInSeconds = 0.0;
	
	/** @brief A set of optional tags to be handled as seen fit by game module implementations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	ECollisionChannel InteractionCollisionChannel{};
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