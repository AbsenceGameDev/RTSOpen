/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Containers/Deque.h"
#include "PDInteractCommon.generated.h"

constexpr float DEFAULT_PEROBJECT_MAX_INTERACTION_DISTANCE = 150;
constexpr float DEFAULT_TRACER_MAX_INTERACTION_DISTANCE = 1500;
constexpr float DEFAULT_TRACER_MAX_RADIAL_DISTANCE = 500;

constexpr ECollisionChannel DEDICATED_INTERACT_CHANNEL_ALT18 = ECollisionChannel::ECC_GameTraceChannel18;
constexpr ECollisionChannel DEDICATED_INTERACT_CHANNEL_ALT17 = ECollisionChannel::ECC_GameTraceChannel17;
constexpr ECollisionChannel DEDICATED_INTERACT_CHANNEL_ALT16 = ECollisionChannel::ECC_GameTraceChannel16;
constexpr ECollisionChannel DEDICATED_INTERACT_CHANNEL_ALT15 = ECollisionChannel::ECC_GameTraceChannel15;
constexpr ECollisionChannel DEDICATED_INTERACT_CHANNEL_ALT14 = ECollisionChannel::ECC_GameTraceChannel14;


UENUM(Blueprintable, BlueprintType)
enum class EPDTickTraceType : uint8
{
	TRACE_RADIAL     UMETA(DisplayName="Radial trace"),           /**< @brief Interaction attempt succeeded */
	TRACE_LINESHAPE  UMETA(DisplayName="Shape trace along line"), /**< @brief Interaction attempt failed */ 
	TRACE_MAX        UMETA(DisplayName="All trace types"),        /**< @brief Interaction attempt delayed. Possibly networked call */
};

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
struct FPDTraceTickSettings
{
	GENERATED_BODY()

public:
	
	/** @brief Which type of trace will we perform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPDTickTraceType TickTraceType = EPDTickTraceType::TRACE_RADIAL;

	/** @brief Needs to be large enough to hit objects of differing 'per-object' interaction distance limit*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTraceDistanceInUnrealUnits = DEFAULT_TRACER_MAX_RADIAL_DISTANCE; 	

	/** @brief Value is interpreted as per seconds. 0.0 will defaults to 'per-frame' for traces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double TickInterval  = 0.0;
	
};

/**
 * @brief This structure encapsulated some settings related to interaction tracing
 */
USTRUCT(BlueprintType, Blueprintable)
struct FPDTraceSettings : public FTableRowBase
{
	GENERATED_BODY()

public:
	void Setup();
	
	/** @brief Which type of trace will we perform */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPDTickTraceType TickTraceType = EPDTickTraceType::TRACE_RADIAL;

	/** @brief This contains settings for different trace types, as 'TickTraceType' can change we just change which settings we are reading from here */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<EPDTickTraceType, FPDTraceTickSettings> TickTraceTypeSettings;
	
	/** @brief THe collision channel we want to trace against */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = DEDICATED_INTERACT_CHANNEL_ALT18;

	/** @brief Is generated from the collision channel using 'UEngineTypes::ConvertToObjectType' */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	TEnumAsByte<EObjectTypeQuery> GeneratedObjectType = EObjectTypeQuery::ObjectTypeQuery_MAX;

	/** @brief Needs to be large enough to hit objects of differing 'per-object' interaction distance limit*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTraceDistanceInUnrealUnits = DEFAULT_TRACER_MAX_INTERACTION_DISTANCE; 

	/** @brief Needs to be large enough to hit objects of differing 'per-object' interaction distance limit*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxRadialTraceDistanceInUnrealUnits = DEFAULT_TRACER_MAX_RADIAL_DISTANCE; 	
	
	/** @brief Value 0.5 Defaults to 2 traces per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double RadialTraceTickInterval = 0.5; 

	/** @brief Value 0.0 Defaults to 'per-frame' traces */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double LineTraceTickInterval  = 0.0;
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
	UPROPERTY()
	TArray<AActor*> RadialTraceActors{};

	double RadialTraceTickTime = 0.0;
	double LineTraceTickTime = 0.0;

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


USTRUCT(BlueprintType, Blueprintable)
struct PDINTERACTION_API FRTSSavedInteractables
{
	GENERATED_BODY()

	UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadWrite)
	AActor* ActorInWorld = nullptr; // 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> ActorClass; // Tag of related item which inherits from the interact interface
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	double Usability = 0.0; // Could be interpreted as
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