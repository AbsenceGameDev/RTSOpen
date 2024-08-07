﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityTypes.h"
#include "NativeGameplayTags.h"
#include "Subsystems/EngineSubsystem.h"

#include "PDRTSCommon.generated.h"

class UNiagaraSystem;
class UNiagaraEmitter;
class UBehaviorTree;

DECLARE_LOG_CATEGORY_CLASS(PDLog_RTSBase, Log, All);

/** Declaring the "AI.Job" gameplay tag. to be defined in an object-file */
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Job_Idle);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Job_WalkToTarget);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Job_GenericInteract);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Job_GatherResource);

/** Declaring the "CTRL.Context." gameplay tags. to be defined in an object-file */
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_CTRL_Ctxt_BaseInput);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_CTRL_Ctxt_DragMove);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_CTRL_Ctxt_WorkerUnitMode);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_CTRL_Ctxt_BuildMode);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_CTRL_Ctxt_ConversationMode);

#define MASK_BYTE(Value, ByteIdx) (uint8)(((Value) >> (ByteIdx*8)) & 0xffff)
#define RESTORE_BYTE(Bytes, ByteIdx) ((((uint32)Bytes[ByteIdx]) << (ByteIdx*8)))

#define MOpaque(FClassToOpaque)\
	private:\
		FClassToOpaque() {}\
		~FClassToOpaque() {}\
\
		struct PImpl;\
		TOpaqueSelf<PImpl> Self;\
\
		friend TOpaqueImpl<FClassToOpaque>;\
	public:
	
#define MOpaquePImpl(FClassToOpaque)\
	struct FClassToOpaque::PImpl : TOpaqueImpl<FClassToOpaque>


/** @brief Opaque class idea */
template <typename TOpaqueClass>
class TOpaqueImpl : public TOpaqueClass
{
	static_assert(sizeof(TOpaqueClass) <= 1, "Opaque classes cannot have member variables nor virtual functions.");
private:
	friend typename TOpaqueClass::PImpl;
	TOpaqueImpl() {}
};

/** @brief Opaque class idea */
template <typename TPImpl>
struct TOpaqueSelf
{
	TPImpl*        Ptr()              { return (TPImpl*)this; }
	TPImpl const*  Ptr() const        { return (TPImpl*)this; }
	
	TPImpl&        operator*()        { return *Ptr(); }
	TPImpl const&  operator*() const  { return *Ptr(); }
	TPImpl*        operator->()       { return Ptr(); }
	TPImpl const*  operator->() const { return Ptr(); }
};


/** @brief Query groups */
enum EPDQueryGroups : int32
{
	INVALID_QUERY_GROUP = INDEX_NONE,
	QUERY_GROUP_MINIMAP = 1,
	QUERY_GROUP_HOVERSELECTION = 2,
	QUERY_GROUP_BUILDABLE_ACTORS = 3,
	QUERY_GROUP_4 = 4,
	QUERY_GROUP_X = 20,
};

/** @brief Selector for some standard types of animations, might need to expand */
UENUM()
enum class EPDVertexAnimSelector : uint8
{
	VertexAction    = 0 UMETA(DisplayName="Action"),
	VertexIdle      = 1 UMETA(DisplayName="Idle"),
	VertexSlowWalk  = 2 UMETA(DisplayName="SlowWalk"),
	VertexWalk      = 3 UMETA(DisplayName="Walk"),
	VertexJog       = 4 UMETA(DisplayName="Jog"),
	VertexSprint    = 5 UMETA(DisplayName="Sprint"),
};

/** @brief Build queue for builds without build-stages, used when there isn¨t enough resource to buy a buildable when placing it */
USTRUCT()
struct FPDOctreeSetting : public FTableRowBase
{
	GENERATED_BODY()
	
	/** @brief Uniform bounds for the requested octree */
	UPROPERTY(Config, EditAnywhere, Category="Octree")
	float OctreeUniformBounds = UE_OLD_WORLD_MAX;

	/** @brief Default cell size if no other sizes are set */
	UPROPERTY(Config, EditAnywhere, Category="Octree")
	float DefaultOctreeCellSize = 40.0;
};

/** @brief Subsystem (developer) settings, set the uniform bounds, the default grid cell size and the work tables for the entities to use */
UCLASS(Config = "Game", DefaultConfig)
class PDRTSBASE_API UPDOctreeSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPDOctreeSettings(){}

	/** @brief  Settings for the (buildable) actor octree */
	UPROPERTY(Config, EditAnywhere, Category="Octree")
	FPDOctreeSetting ActorOctreeSettings;
	/** @brief  Settings for the (unit) entity octree */
	UPROPERTY(Config, EditAnywhere, Category="Octree")
	FPDOctreeSetting EntityOctreeSettings;
};


/** @brief Subsystem (developer) settings, set the uniform bounds, the default grid cell size and the work tables for the entities to use */
UCLASS(Config = "Game", DefaultConfig)
class PDRTSBASE_API UPDRTSSubsystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPDRTSSubsystemSettings(){}

	/** @brief Work table soft objects */
	UPROPERTY(Config, EditAnywhere, Category = "Worker AI Subsystem", Meta = (RequiredAssetDataTags="RowStructure=/Script/PDRTSBase.PDWorkUnitDatum"))
	TArray<TSoftObjectPtr<UDataTable>> WorkTables;

	/** @brief Should we visualize a workers potential path? */
	UPROPERTY(Config, EditAnywhere, Category = "Worker AI Subsystem")
	bool bVisualizeWorkerPaths = true;
	
};


/** @brief Datatable row type. Defines A joh, what unit types can take said job and if it is shareable */
USTRUCT(BlueprintType, Blueprintable)
struct PDRTSBASE_API FPDWorkUnitDatum : public FTableRowBase
{
	GENERATED_BODY()

	FPDWorkUnitDatum() : bCanShareJob(0) {};

	/** @brief The tag for the actual job this entry defines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	FGameplayTag JobTag = FGameplayTag();

	/** @brief The tag for the required unit type(s) for this job, if empty all types are valid. Block all types if AI.Type.InvalidUnit is set */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	TArray<FGameplayTag> RequiredUnitTypeTags{};

	/** @brief If job can be shared between players */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	uint8 bCanShareJob : 1;
};


/** @brief Simple delegate deltatime param */
DECLARE_DELEGATE_OneParam(FPDTickBucket_SimpleDelegate, float);
/** @brief Dynamic delegate without parameters */
DECLARE_DYNAMIC_DELEGATE_OneParam(FPDTickBucket_DynamicDelegate, float, DeltaTime);
/** @brief Multicast delegate without parameters */
DECLARE_MULTICAST_DELEGATE_OneParam(FPDTickBucket_MulticastDelegate, float);
/** @brief Dynamic Multicast delegate without parameters */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPDTickBucket_DynMulticastDelegate, float, DeltaTime);

/** @brief Handles buckets of a given type with tick intervals, small time complexity, large spatial complexity
 *
 * @note
 * Whatever class TBucketType is, it will have to have these members to be able to compile:
 * - double UserInterval; // Example: set 50 for 50 second interval per tick
 * - void Execute(...); // An execution function
 * - double StoredFraction = INDEX_NONE; // Gets calculated in RegisterNewChunkForTicks
 */
template<typename TBucketType>
class TBucketTickHandler
{
public:

	MOpaque(TBucketTickHandler<TBucketType>)
	
	static TBucketTickHandler* CreateHandler();
	void DestroyHandler();

	/** @brief adds the bucket data ptr to TickEveryFrame */
	void AddToAllTickgroups(TBucketType* BucketData); // { TickEveryFrame.AddUnique(BucketData); }
	/** @brief removes the bucket data ptr from TickEveryFrame */
	void RemoveFromAllTickgroups(TBucketType* BucketData); // { TickEveryFrame.Remove(BucketData); }	
	/** @brief Registers the bucket in the tickgroups we resolve it's interval to  */
	void RegisterNewBucketForTicks(TBucketType* BucketData, double StartTime); // { HandleBucketFromTicks<false>(BucketData, StartTime); }
	/** @brief Registers the bucket from the tickgroups it exists in */
	void DeregisterBucketFromTicks(TBucketType* BucketData); // { HandleBucketFromTicks<true>(BucketData, 0.0); }
	
	/** @brief ticks our buckets
	 * O(1) == If all element are in a single bucket, we get this time complexity
	 * O(n / BucketCount) == If all buckets are the same size of elements, we get this time complexity
	 * O(n) == If all buckets are only 1 element size, we get this time complexity */
	void TickBuckets(float DeltaTime);
};

class DelegatedBucketTickHandlerBase
{
public:
	virtual ~DelegatedBucketTickHandlerBase() = default;

private:
	double UserInterval = 50.0; // Example: 50 second interval per tick
	virtual void Execute(float DeltaTime); // { Delegate.Execute(); }; 
	double StoredFraction = 0.0; // Gets calculated in RegisterNewChunkForTicks
	double LastTickOffset = 0.0; 
};
class Simple_BucketTickHandlerBase final : public DelegatedBucketTickHandlerBase
{
public:	
	FPDTickBucket_SimpleDelegate Delegate;
	virtual void Execute(float DeltaTime) override { Delegate.Execute(DeltaTime); } 
};
class Dynamic_BucketTickHandlerBase final : public DelegatedBucketTickHandlerBase
{
public:	
	FPDTickBucket_DynamicDelegate Delegate;
	virtual void Execute(float DeltaTime) override { Delegate.Execute(DeltaTime); } 
};
class Multicast_BucketTickHandlerBase final : public DelegatedBucketTickHandlerBase
{
public:	
	FPDTickBucket_MulticastDelegate Delegate;
	virtual void Execute(float DeltaTime) override { Delegate.Broadcast(DeltaTime); } 
};
class DynMulticast_BucketTickHandlerBase final : public DelegatedBucketTickHandlerBase
{
public:	
	FPDTickBucket_DynMulticastDelegate Delegate;
	virtual void Execute(float DeltaTime) override { Delegate.Broadcast(DeltaTime); } 
};

using SimpleDlgt_BucketTickHandler       = TBucketTickHandler<Simple_BucketTickHandlerBase>;
using DynDlgt_BucketTickHandler          = TBucketTickHandler<Dynamic_BucketTickHandlerBase>;
using MulticastDlgt_BucketTickHandler    = TBucketTickHandler<Multicast_BucketTickHandlerBase>;
using DynMulticastDlgt_BucketTickHandler = TBucketTickHandler<DynMulticast_BucketTickHandlerBase>;

inline SimpleDlgt_BucketTickHandler* SampleHandler = nullptr; // = SimpleDlgt_BucketTickHandler::CreateHandler();

//
// Legal according to ISO due to explicit template instantiation bypassing access rules: https://eel.is/c++draft/temp.friend

// Creates a static data member that will store a of type Tag::TType in which to store the address of a private member.
template <class Tag> struct TPrivateAccessor { static typename Tag::TType TypeValue; }; 
template <class Tag> typename Tag::TType TPrivateAccessor<Tag>::TypeValue;

// Generate a static data member whose constructor initializes TPrivateAccessor<Tag>::TypeValue.
// This type will only be named in an explicit instantiation, where it is legal to pass the address of a private member.
template <class Tag, typename Tag::TType TypeValue>
struct TTagPrivateMember
{
	TTagPrivateMember() { TPrivateAccessor<Tag>::TypeValue = TypeValue; }
	static TTagPrivateMember PrivateInstance;
};
template <class Tag, typename Tag::TType x> 
TTagPrivateMember<Tag,x> TTagPrivateMember<Tag,x>::PrivateInstance;

// A templated tag type for a given private member.  Each distinct private member you need to access should have its own tag.
// Each tag should contain a nested ::TType that is the corresponding pointer-to-member type.'
template<typename TAccessorType, typename TAccessorValue>
struct TAccessorTypeHandler { typedef TAccessorValue(TAccessorType::*TType); };

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
                      4. Must not be resold or repackaged or redistributed as another product, is only allowed to be used within a commercial or non-commercial game project.
                      5. Teams with yearly budgets larger than 100000 USD must contact the owner for a custom license or buy the framework from a marketplace it has been made available on.

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