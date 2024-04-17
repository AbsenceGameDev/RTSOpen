/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

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
	INTERACT_SUCCESS   UMETA(DisplayName="Succeeded"),  /**< @brief Interaction attempt succeeded */
	INTERACT_FAIL      UMETA(DisplayName="Failed"),     /**< @brief Interaction attempt failed */ 
	INTERACT_DELAYED   UMETA(DisplayName="Delayed"),    /**< @brief Interaction attempt delayed. Possibly networked call */
	INTERACT_UNHANDLED UMETA(DisplayName="Unhandled"),    /**< @brief Interaction attempt delayed. Possibly networked call */
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
	explicit FPDInteractionParams(double _InteractionPercent, AActor* _InstigatorActor, TSubclassOf<UActorComponent> _InstigatorComponentClass, const TSet<FGameplayTag>& _OptionalInteractionTags)
		: InteractionPercent(_InteractionPercent), InstigatorActor(_InstigatorActor), InstigatorComponentClass(_InstigatorComponentClass),  OptionalInteractionTags(_OptionalInteractionTags) {}

	/** @brief Will contain a normalized value representing progress [0.0, 1.0]*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	double InteractionPercent = 0.0;
	
	/** @brief Passing in the instigator, for some interactables this will be needed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	AActor* InstigatorActor = nullptr;

	/** @brief Passing in the instigator, for some interactables this will be needed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	TSubclassOf<UActorComponent> InstigatorComponentClass = nullptr;
	
	/** @brief A set of optional tags to be handled as seen fit by game module implementations */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	TSet<FGameplayTag> OptionalInteractionTags{};
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FPDCustomProcessInteraction, const FPDInteractionParams&, InteractionParams, EPDInteractResult&, InteractResult);
USTRUCT(Blueprintable, BlueprintType)
struct FPDInteractionParamsWithCustomHandling : public FPDInteractionParams
{
	GENERATED_BODY()
	
	// @todo Handle callback on delayed events soon
	// DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPDCustomProcessInteractionDelayedCallback, const AActor*, OriginalInteractable, const FPDInteractionParams&, NewInteractionPayload, EPDInteractResult&, CallbackInteractResult);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Params")
	FPDCustomProcessInteraction CustomInteractionProcessor;
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


/**
Business Source License 1.1

Parameters

Licensor:             Ario Amin (@ Permafrost Development)
Licensed Work:        RTSOpen (Source available on github)
                      The Licensed Work is (c) 2024 Ario Amin (@ Permafrost Development)
Additional Use Grant: You may make free use of the Licensed Work in a commercial product or service provided these three additional conditions as met; 
                      1. Must give attributions to the original author of the Licensed Work, in 'Credits' if that is applicable.
                      2. The Licensed Work must be Compiled before being redistributed.
                      3. The Licensed Work Source may be linked but may not be packaged into the product or service being sold

                      "Credits" indicate a scrolling screen with attributions. This is usually in a products end-state

                      "Package" means the collection of files distributed by the Licensor, and derivatives of that collection
                      and/or of those files..   

                      "Source" form means the source code, documentation source, and configuration files for the Package, usually in human-readable format.

                      "Compiled" form means the compiled bytecode, object code, binary, or any other
                      form resulting from mechanical transformation or translation of the Source form.

Change Date:          2028-04-17

Change License:       Apache License, Version 2.0

For information about alternative licensing arrangements for the Software,
please visit: https://permadev.se/

Notice

The Business Source License (this document, or the “License”) is not an Open
Source license. However, the Licensed Work will eventually be made available
under an Open Source License, as stated in this License.

License text copyright (c) 2017 MariaDB Corporation Ab, All Rights Reserved.
“Business Source License” is a trademark of MariaDB Corporation Ab.

-----------------------------------------------------------------------------

Business Source License 1.1

Terms

The Licensor hereby grants you the right to copy, modify, create derivative
works, redistribute, and make non-production use of the Licensed Work. The
Licensor may make an Additional Use Grant, above, permitting limited
production use.

Effective on the Change Date, or the fourth anniversary of the first publicly
available distribution of a specific version of the Licensed Work under this
License, whichever comes first, the Licensor hereby grants you rights under
the terms of the Change License, and the rights granted in the paragraph
above terminate.

If your use of the Licensed Work does not comply with the requirements
currently in effect as described in this License, you must purchase a
commercial license from the Licensor, its affiliated entities, or authorized
resellers, or you must refrain from using the Licensed Work.

All copies of the original and modified Licensed Work, and derivative works
of the Licensed Work, are subject to this License. This License applies
separately for each version of the Licensed Work and the Change Date may vary
for each version of the Licensed Work released by Licensor.

You must conspicuously display this License on each original or modified copy
of the Licensed Work. If you receive the Licensed Work in original or
modified form from a third party, the terms and conditions set forth in this
License apply to your use of that work.

Any use of the Licensed Work in violation of this License will automatically
terminate your rights under this License for the current and all other
versions of the Licensed Work.

This License does not grant you any right in any trademark or logo of
Licensor or its affiliates (provided that you may use a trademark or logo of
Licensor as expressly required by this License).

TO THE EXTENT PERMITTED BY APPLICABLE LAW, THE LICENSED WORK IS PROVIDED ON
AN “AS IS” BASIS. LICENSOR HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS,
EXPRESS OR IMPLIED, INCLUDING (WITHOUT LIMITATION) WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT, AND
TITLE.

MariaDB hereby grants you permission to use this License’s text to license
your works, and to refer to it using the trademark “Business Source License”,
as long as you comply with the Covenants of Licensor below.

Covenants of Licensor

In consideration of the right to use this License’s text and the “Business
Source License” name and trademark, Licensor covenants to MariaDB, and to all
other recipients of the licensed work to be provided by Licensor:

1. To specify as the Change License the GPL Version 2.0 or any later version,
   or a license that is compatible with GPL Version 2.0 or a later version,
   where “compatible” means that software provided under the Change License can
   be included in a program with software provided under GPL Version 2.0 or a
   later version. Licensor may specify additional Change Licenses without
   limitation.

2. To either: (a) specify an additional grant of rights to use that does not
   impose any additional restriction on the right granted in this License, as
   the Additional Use Grant; or (b) insert the text “None”.

3. To specify a Change Date.

4. Not to modify this License in any other way.
 **/