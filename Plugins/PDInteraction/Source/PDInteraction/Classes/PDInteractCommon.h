/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Containers/Deque.h"
#include "PDInteractCommon.generated.h"

constexpr float DEFAULT_PEROBJECT_MAX_INTERACTION_DISTANCE = 150;
constexpr float DEFAULT_TRACER_MAX_INTERACTION_DISTANCE = 1500;

UENUM(Blueprintable, BlueprintType)
enum class EPDInteractResult : uint8
{
	INTERACT_SUCCESS UMETA(DisplayName="Succeeded"),  /**< @brief Interaction attempt succeeded */
	INTERACT_FAIL    UMETA(DisplayName="Failed"),     /**< @brief Interaction attempt failed */ 
	INTERACT_DELAYED UMETA(DisplayName="Delayed"),    /**< @brief Interaction attempt delayed. Possibly networked call */
};


UENUM(Blueprintable, BlueprintType)
enum class EPDTraceResult : uint8 
{
	TRACE_SUCCESS UMETA(DisplayName="Succeeded"),  /**< @brief Trace succeeded at finding an interactable */
	TRACE_FAIL    UMETA(DisplayName="Failed"),     /**< @brief Trace failed at finding an interactable  */ 
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
 * @brief This structure is meant to be placed in a interactable datatable or in an interactable baseactor
 */
USTRUCT(Blueprintable, BlueprintType) 
struct FPDInteractionSettings
{
	GENERATED_BODY()

	FPDInteractionSettings() = default;
	explicit FPDInteractionSettings(double _InteractionTimeInSeconds) : InteractionTimeInSeconds(_InteractionTimeInSeconds){}
	
	/** @brief Represents teh full time, in seconds, it takes to complete the interaction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Settings")
	double InteractionTimeInSeconds = 0.0;

	// @todo might need more settings
};

/**
 * @brief This structure encapsulated some settings related to interaction tracing
 */
USTRUCT(BlueprintType, Blueprintable)
struct FPDTraceSettings
{
	GENERATED_BODY()

public:
	void Setup();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_GameTraceChannel10;

	/** @brief Is generated from the collision channel using  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TEnumAsByte<EObjectTypeQuery> GeneratedObjectType = EObjectTypeQuery::ObjectTypeQuery_MAX; 	
};


USTRUCT(BlueprintType)
struct FPDTraceResult
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	EPDTraceResult ResultFlag = EPDTraceResult::TRACE_FAIL;

	UPROPERTY(BlueprintReadOnly)
	FHitResult HitResult;
	
	bool operator==(const FPDTraceResult& Other) const
	{
		return ResultFlag == Other.ResultFlag
		&& HitResult.GetActor() == Other.HitResult.GetActor()
		&& HitResult.Location.Equals(Other.HitResult.Location);
	}
};

constexpr int32 FRAMERESET_LIMIT = 10;

USTRUCT(BlueprintType)
struct FPDTraceBuffer
{
	GENERATED_USTRUCT_BODY()

public:
	TDeque<FPDTraceResult> Frames;
	int32 ValidFrameResetIt = 1;
	FPDTraceResult CachedValidFrame;

	void Setup();
	const FPDTraceResult& GetLastTraceResult() const;
	const FPDTraceResult& GetLastValidResult() const;
	const bool HasValidResults() const;
	void ClearTraceResults();
	void AddTraceFrame(EPDTraceResult TraceResult, const FHitResult& HitResult);
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