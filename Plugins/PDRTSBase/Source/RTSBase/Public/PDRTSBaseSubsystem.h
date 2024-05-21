/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTypes.h"
#include "MassArchetypeTypes.h"
#include "AI/Mass/PDMassFragments.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "PDRTSBaseSubsystem.generated.h"

class UMassEntitySubsystem;

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

/** @brief Octree cell data */
struct PDRTSBASE_API FPDEntityOctreeCell
{
	/** @brief Tracked Entity */
	FMassEntityHandle EntityHandle;
	
	/** @brief Cell Bounds */
	FBoxCenterAndExtent Bounds;

	/** @brief Cell ID */
	TSharedPtr<FOctreeElementId2> SharedCellID;
};

/** @brief Boilerplate TOctree2 functional structure */
struct FPDEntityOctreeSemantics 
{
	/** @brief anonymous enum, won't collide. Tells the octree how many elements we can store per leaf node  */
	enum { MaxElementsPerLeaf = 128 };
	/** @brief anonymous enum, won't collide. Tells the octree our minimum limit for inclusive elements per node */
	enum { MinInclusiveElementsPerNode = 7 };
	/** @brief anonymous enum, won't collide. Tells the octree how deep our nodes can get */
	enum { MaxNodeDepth = 12 };

	/** @brief Leaf element allocator */
	typedef TInlineAllocator<MaxElementsPerLeaf> ElementAllocator;

	/** @brief Returns the bounding box of the give cell */
	FORCEINLINE static const FBoxCenterAndExtent& GetBoundingBox(const FPDEntityOctreeCell& Element)
	{
		return Element.Bounds;
	}

	/** @brief Octree Cell comparison */
	FORCEINLINE static bool AreElementsEqual(const FPDEntityOctreeCell& A, const FPDEntityOctreeCell& B)
	{
		return A.EntityHandle == B.EntityHandle;
	}

	/** @brief Octree Cell ID assignment */
	FORCEINLINE static void SetElementId(const FPDEntityOctreeCell& Element, FOctreeElementId2 Id)
	{
		*Element.SharedCellID = Id;
	};
};

/** @brief Boilerplate Entity namespace, defines an octree childclass. @todo write some code to scope lock the tree when doing certain function calls. Added come booleans as placeholders in code to remind me */
namespace PD::Mass::Entity
{
	typedef TOctree2<FPDEntityOctreeCell, FPDEntityOctreeSemantics> UnsafeOctree;
	
	/** @brief Safe octree implementation. Crudely avoids data-race conditions.
	 * @todo re-add the mutex when I have time to debug why it wasn't reliable last time. I have a hint I was locking the data only on one thread but right now this works fine  */
	class FPDSafeOctree : public UnsafeOctree
	{
	public:
		FPDSafeOctree()
			: UnsafeOctree() {}
		FPDSafeOctree(const FVector& InOrigin, FVector::FReal InExtent)
			: UnsafeOctree(InOrigin, InExtent) {}

		/** @brief Call this when you expect potential data race conditions and you want to prioritize one codepath over another. */
		void Lock() { bLockedReadInMainThread = true; };
		/** @brief Call this when you expect potential data race conditions and you want to prioritize one codepath over another. */
		void Unlock() { bLockedReadInMainThread = false; };
		/** @brief Call this when you expect potential data race conditions and you want to prioritize one codepath over another. */
		bool IsLocked() const { return bLockedReadInMainThread; };
		
		/** @brief Calls into the inner, the octree, as long as the read access is not locked */
		template<typename IterateBoundsFunc>
		inline void FindElementsWithBoundsTest(const FBoxCenterAndExtent& BoxBounds, const IterateBoundsFunc& Func, const bool bShouldLock = false)
		{
			if (IsLocked()) { return; }
			
			if (bShouldLock) { Lock();}
			
			// FScopeLock ReadLock(CriticalSectionMutex);
			UnsafeOctree::FindElementsWithBoundsTest(BoxBounds, Func);

			if (bShouldLock) { Unlock(); }						
		}

		/** @brief Calls into the inner, the octree, as long as the read access is not locked */
		template<typename IterateBoundsFunc>
		inline void FindFirstElementWithBoundsTest(const FBoxCenterAndExtent& BoxBounds, const IterateBoundsFunc& Func, const bool bShouldLock = false)
		{
			if (IsLocked()) { return; }
			
			if (bShouldLock) { Lock(); }
			
			// FScopeLock ReadLock(CriticalSectionMutex);
			bool ContinueTraversal = true;
			UnsafeOctree::FindFirstElementWithBoundsTest(BoxBounds, Func, ContinueTraversal);

			if (bShouldLock) { Unlock(); }			
		}
		// FCriticalSection* CriticalSectionMutex = nullptr;

	private:
		// @todo possibly keep track of how many threads have requested a lock on octree

		/** @brief Actual flag handled by the functions Lock(), Unlock(), IsLocked() */
		bool bLockedReadInMainThread = false;
	};

}

struct FPDWorkUnitDatum;

/** @brief Subsystem to handle octree size changes and to act as a manager for the entity workers */
UCLASS()
class PDRTSBASE_API UPDRTSBaseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:

	/** @brief Loads the worktables when the subsystem initializes during engine startup, far earlier than any world exists.
	 * It uses developer settings (UPDRTSSubsystemSettings) to read and write selected worktable paths via config, and or project settings window  
	 */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/** @brief Dispatch the octree generation on a latent action, via the worlds 'LatentActionManager' */
	UFUNCTION()
	virtual void DispatchOctreeGeneration();
	
	/** @brief Calls SetupOctreeWithNewWorld(TemporaryWorldCache) */
	UFUNCTION()
	virtual void SetupOctree();
	
	/** @brief
	 * - Registers world into 'WorldsWithOctrees' if not already registered.
	 * - Calls 'DispatchOctreeGeneration' is world is not valid or not initialized
	 * - Generates octree in and adds NewWorld to WorldsWithOctrees to track it
	 */
	UFUNCTION()
	virtual void SetupOctreeWithNewWorld(UWorld* NewWorld);
	
	/** @brief Bound to the given developer setting. Resolved the paths into actual WorkTables Array */
	void OnDeveloperSettingsChanged(UObject* SettingsToChange, FPropertyChangedEvent& PropertyEvent);

	/** @brief Processes the tables in 'WorkTables' and fills a number of maps for fast lookups downstream for entity jobs and such */
	UFUNCTION()
	virtual void ProcessTables();

	/** @brief Requests to generate a navpath for the selection group to the given target*/
	UFUNCTION()
	virtual void RequestNavpathGenerationForSelectionGroup(
		int32 OwnerID,
		int32 SelectionGroup,
		const FVector& SelectionCenter,
		const FPDTargetCompound& TargetCompound);
	
	/** @brief Returns the default work data via it's job-tag*/
	const FPDWorkUnitDatum* GetWorkEntry(const FGameplayTag& JobTag);
	/** @brief Returns the default work data via it's rowname in the table it was sourced from*/
	const FPDWorkUnitDatum* GetWorkEntry(const FName& JobRowName);
	
	/** @brief Associates and FMassArchetypeHandle with a config asset, so we can retrieve this info back to our save system fast when needed */
	void AssociateArchetypeWithConfigAsset(const FMassArchetypeHandle& Archetype, TSoftObjectPtr<UMassEntityConfigAsset> EntityConfig);

	/** @brief Retrieves the config asset */
	TSoftObjectPtr<UMassEntityConfigAsset> GetConfigAssetForArchetype(const FMassArchetypeHandle& Archetype);	
	
	/** @brief Does some portable iso-approved 'hacks' to fetch the all the mass ISM's */
	static const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& GetMassISMs(const UWorld* InWorld);

public:	
	/** @brief Work tables used by subsystem to organize ai-jobs */
	UPROPERTY()
	TArray<UDataTable*> WorkTables;

	/** @brief Selection group navpath map, Keyed by owner ID, valued by selection group navdata container*/
	UPROPERTY()
	TMap<int32 /*OwnerID*/, FPDWrappedSelectionGroupNavData> SelectionGroupNavData{};
	
	/** @brief Selection group Dirty data array holding a tuple, each tuple is keyed by the owner ID and has a value of the dirtied selection group index
	 *  @todo think on a solution which marks which actual data we want to update for the group, but this for now works as a solid enough optimization
	 */
	TArray<TTuple<int32 /*OwnerID*/, int32/*Player 'Selection' Index for non-hotkeyed groups*/> > DirtySharedData{};
	
	/** @brief Map for fast lookups. Keyed by job-tag, valued by default data entry */
	TMap<const FGameplayTag, const FPDWorkUnitDatum*> TagToJobMap{};
	/** @brief Map for fast lookups. Keyed by Rowname, valued by job-tag */
	TMap<const FName, FGameplayTag> NameToTagMap{};
	/** @brief Map for fast lookups. Keyed by Job-tag, valued by Rowname */
	TMap<const FGameplayTag, FName> TagToNameMap{};
	/** @brief Map for fast lookups. Keyed by Job-tag, valued by source datatable */
	TMap<const FGameplayTag, const UDataTable*> TagToTable{};

	/** @brief Flag that checks if we have processed the worktables */
	uint8 bHasProcessedTables = false;
	/** @brief Counter how many times the ProcessTables() function failed internally  */
	uint16 ProcessFailCounter = 0;

	/** @brief Reserved for later use */
	FStreamableManager DataStreamer;

	/** @brief Cached entity manager ptr*/
	FMassEntityManager* EntityManager = nullptr;
	/** @brief effectively unused. @todo revise if it still needed. any significant use of it has been removed since prototpying this  */
	UWorld* TemporaryWorldCache = nullptr;
	/** @brief The actual octree our worlds and our entities will make use of*/
	PD::Mass::Entity::FPDSafeOctree WorldOctree;

	/** @brief Tracking worlds that has been setup with this WorldOctree.  */
	TMap<void*, bool> WorldsWithOctrees{};
	
	TMap<FMassArchetypeHandle /*Archetype*/, TSoftObjectPtr<UMassEntityConfigAsset> /* EntityConfig */> ConfigAssociations{};
	
	/** @brief This will be useful on a server or shared screen environment */
	UPROPERTY(EditAnywhere)
	TMap<int32 /*OwnerID*/, AActor* /*OwningActor*/> SharedOwnerIDMappings;
	/** @brief This will be useful on a server or shared screen environment */
	UPROPERTY(EditAnywhere)
	TMap<AActor* /*OwningActor*/, int32 /*OwnerID*/> SharedOwnerIDBackMappings;	
};

/** @brief Subsystem (developer) settings, set the uniform bounds, the default grid cell size and the work tables for the entities to use */
UCLASS(Config = "Game", DefaultConfig)
class PDRTSBASE_API UPDRTSSubsystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPDRTSSubsystemSettings(){}

	/** @brief Uniform bounds for the requested octree */
	UPROPERTY(Config, EditAnywhere, Category="Visible")
	float OctreeUniformBounds = UE_OLD_WORLD_MAX;

	/** @brief */
	UPROPERTY(Config, EditAnywhere, Category="Visible")
	float DefaultCellSize = 40.0;

	/** @brief Work tables softobjects */
	UPROPERTY(Config, EditAnywhere, Category = "Worker AI Subsystem", Meta = (RequiredAssetDataTags="RowStructure=/Script/PDRTSBase.PDWorkUnitDatum"))
	TArray<TSoftObjectPtr<UDataTable>> WorkTables;
	
};

/** @brief Entity octree ID, used for observer tracking if IDs has been invalidated */
USTRUCT()
struct PDRTSBASE_API FPDOctreeFragment : public FMassFragment
{
	GENERATED_BODY()

	/** @brief CellID of a given octree cell using this fragment*/
	TSharedPtr<FOctreeElementId2> CellID;
};

/** @brief Entity octree grid cell tag */
USTRUCT(BlueprintType)
struct PDRTSBASE_API FPDInOctreeGridTag : public FMassTag
{
	GENERATED_BODY()
};

/** @brief Entity  query ('linetrace') tag. Used for entities we want to be processed with line intersection octree searches */
USTRUCT(BlueprintType)
struct PDRTSBASE_API FPDOctreeQueryTag : public FMassTag
{
	GENERATED_BODY()
};

namespace MassSample::Signals
{
	/** @brief Signal name for receiving a hit */
	static const FName OnReceiveHit = FName("OnReceiveHit");
	/** @brief Signal name for performing a hit */
	static const FName OnEntityHitOther = FName("OnEntityHitOther");
}



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