﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NativeGameplayTags.h"
#include "Subsystems/EngineSubsystem.h"

#include "PDRTSCommon.generated.h"

class UNiagaraSystem;
class UNiagaraEmitter;
DECLARE_LOG_CATEGORY_CLASS(PDLog_RTSBase, Log, All);


class UBehaviorTree;

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

/** Declaring the "AI.Type." gameplay tags. to be defined in an object-file */
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Type_DefaultUnit);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Type_InvalidUnit);

PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Type_BuilderUnit_Novice);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Type_BuilderUnit_Intermeditate);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Type_BuilderUnit_Advanced);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_AI_Type_BuilderUnit_Expert);

/** Declaring the "BUILD.ContextMenu." gameplay tags. to be defined in an object-file */
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_BUILD_ContextMenu_DefaultWorker);

PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_BUILD_ContextMenu_Builder_Novice);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_BUILD_ContextMenu_Builder_Intermediate);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_BUILD_ContextMenu_Builder_Advanced);
PDRTSBASE_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_BUILD_ContextMenu_Builder_Expert);


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


//
// Ghosts
/** @brief Settings for a 'ghostable' actor */
USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDRTSBuildableCollisionSettings
{
	GENERATED_BODY()

	FPDRTSBuildableCollisionSettings() = default;
	explicit FPDRTSBuildableCollisionSettings(bool bInIsSet) : bIsSet(bInIsSet) {};
	
	/** @brief Is the actor collision enabled on the owning actor, for this context  */
	UPROPERTY()
	bool bIsActorCollisionEnabled = true;

	/** @brief Map of collision enable settings, keyed by (component) name  */
	UPROPERTY()
	TMap<FName, TEnumAsByte<ECollisionEnabled::Type>>  ComponentCollisionEnabledSettings;

	/** @brief  helps us control when we want to apply the collision settings above */
	bool bIsSet = false;
};

/** @brief Ghost transition enum state */
UENUM(Blueprintable)
enum class EPDRTSGhostTransition
{
	ESingleStage UMETA(DisplayName="SingleStage"),
	EMultipleStages UMETA(DisplayName="MultipleStages"),
};

/** @brief Ghost transition behaviour enum selector */
UENUM(Blueprintable)
enum class EPDRTSGhostStageBehaviour
{
	EOnStart UMETA(DisplayName="OnStart"),
	EOnEnd UMETA(DisplayName="OnEnd"),
};

/** @brief Data-assets for the visual effects of different build stages */
UCLASS(Blueprintable)
class UPDRTSGhostStageAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief Mesh that we should be apply this stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	UStaticMesh* StageGhostMesh = nullptr;

	/** @brief Effects that we should be apply this stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	UNiagaraSystem* StageGhostNiagaraSystem = nullptr;

	/** @brief Set if we should loop niagara effects until stage is finished */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	bool bLoopNiagara = false;		
};

/** @brief Per-Stage settings for a ghost */
USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDRTSGhostStageData
{
	GENERATED_BODY()

public:
	/** @brief The mesh behaviour we are expecting for this stage, (OnStartStage vs. OnEndStage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	EPDRTSGhostStageBehaviour StageMesh_ApplyBehaviour = EPDRTSGhostStageBehaviour::EOnStart;	
	/** @brief The vfx behaviour we are expecting for this stage, (OnStartStage vs. OnEndStage) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	EPDRTSGhostStageBehaviour StageVFX_ApplyBehaviour = EPDRTSGhostStageBehaviour::EOnStart;	
	
	/** @brief The data-asset for the current stage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	UPDRTSGhostStageAsset* StageDA = nullptr;

	/** @brief Potential max duration for a stage, this defines the space between the start stage effects and end stage effects
	 * @note If set to -1.0, this will not wait for end of stage. if set to 0.0, then this will rely on manual end of stage, this will in many cases be when a resource, or tag, quota has been met */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	double MaxDurationForStage = 0.0; 
};

/** @brief Stage(s) configuration for a ghost */
USTRUCT(Blueprintable)
struct FPDRTSGhostDatum : public FTableRowBase
{
	GENERATED_BODY()

	/** @rief Will be the only stage if TransitionStageType == EPDRTSGhostTransition::ESingleStage, can be left empty for no transition effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	EPDRTSGhostTransition TransitionStageType;

	/** @rief Will be the only stage if TransitionStageType == EPDRTSGhostTransition::ESingleStage, can be left empty for no transition effects */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings", Meta = (EditCondition = "TransitionStageType == EPDRTSGhostTransition::ESingleStage", EditConditionHides))
	FPDRTSGhostStageData SingleStageAsset;
	
	/** @rief Will be all stages (and in order) if TransitionStageType == EPDRTSGhostTransition::EMultipleStages, can be left empty for no transition effect */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor|Ghost|Settings", Meta = (EditCondition = "TransitionStageType == EPDRTSGhostTransition::EMultipleStages", EditConditionHides))
	TArray<FPDRTSGhostStageData> MultiStageStageAssets;	
};

/** @brief Ghost stat for a buildable, currenly only contains the ghost stage */
USTRUCT(Blueprintable)
struct FPDRTSGhostBuildState
{
	GENERATED_BODY()

	/** @rief Will not be used in the datatable, but each buildable with a ghost should have a 'FPDRTSGhostDatum' which reflects   */
	UPROPERTY(BlueprintReadWrite, Category = "Actor|Ghost|Settings")
	int32 CurrentStageIdx = 0;	
};


/** @brief Should Place Ghost or Buildable ? */
/** @brief If Ghost, should the build start immediately or should it have some processing duration? */
/** @brief ^ If immediately - Then, If no resources, put in waiting queue; when resources: start build  */
/** @brief ^ If duration    - Then, If no resources, put in waiting queue; when resources: send workers to gather to location  */
UENUM(Blueprintable)
enum class EPDRTSGhostSettings
{
	Ghost_FireThenForget, /** @brief Immediate Ghost;  If no resources, put in waiting queue; when resources: start build automatically without workers needing to gather any resourced there */
	Ghost_WaitThenFire,   /** @brief Processing: If no resources, put in waiting queue; when resources: send workers to gather to location */
	Buildable_FireThenForget, /** @brief Immediate Buildable;  If no resources, put in waiting queue; when resources: build immediately */
};

/** @brief If Buildable, should building be built immediately or after a timed delay? Costs should be taken from player bank or base? */
UCLASS(Config = "Game", DefaultConfig)
class PDRTSBASE_API UPDRTSBuildablePlacementBehaviour : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPDRTSBuildablePlacementBehaviour(){}

	/** @rief Tells us how we want the placement behaviour to be when placing a ghost  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPDRTSGhostSettings PlacementBehaviour = EPDRTSGhostSettings::Buildable_FireThenForget;
};

//
// Buildables
/** @rief Buildable data-asset, currently only keeps our widget related data in it  */
UCLASS(Blueprintable)
class PDRTSBASE_API UPDBuildableDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:

	/** @rief  Texture resource to apply on a buildable button (Does not take priority) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Buildable_Texture = nullptr;

	/** @rief  Material resource to apply on a buildable button (Takes priority) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* Buildable_MaterialInstance = nullptr;
	
	/** @rief  Audio resource to play when clicking a menu-button */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* Buildable_AudioLoop = nullptr;
};

/** @rief Buildable context data-asset, currently only keeps our widget related data in it  */
UCLASS(Blueprintable)
class PDRTSBASE_API UPDBuildContextDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	/** @rief  Texture resource to apply on a buildable button (Does not take priority) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* BuildContext_Texture = nullptr;

	/** @rief  Material resource to apply on a buildable button (Takes priority) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* BuildContext_MaterialInstance = nullptr;
};

/** @rief Main Buildable data, no tag associated  */
USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDBuildableData
{
	GENERATED_BODY()

	/** @brief Not needed, but use to display clearer name, with being set to nothing the name will be tha buildables tag converted to a name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	FText ReadableName{};

	/** @brief Actor class to spawn when placing the buildable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	TSubclassOf<AActor> ActorToSpawn{};
	
	/** @brief The buildables data-asset, if not spawned as ghost, or when transitioning from ghost to real building */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	UPDBuildableDataAsset* DABuildAsset = nullptr;

	/** @brief The buildables ghost configurations, keeps stage settings and such  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|BuildSystem")
	FPDRTSGhostDatum GhostData{};
};

/** @rief Actual Buildable row structure, associates tag and it's data with a buildable  */
USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDBuildable : public FTableRowBase
{
	GENERATED_BODY()

	FPDBuildable() = default;

	/** @brief The tag of this buildable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|BuildSystem")
	FGameplayTag BuildableTag{};	

	/** @brief The data of this buildable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|BuildSystem")
	FPDBuildableData BuildableData{};
};

/** @rief Main Build-context data, no tag associated, keeps a readable name and a data-asset  */
USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDBuildContextData
{
	GENERATED_BODY()

	/** @brief Not needed, but use to display clearer name, with being set to nothing the name will be tha buildables tag converted to a name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|BuildSystem")
	FText ReadableName{};
	
	/** @brief The build-context data-asset, only used for widget flair as of now  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|BuildSystem")
	UPDBuildContextDataAsset* DABuildContextAsset = nullptr;
};

/** @rief Actual Build-context row structure, associates tag and it's data with a buildable  */
USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDBuildContext : public FTableRowBase
{
	GENERATED_BODY()

	FPDBuildContext() = default;

	/** @brief The tags of this buildable context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	FGameplayTag ContextTag{};	

	/** @brief The readable name of this buildable context, todo, remove, already exists within 'ContextData'  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	FText ContextReadableName{};	
	
	/** @brief The resource data of buildables this context provides */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits", Meta = (RowType = "/Script/PDRTSBase.PDBuildable"))
	TArray<FDataTableRowHandle> BuildablesData{};

	/** @brief The resource data of this actual context */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	FPDBuildContextData ContextData{};
};

/** @rief Shared widget flair table row, for setting not specific to a given buildable or build-context but are instead applied as a whole  */
USTRUCT()
struct PDRTSBASE_API FPDSharedBuildWidgetFlair : public FTableRowBase
{
	GENERATED_BODY()

	/** @brief Tint when context is selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	FLinearColor SelectedContextTint = FLinearColor::Gray; 
	/** @brief Tint when context is not selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	FLinearColor NotSelectedContextTint = FLinearColor::Blue;
	
	/** @brief Tint when a buildable is selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	FLinearColor SelectedBuildableTint = FLinearColor::Gray; 
	/** @brief Tint when a buildable is not selected */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)	
	FLinearColor NotSelectedBuildableTint = FLinearColor::Blue;	
};

/** @rief Worker builder definition table row, Defines what entity types are granted which build-contexts  */
USTRUCT(Blueprintable)
struct PDRTSBASE_API FPDBuildWorker : public FTableRowBase
{
	GENERATED_BODY()

	FPDBuildWorker() = default;

	/** @brief The worker that this entry defines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RTSBase|WorkerUnits")
	FGameplayTag WorkerType{};
	
	/** @brief The buildable contexts granted to this worked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/PDRTSBase.PDBuildContext"))
	TArray<FDataTableRowHandle> GrantedContexts;	
};

/** @brief Intvector that we are using a floored and then truncated gridcell
 *  @note has some functions to check if a given cell is a neighbour in 2d, and/or in 3d, to convert to a floatvector alongside some basic operators overloads */
USTRUCT()
struct PDRTSBASE_API FPDGridCell : public FIntVector 
{
	GENERATED_BODY()
	
	/** @brief Converts cell to float-vector  */
	inline FVector ToFloatVector(const double ScalingFactor = 1.0) const
	{
		FVector RetVector;
		RetVector.X = this->X;
		RetVector.Y = this->Y;
		RetVector.Z = this->Z;
		return RetVector * ScalingFactor;
	}		

	/** @brief  Compares if cell is neighbour in X and Y */
	bool IsNeighbour2D(const FPDGridCell& OtherCell) const
	{
		const FPDGridCell Delta = OtherCell - *this; 
		return Delta.X >= -1 && Delta.X <= 1 && Delta.Y >= -1 && Delta.Y <= 1;
	}
	
	/** @brief  Compares if cell is neighbour in X, Y and Z */
	bool IsNeighbour3D(const FPDGridCell& OtherCell) const
	{
		const FPDGridCell Delta = OtherCell - *this; 
		return IsNeighbour2D(OtherCell) && Delta.Z >= -1 && Delta.Z <= 1 ;
	}

	/** @brief this->XYZ must be equal to Other.XYZ */
	bool operator==(const FPDGridCell & OtherCell) const
	{
		return this->X == OtherCell.X
		    && this->Y == OtherCell.Y
		    && this->Z == OtherCell.Z;
	}
	FPDGridCell& operator+=(const FPDGridCell& OtherCell) 
	{
		*this = *this + OtherCell;
		return *this;
	}
	FPDGridCell& operator-=(const FPDGridCell & OtherCell) 
	{
		*this = *this - OtherCell;
		return *this;
	}
	FPDGridCell operator+(const FPDGridCell & OtherCell) const
	{
		FPDGridCell ThisCopy = *this;
		ThisCopy.X += OtherCell.X;
		ThisCopy.Y += OtherCell.Y;
		ThisCopy.Z += OtherCell.Z;
		return ThisCopy;
	}
	FPDGridCell operator-(const FPDGridCell& OtherCell) const
	{
		FPDGridCell ThisCopy = *this;
		ThisCopy.X -= OtherCell.X;
		ThisCopy.Y -= OtherCell.Y;
		ThisCopy.Z -= OtherCell.Z;
		return ThisCopy;
	}
};


/** @brief Dynamic hash-grid developer settings, default to 200.0 cell size, modify in .ini config or in editor project settings */
UCLASS(Config = "Game", DefaultConfig)
class PDRTSBASE_API UPDHashGridDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPDHashGridDeveloperSettings(){}
	
	/** @brief Default grid data */
	UPROPERTY(Config, EditAnywhere, Category = "HashGrid")
	double UniformCellSize = 200.0f; 
	
};

/** @brief Dynamic hash-grid sub-system, Manages grid functionality.
 * @note Loads settings from UPDHashGridDeveloperSettings, and keeps it synced.
 * @note Has functions generate cell index constructs and apply them, transfer between vector and gridcell. Allows for easy snapping to grid  */
UCLASS()
class PDRTSBASE_API UPDHashGridSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** @brief Loads hashgrid config from developer settings and binds to said developer settings value changes */
	virtual void Initialize(FSubsystemCollectionBase& Collection) final override;

#if WITH_EDITOR
	/** @brief Updates 'UniformCellSize'. Function is bound to default UPDHashGridDeveloperSettings OnSettingChanged */
	void OnDeveloperSettingsChanged(UObject* SettingsToChange, const FPropertyChangedEvent& PropertyEvent);
#endif

	/** @brief  Floor vector and return as gridcell index */
    inline static struct FPDGridCell FloorVectorC(const FVector&& LocationToCell)
    {
        return FPDGridCell
        {
            {
				static_cast<int32_t>(FMath::Floor(LocationToCell.X)),
				static_cast<int32_t>(FMath::Floor(LocationToCell.Y)),
				static_cast<int32_t>(FMath::Floor(LocationToCell.Z))
            }
        };
    }

	/** @brief  Floor vector and return as FVector */
	inline static FVector FloorVectorV(const FVector&& LocationToCell)
    {
    	return FVector
		{
		    FMath::Floor(LocationToCell.X),
			FMath::Floor(LocationToCell.Y),
			FMath::Floor(LocationToCell.Z)
		};
    }	
  
	/** @brief  Calculate cell index */
    inline FPDGridCell GetCellIndex(const FVector& LocationToCell) const
    {
        return FloorVectorC(LocationToCell / UniformCellSize);
    }

	/** @brief  Calculate cell-clamped vector */
	inline FVector GetCellVector(const FVector& LocationToCell) const
    {
    	// Bring it back to proper world space dims by 
    	return FloorVectorV(LocationToCell / UniformCellSize) * UniformCellSize;
    }	

    /** @brief Slow if used often, instead cache a pointer to the subsystem and call it 'GetCellIndex' instead  */
	static inline FPDGridCell GetCellIndexStatic(const FVector& LocationToCell)
    {
    	const double CellSize = GEngine->GetEngineSubsystem<UPDHashGridSubsystem>()->UniformCellSize;
    	
    	return FloorVectorC(LocationToCell / CellSize);
    }

	/** @brief Slow if used often, instead cache a pointer to the subsystem and call it 'GetCellIndex' instead  */
	static inline FVector GetCellVectorStatic(const FVector& LocationToCell)
    {
    	const double CellSize = GEngine->GetEngineSubsystem<UPDHashGridSubsystem>()->UniformCellSize;
    	
    	return FloorVectorV(LocationToCell / CellSize) * CellSize; // Bring it back to proper world space dims
    }		

	/** @brief Uniform cell size instance data, value is dictated by the value in the developer settings */
	UPROPERTY()
    double UniformCellSize = 200.0f; 
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