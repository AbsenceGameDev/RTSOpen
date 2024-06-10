/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTypes.h"
#include "MassArchetypeTypes.h"
#include "AI/Mass/PDMassFragments.h"
#include "Containers/Deque.h"
#include "Engine/StreamableManager.h"
#include "Subsystems/EngineSubsystem.h"
#include "PDRTSBaseSubsystem.generated.h"

class UPDRTSBaseUnit;
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

struct PDRTSBASE_API FPDActorOctreeCell
{
	/** @brief Tracked Entity */
	uint32 ActorInstanceID = 0;

	int32 OwnerID = INDEX_NONE;

	TDeque<FMassEntityHandle> IdleUnits; // if we add sharing units and such with teammates this will help
	
	/** @brief Cell Bounds */
	FBoxCenterAndExtent Bounds{};

	/** @brief Cell ID */
	TSharedPtr<FOctreeElementId2> SharedCellID;
};


/** @brief Boilerplate TOctree2 functional structure */
struct FPDActorOctreeSemantics 
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
	FORCEINLINE static const FBoxCenterAndExtent& GetBoundingBox(const FPDActorOctreeCell& Element)
	{
		return Element.Bounds;
	}

	/** @brief Octree Cell comparison */
	FORCEINLINE static bool AreElementsEqual(const FPDActorOctreeCell& A, const FPDActorOctreeCell& B)
	{
		return A.ActorInstanceID == B.ActorInstanceID;
	}
	

	/** @brief Octree Cell ID assignment */
	FORCEINLINE static void SetElementId(const FPDActorOctreeCell& Element, FOctreeElementId2 Id)
	{
		*Element.SharedCellID = Id;
	};
};

/** @brief Boilerplate Entity namespace, defines an octree sub-class. */
namespace PD::Mass
{
	namespace Entity
	{
		typedef TOctree2<FPDEntityOctreeCell, FPDEntityOctreeSemantics> UnsafeOctree;
		
		/** @note Safe octree implementation removed, avoided data-race conditions by designing access into the octree differently.   */
		class FPDEntityOctree : public UnsafeOctree
		{
		public:
			FPDEntityOctree()
				: UnsafeOctree() {}
			FPDEntityOctree(const FVector& InOrigin, FVector::FReal InExtent)
				: UnsafeOctree(InOrigin, InExtent) {}
	
		private:
		};
	}

	namespace Actor
	{
		typedef TOctree2<FPDActorOctreeCell, FPDActorOctreeSemantics> UnsafeActorOctree;
		class FPDActorOctree : public UnsafeActorOctree
		{
		public:
			FPDActorOctree()
				: UnsafeActorOctree() {}
			FPDActorOctree(const FVector& InOrigin, FVector::FReal InExtent)
				: UnsafeActorOctree(InOrigin, InExtent) {}
	
		private:
		};	
	}	

}

struct FPDWorkUnitDatum;

USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDEntityPingDatum
{
	GENERATED_BODY()

	FPDEntityPingDatum() : FPDEntityPingDatum(0) {};
private:
	FPDEntityPingDatum(uint32 InInstanceID);
public:
	FPDEntityPingDatum(AActor* InWorldActor, const FGameplayTag& InJobTag);
	FPDEntityPingDatum(AActor* InWorldActor, const FGameplayTag& InJobTag, int32 InAltOwnerID, double InInterval = 10.0, int32 InMaxCountIntervals = 15);
	~FPDEntityPingDatum();

	TArray<uint8> ToBytes() const;
	void FromBytes(const TArray<uint8>&& Bytes);

	void Unsafe_SetOwnerIDFromOwner();

	bool operator==(const FPDEntityPingDatum& OtherDatum) const { return this->InstanceID == OtherDatum.InstanceID; }
	
	/** @brief  Instance ID,only visible in instance, get from world actor*/
	uint32 InstanceID = INDEX_NONE;
	
	/** @brief The world actor this ping is calling entities for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Procedures")
	AActor* WorldActor = nullptr;
	
	/** @brief  job tag associated with this ping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Procedures")
	FGameplayTag JobTag{};
	
	/** @brief  Owner ID, error if neither supplied and can't be found on WorldActors owner controller*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Procedures")
	int32 OwnerID = INDEX_NONE;

	/** @brief default ping at 10 seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Procedures")
	double Interval = 10.0;

	/** @brief default ping at 10 seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Procedures")
	int32 MaxCountIntervals = 15;	

	friend class UPDEntityPinger;
};

/** @brief Hash the ping data, really just pass through the world-actors UID*/
inline uint32 GetTypeHash(const FPDEntityPingDatum& PingDatum)
{
	if (PingDatum.InstanceID == 0)
	{
		uint32 Hash = 0;
		Hash = HashCombine(Hash, GetTypeHash(PingDatum.JobTag));
		Hash = HashCombine(Hash, GetTypeHash(PingDatum.WorldActor));
		return Hash;
	}
	
	return PingDatum.InstanceID;
}

UCLASS()
class PDRTSBASE_API UPDEntityPinger : public UEngineSubsystem
{
	GENERATED_BODY()
	
	UPDEntityPinger() = default;
	explicit UPDEntityPinger(const FPDEntityPingDatum& PingDatum);
public:	

	UFUNCTION(BlueprintCallable)
	TArray<uint8> AddPingDatum(const FPDEntityPingDatum& PingDatum);
	UFUNCTION(BlueprintCallable)
	void RemovePingDatum(const FPDEntityPingDatum& PingDatum);
	UFUNCTION(BlueprintCallable)
	void RemovePingDatumWithHash(const FPDEntityPingDatum& PingDatum);
	UFUNCTION(BlueprintCallable)
	FTimerHandle GetPingHandleCopy(const TArray<uint8>& PingHashBytes);
	FTimerHandle GetPingHandleCopy(uint32 PingHash);
	
	UFUNCTION(BlueprintCallable)
	TArray<uint8> EnablePing(const FPDEntityPingDatum& PingDatum);
	UFUNCTION(BlueprintCallable)
	static TArray<uint8> EnablePingStatic(const FPDEntityPingDatum& PingDatum);
	UFUNCTION(BlueprintNativeEvent)
	void Ping(UWorld* World, const FPDEntityPingDatum& PingDatum);
	UFUNCTION(BlueprintCallable)
	void DisablePing(const FPDEntityPingDatum& PingDatum);
	UFUNCTION(BlueprintCallable)
	static void DisablePingStatic(const FPDEntityPingDatum& PingDatum);

	/** @brief The pings we are managing, at 10k actors to ping we'll still only take up */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Procedures")
	TMap<FPDEntityPingDatum, FTimerHandle> PingDataAndHandles{};
};

USTRUCT()
struct FLEntityCompound
{
	GENERATED_BODY()

	bool operator==(const FLEntityCompound& Other) const { return this->EntityHandle == Other.EntityHandle; }
	
	UPROPERTY()
	FMassEntityHandle EntityHandle;
	UPROPERTY()
	FVector Location;
	UPROPERTY()
	int32 OwnerID;
};

USTRUCT()
struct FPDRTSBuildQueue
{
	GENERATED_BODY()
	
	TDeque<AActor*> ActorInWorld;
};


/** @brief Subsystem to handle octree size changes and to act as a manager for the entity workers */
UCLASS()
class PDRTSBASE_API UPDRTSBaseSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
public:
	void ProcessBuildContextTable(const TSoftObjectPtr<UDataTable>& TablePath);
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

	/** @brief Returns the default build context data via it's BuildContext-tag*/
	const FPDBuildContext* GetBuildContextEntry(const FGameplayTag& BuildContextTag);
	/** @brief Returns the default Buildable data via it's Buildable-tag*/
	const FPDBuildableData* GetBuildableData(const FGameplayTag& BuildableTag);
	const FGameplayTag& GetBuildableTagFromData(const FPDBuildableData* BuildableData);
	
	
	void QueueRemoveFromWorldBuildTree(int32 UID);
	void PassOverDataFromQueue();

	static TArray<FMassEntityHandle> FindIdleEntitiesOfType(TArray<FGameplayTag> EligibleEntityTypes, const AActor* ActorToBuild, int32 OwnerID);

	static UPDRTSBaseSubsystem* Get();


	static void ProcessGhostStageDataAsset(const AActor* GhostActor, bool bIsStartOfStage, const FPDRTSGhostStageData& SelectedStageData);

	UFUNCTION(BlueprintCallable, Category = "Actor|Ghost")
	static void ProcessGhostStage(AActor* GhostActor, const FGameplayTag& BuildableTag, FPDRTSGhostTransitionState& MutableGhostDatum, bool bIsStartOfStage);

	UFUNCTION(BlueprintCallable, Category = "Actor|Ghost")
	static double GetMaxDurationGhostStage(const FGameplayTag& BuildableTag, FPDRTSGhostTransitionState& MutableGhostDatum);	

	UFUNCTION(BlueprintCallable, Category = "Actor|Ghost")
	static bool IsPastFinalIndex(const FGameplayTag& BuildableTag, FPDRTSGhostTransitionState& MutableGhostDatum);	


	/** @brief Associates and FMassArchetypeHandle with a config asset, so we can retrieve this info back to our save system fast when needed */
	void AssociateArchetypeWithConfigAsset(const FMassArchetypeHandle& Archetype, const TSoftObjectPtr<UMassEntityConfigAsset>& EntityConfig);

	/** @brief Retrieves the config asset */
	TSoftObjectPtr<UMassEntityConfigAsset> GetConfigAssetForArchetype(const FMassArchetypeHandle& Archetype);

	void WorldInit(const UWorld* World);

	/** @brief Does some portable iso-approved 'hacks' to fetch the all the mass ISM's */
	static const TArray<TObjectPtr<UInstancedStaticMeshComponent>>& GetMassISMs(const UWorld* InWorld);


public:	
	/** @brief Work tables used by subsystem to organize ai-jobs */
	UPROPERTY()
	TArray<UDataTable*> WorkTables{};

	/** @brief Build context tables held by subsystem for others to fetch */
	UPROPERTY()
	TArray<UDataTable*> BuildContextTables{};

	/** @brief Build queues per player */
	UPROPERTY()
	TMap<int32 /*Player ID*/, FPDRTSBuildQueue> BuildQueues{};	

	TMap<FGameplayTag, const FPDBuildWorker*> GrantedBuildContexts_WorkerTag{};		
	TMap<FGameplayTag, const FPDBuildContext*> BuildContexts_WTag{};	
	TMap<FGameplayTag, const FPDBuildableData*> BuildableData_WTag{};	
	TMap<FPDBuildableData*, FGameplayTag>       BuildableData_WTagReverse{};	
	
	/** @brief Selection group navpath map, Keyed by owner ID, valued by selection group navdata container*/
	UPROPERTY()
	TMap<int32 /*OwnerID*/, FPDWrappedSelectionGroupNavData> SelectionGroupNavData{};
	
	/** @brief Selection group Dirty data array holding a tuple, each tuple is keyed by the owner ID and has a value of the dirtied selection group index
	 *  @todo think on a solution which marks which actual data we want to update for the group, but this for now works as a solid enough optimization
	 */
	TArray<TTuple<int32 /*OwnerID*/, int32/*Player 'Selection-group' Index */> > DirtySharedData{};
	
	/** @brief Map for fast lookups. Keyed by job-tag, valued by default data entry */
	TMap<const FGameplayTag, const FPDWorkUnitDatum*> TagToJobMap{};
	/** @brief Map for fast lookups. Keyed by Rowname, valued by job-tag */
	TMap<const FName, FGameplayTag> NameToTagMap{};
	/** @brief Map for fast lookups. Keyed by Job-tag, valued by Rowname */
	TMap<const FGameplayTag, FName> TagToNameMap{};
	/** @brief Map for fast lookups. Keyed by Job-tag, valued by source datatable */
	TMap<const FGameplayTag, const UDataTable*> TagToTable{};

	TMap<UWorld*, UPDRTSBaseUnit*> WorldToEntityHandler{};

	/** @brief Flag that checks if we have processed the worktables */
	uint8 bHasProcessedTables = false;
	/** @brief Counter how many times the ProcessTables() function failed internally  */
	uint16 ProcessFailCounter = 0;

	/** @brief Reserved for later use */
	FStreamableManager DataStreamer;

	/** @brief Cached entity manager ptr*/
	const FMassEntityManager* EntityManager = nullptr;
	/** @brief effectively unused. @todo revise if it still needed. any significant use of it has been removed since prototyping this  */
	UWorld* TemporaryWorldCache = nullptr;

	/** @brief The actual octree our entities will make use of*/
	PD::Mass::Entity::FPDEntityOctree WorldEntityOctree;
	/** @brief The actual octree our buildable actors will make use of*/
	PD::Mass::Actor::FPDActorOctree WorldBuildActorOctree;
	TMap<int32 /*UID*/, TSharedPtr<FOctreeElementId2>> ActorsToCells;

	bool bIsProcessingBuildableRemovalQueue = false;
	TDeque<int32 /*UID*/> RemoveBuildableQueue_FirstBuffer{};  // First  buffer, tells our processor twhihc UIDS to flush and remove their cells
	TDeque<int32 /*UID*/> RemoveBuildableQueue_SecondBuffer{}; // Second buffer, while processing first buffer, use this so we don't get race conditions

	// @todo move into developer settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitPingLimitBuilding = 100; // max 100 eligible units may be pinged


	// Move to real struct perhaps
	struct FPDActorCompound
	{
		int32 OwnerID = INDEX_NONE;
		TArray<AActor*>* WorldActorsPtr;
		bool operator==(const UPDRTSBaseSubsystem::FPDActorCompound& Other) const
		{
			return
				this->WorldActorsPtr    != nullptr
				&& Other.WorldActorsPtr != nullptr
				&& this->OwnerID == Other.OwnerID;
		}
	};

	bool bCanEditActorArray = true;
	TArray<FPDActorCompound> WorldBuildActorArrays{};
	TArray<FVector> WorldBuildableLocationList{};
	TMap<int32, int32> IndexToID{};
	TMap<int32, int32> IDToIndex{};
	TDeque<int32> QueuedRemovals_BuildablesArrays{};
	TDeque<TTuple<int32, void*>> QueuedAdditions_BuildablesArrayPointers{};

	TArray<FPDActorCompound>& PrepareMutateBuildableTrackingData()
	{
		bCanEditActorArray = false;
		bIsProcessingBuildableRemovalQueue = true;
		return WorldBuildActorArrays;
	};	

	void EndMutateBuildableTrackingData()
	{
		bCanEditActorArray = true;

		RemoveBuildableQueue_FirstBuffer.Empty();
		bIsProcessingBuildableRemovalQueue = false;
		PassOverDataFromQueue();

		// Processing FILO deque
		ProcessQueuedAdditionsAndRemovals_BuildablesArrayTracker();
		return;
	};	

	void ProcessQueuedAdditionsAndRemovals_BuildablesArrayTracker()
	{
		for (int32 Step = QueuedAdditions_BuildablesArrayPointers.Num(); Step > 0;)
		{
			const TTuple<int32, void*>& Top = QueuedAdditions_BuildablesArrayPointers.Last();
			const int32 QueuedOwnerID = Top.Get<0>();
			TArray<AActor*>* QueuedArrayPtr = static_cast<TArray<AActor*>*>(Top.Get<1>());

			AddBuildActorArray(QueuedOwnerID,  QueuedArrayPtr);
			QueuedAdditions_BuildablesArrayPointers.PopLast();
			Step--;
		}

		for (int32 Step = QueuedRemovals_BuildablesArrays.Num(); Step > 0;)
		{
			const int32 QueuedOwnerID = QueuedRemovals_BuildablesArrays.Last();
			RemoveBuildActorArray(QueuedOwnerID);
			QueuedRemovals_BuildablesArrays.PopLast();
			Step--;
		}		
	};	
	
	void AddBuildActorArray(int32 OwnerID, TArray<AActor*>* OwnerArrayPtr)
	{
		if (bCanEditActorArray == false)
		{
			QueuedAdditions_BuildablesArrayPointers.EmplaceFirst(TTuple<int32, void*>{OwnerID, OwnerArrayPtr});
			return;
		}
			
		int32 Idx = WorldBuildActorArrays.AddUnique(FPDActorCompound{OwnerID, OwnerArrayPtr});
		IndexToID.Emplace(Idx, OwnerID);
		IDToIndex.Emplace(OwnerID, Idx);
		
		// // Iterate WorldBuildActorArrays and put it's entries into WorldBuildableLocationList
		// WorldBuildableLocationList.EmplaceAt(Idx);
	};

	void AddBuildActorArray(FPDActorCompound ActorCompound)
	{
		if (bCanEditActorArray == false)
		{
			QueuedAdditions_BuildablesArrayPointers.EmplaceFirst(TTuple<int32, void*>(ActorCompound.OwnerID, ActorCompound.WorldActorsPtr));
			return;
		}

		int32 Idx = WorldBuildActorArrays.AddUnique(ActorCompound);
		IndexToID.Emplace(Idx, ActorCompound.OwnerID);
		IDToIndex.Emplace(ActorCompound.OwnerID, Idx);

		// // Iterate WorldBuildActorArrays and put it's entries into WorldBuildableLocationList
		// WorldBuildableLocationList.EmplaceAt(Idx);
	};	

	void RemoveBuildActorArray(int32 OwnerID)
	{
		if (bCanEditActorArray == false)
		{
			QueuedRemovals_BuildablesArrays.EmplaceFirst(OwnerID);
			return;
		}
		
		if (IDToIndex.Contains(OwnerID) == false) { return; } 
		
		WorldBuildActorArrays.RemoveAt(IDToIndex.FindRef(OwnerID));
	};
	
	
	/** @brief Tracking worlds that has been setup with this WorldOctree.  */
	TMap<void*, bool> WorldsWithOctrees{};
	
	TMap<FMassArchetypeHandle /*Archetype*/, TSoftObjectPtr<UMassEntityConfigAsset> /* EntityConfig */> ConfigAssociations{};
	
	/** @brief This will be useful on a server or shared screen environment */
	UPROPERTY(EditAnywhere)
	TMap<int32 /*OwnerID*/, AActor* /*OwningActor*/> SharedOwnerIDMappings;
	/** @brief This will be useful on a server or shared screen environment */
	UPROPERTY(EditAnywhere)
	TMap<AActor* /*OwningActor*/, int32 /*OwnerID*/> SharedOwnerIDBackMappings;


	enum EPDQueryGroups : int32
	{
		INVALID_QUERY_GROUP = INDEX_NONE,
		QUERY_GROUP_MINIMAP = 1,
		QUERY_GROUP_HOVERSELECTION = 2,
		QUERY_GROUP_BUILDABLE_ACTORS = 3,
		QUERY_GROUP_4 = 4,
		QUERY_GROUP_X = 20,
	};
	
	template<typename TDataType>
	struct TPDQueryBase 
	{
		int32 Shape = 0; // 0= Box, 1 Sphere, 2 Ellipsoid? etc..
		UE::Math::TVector<TDataType> Location = UE::Math::TVector<TDataType>::ZeroVector;
	};	
	
	using FPDOctreeUserQuery = struct QPDUserQuery_t
	{
		QPDUserQuery_t() = default;
		explicit QPDUserQuery_t(AActor* CallingActor) : CallingUser(CallingActor) { }
		
		struct QBox    : public TPDQueryBase<double> { UE::Math::TVector<double> QuerySizes = UE::Math::TVector<double>::ZeroVector; };
		struct QSphere : public TPDQueryBase<double> { double QueryRadius = 0.0; };

		// @note Alignment is same and they share the same initial memory layout,
		// @note I'll get away with accessing the inner shape from either or to then resolve the rest to access proper memory space
		union QShapeSelector
		{
			QShapeSelector() { AsBox = QBox{}; };
			
			QBox AsBox;
			QSphere AsSphere;
			void Set(const QBox& InAsBox)
			{
				AsBox = InAsBox;
				AsBox.Shape = 0;
			};
			void Set(const QSphere& InAsSphere)
			{
				AsSphere = InAsSphere;
				AsSphere.Shape = 1;
			};
			
			TPDQueryBase<double>* Get()
			{
				void* This = this;
				TPDQueryBase<double>* PtrHack = static_cast<TPDQueryBase<double>*>(This);
				return PtrHack;
			};
			
		};
		

		static QBox MakeUserQuery(const UE::Math::TVector<double>& Location = UE::Math::TVector<double>::ZeroVector, const FVector& QuerySizes = UE::Math::TVector<double>::OneVector)
		{
			return FPDOctreeUserQuery::QBox{0,Location, QuerySizes};
		}

		static QSphere MakeUserQuery(const UE::Math::TVector<double>& Location = UE::Math::TVector<double>::ZeroVector, const double Radius = 0.0)
		{
			return FPDOctreeUserQuery::QSphere{1,Location, Radius};
		}

		static bool IsPointWithinQuery(const UE::Math::TVector<double>& Point, const QBox& Box)
		{
			const UE::Math::TBox<double> InnerBox{{Box.Location - Box.QuerySizes},{Box.Location + Box.QuerySizes}};
			return IsPointWithinQuery(Point, InnerBox);
		}
		static bool IsPointWithinQuery(const UE::Math::TVector<double>& Point, const QSphere& Sphere)
		{
			const UE::Math::TSphere<double> InnerSphere{Sphere.Location, Sphere.QueryRadius};
			return IsPointWithinQuery(Point, InnerSphere);
		}			
		
		static bool IsPointWithinQuery(const UE::Math::TVector<double>& Point, const UE::Math::TBox<double>& Box)
		{
			return Box.IsInsideOrOn(Point);
		}
		static bool IsPointWithinQuery(const UE::Math::TVector<double>& Point, const UE::Math::TSphere<double>& Sphere)
		{
			return Sphere.IsInside(Point);
		}

		/** Assigns box data */
		void CreateNewQueryEntry(const int32 Key, const QBox& BoxData)
		{
			QShapeSelector InData;
			InData.Set(BoxData);

			QueryArchetypes.FindOrAdd(Key, InData);
			CurrentBuffer.FindOrAdd(Key);
		}

		void CreateNewQueryEntry(const int32 Key, const QSphere& SphereData)
		{
			QShapeSelector InData;
			InData.Set(SphereData);
			
			QueryArchetypes.FindOrAdd(Key, InData);
			CurrentBuffer.FindOrAdd(Key);
		}

		void UpdateQueryPosition(const int32 Key, const FVector& NewPos)
		{
			QueryArchetypes.FindOrAdd(Key).Get()->Location = NewPos;
		}
		
		void UpdateQueryOverlapBuffer(const int32 Key, const FVector& ComparePos, const FMassEntityHandle OptionalEntityHandle, const int32 OptionalID)
		{
			bool bIsPointWithinQuery = false;

			QShapeSelector ObjectAsBase = QueryArchetypes.FindOrAdd(Key);
			TPDQueryBase<double>* BasePtr = ObjectAsBase.Get();
			switch (ObjectAsBase.Get()->Shape)
			{
			case 0: // Box
				bIsPointWithinQuery = IsPointWithinQuery(ComparePos, *static_cast<QBox*>(BasePtr));
				break;
			case 1: // Sphere
				bIsPointWithinQuery = IsPointWithinQuery(ComparePos, *static_cast<QSphere*>(BasePtr));
				break;
			default:
				break;
			}

			
			if (bIsPointWithinQuery)
			{
				FLEntityCompound EntityCompound{OptionalEntityHandle, ComparePos, OptionalID};
				CurrentBuffer.FindOrAdd(Key).Emplace(EntityCompound);
			}

			// Clear key if it still persists for some reason, this fricking data-race condition is fixed now but damn it is not pretty
			else if (CurrentBuffer.Contains(Key))
			{
				TArray<FLEntityCompound>& EntityArray = *CurrentBuffer.Find(Key);
				const FLEntityCompound EntityCompound{OptionalEntityHandle, ComparePos, OptionalID};
				EntityArray.Remove(EntityCompound);
			}
		}
		
		void RemoveQueryData(const int32 Key)
		{
			QueryArchetypes.Remove(Key);
			CurrentBuffer.Remove(Key);
		}

		void ClearQueryDataAndSettings(int32 Key)
		{

			QShapeSelector ObjectAsBase = QueryArchetypes.FindOrAdd(Key);
			TPDQueryBase<double>* BasePtr = ObjectAsBase.Get();
			switch (ObjectAsBase.Get()->Shape)
			{
			case 0: // Box
				{
					QBox& QueryEntry = *static_cast<QBox*>(BasePtr);
					QueryEntry.Location  = FVector::ZeroVector;
					QueryEntry.QuerySizes = FVector::OneVector;
				}
				break;
			case 1: // Sphere
				{
					QSphere& QueryEntry = *static_cast<QSphere*>(BasePtr);
					QueryEntry.Location    = FVector::ZeroVector;
					QueryEntry.QueryRadius = 0.0;
				}
				break;
			default:
				break;
			}

			

			ClearQueryBuffer(Key);
		}

		void ClearQueryBuffer(int32 Key)
		{
			// CurrentBuffer.FindOrAdd(Key).Empty();
			if (CurrentBuffer.Contains(Key))
			{
				CurrentBuffer.FindRef(Key).Empty();
			}
			else
			{
				CurrentBuffer.Add(Key).Empty();
			}			
		}		
		
		TMap<int32, QShapeSelector> QueryArchetypes;
		TMap<int32, TArray<FLEntityCompound>> CurrentBuffer;

		AActor* CallingUser = nullptr;
	};

	FPDOctreeUserQuery OctreeUserQuery{};
	FPDOctreeUserQuery OctreeBuildSystemEntityQuery{}; // @todo
};

UENUM()
enum class EPDRTSBuildProgressionBehaviour
{
	EBuildWaitsForWorkers,
	EBuildCallsForWorkers,
};

UENUM()
enum class EPDRTSBuildCostBehaviour
{
	EBuildCostPlayerBank,
	EBuildCostBaseBank, // may differ from player bank
	EBuildCostWaitForWorkers,
};

USTRUCT(Blueprintable)
struct FPDRTSBuildSystemBehaviours
{
	GENERATED_BODY()

	UPROPERTY(Config, EditAnywhere, Category="Visible")
	EPDRTSBuildProgressionBehaviour Progression = EPDRTSBuildProgressionBehaviour::EBuildCallsForWorkers;

	UPROPERTY(Config, EditAnywhere, Category="Visible")
	EPDRTSBuildCostBehaviour Cost = EPDRTSBuildCostBehaviour::EBuildCostWaitForWorkers;
};

/** @brief Subsystem (developer) settings, set the uniform bounds, the default grid cell size and the work tables for the entities to use */
UCLASS(Config = "Game", DefaultConfig)
class PDRTSBASE_API UPDRTSSubsystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPDRTSSubsystemSettings(){}

	/** @brief Uniform bounds for the requested octree */
	UPROPERTY(Config, EditAnywhere, Category="RTS Octree")
	float OctreeUniformBounds = UE_OLD_WORLD_MAX;

	/** @brief */
	UPROPERTY(Config, EditAnywhere, Category="Visible")
	float DefaultOctreeCellSize = 40.0;

	/** @brief Work table soft objects */
	UPROPERTY(Config, EditAnywhere, Category = "Worker AI Subsystem", Meta = (RequiredAssetDataTags="RowStructure=/Script/PDRTSBase.PDWorkUnitDatum"))
	TArray<TSoftObjectPtr<UDataTable>> WorkTables;

	/** @brief Build Contexts (categories) table soft objects */
	UPROPERTY(Config, EditAnywhere, Category = "Worker AI Subsystem", Meta = (RequiredAssetDataTags="RowStructure=/Script/PDRTSBase.PDBuildContext"))
	TArray<TSoftObjectPtr<UDataTable>> BuildContextTables;	

	/** @brief Build Workers (Worker types and their granted contexts) table soft objects */
	UPROPERTY(Config, EditAnywhere, Category = "Worker AI Subsystem", Meta = (RequiredAssetDataTags="RowStructure=/Script/PDRTSBase.PDBuildWorker"))
	TArray<TSoftObjectPtr<UDataTable>> BuildWorkerTables;

	/** @brief Build Workers (Worker types and their granted contexts) table soft objects */
	UPROPERTY(Config, EditAnywhere, Category="Worker AI Subsystem|Builder|Behaviour")
	FPDRTSBuildSystemBehaviours DefaultBuildSystemBehaviours{};
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

/** @brief Entity  query ('line-trace') tag. Used for entities we want to be processed with line intersection octree searches */
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