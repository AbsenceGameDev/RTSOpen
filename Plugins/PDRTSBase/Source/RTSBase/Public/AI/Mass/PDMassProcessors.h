/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#pragma once

#include "CoreMinimal.h"
#include "MassLODCollectorProcessor.h"
#include "MassObserverProcessor.h"
#include "MassProcessor.h"
#include "MassRepresentationProcessor.h"
#include "MassVisualizationLODProcessor.h"
#include "MassSignalProcessorBase.h"
#include "PDMassProcessors.generated.h"

class UMassCrowdRepresentationSubsystem;
struct FMassMoveTargetFragment;
struct FPDMFragment_EntityAnimation;
class UMassSignalSubsystem;
class UNavigationSystemV1;
class UMassEntitySubsystem;

#define DECLARE_PROCESSOR_BODY \
virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override; \
virtual void ConfigureQueries() override; \
virtual void Initialize(UObject& Owner) override;

#define DEFINE_PROCESSOR_EMPTY_DEFAULTS \
virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override {}; \
virtual void ConfigureQueries() override {}; \
virtual void Initialize(UObject& Owner) override {};

/**
 * @brief Moves the entities around to a given target, tasks have the responsibility to create actual multi-point paths
 */
UCLASS()
class PDRTSBASE_API UPDProcessor_MoveTarget : public UMassProcessor
{
	GENERATED_BODY()

public:
	UPDProcessor_MoveTarget();

	DECLARE_PROCESSOR_BODY

private:
	FMassEntityQuery EntityQuery;
	TObjectPtr<UMassSignalSubsystem> SignalSubsystem;
};

/**
 * @brief Initializes RTS Entities, currently only sets up possibly shared animation data
 */
UCLASS()
class PDRTSBASE_API UPDMProcessor_InitializeEntities : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UPDMProcessor_InitializeEntities();
	
	DECLARE_PROCESSOR_BODY

protected:
	FMassEntityQuery EntityQuery;
};

/**
 * @brief Setup ISMs, animation, and textures for RTS Entities
 */
UCLASS()
class UPDMProcessor_EntityAnimations : public UMassProcessor
{
	GENERATED_BODY()

public:
	UPDMProcessor_EntityAnimations();

	DECLARE_PROCESSOR_BODY
	
	void UpdateISMVertexAnimation(FMassInstancedStaticMeshInfo& ISMInfo, FPDMFragment_EntityAnimation& AnimationData,
								  float LODSignificance, float PrevLODSignificance, int32 NumFloatsToPad) const;

protected:
	FMassEntityQuery EntityQuery;
	TObjectPtr<UMassCrowdRepresentationSubsystem> RepresentationSubsystem;
};

UCLASS() class PDRTSBASE_API UPDMProcessor_Visualization : public UMassVisualizationProcessor
{ public: GENERATED_BODY() UPDMProcessor_Visualization() { bAutoRegisterWithProcessingPhases = true; } };

/*
 * Created a crowd version for parallelization of the crowd with the traffic
 */
UCLASS(Meta=(DisplayName="(Permadev) Crowd visualization"))
class PDRTSBASE_API UPDMProcessor_LODVisualization : public UMassVisualizationLODProcessor
{
	GENERATED_BODY()
public:
	UPDMProcessor_LODVisualization();

protected:
	DECLARE_PROCESSOR_BODY

	// void DispatchCollisionOverwrite();

	// UFUNCTION()
	// virtual void OverrideISMOwnerCollision();

protected:
	bool bHasUpdatedMassISMSettings = false;
};


UCLASS(Meta = (DisplayName = "Crowd LOD Collection"))
class PDRTSBASE_API UPDMProcessor_LODCollector : public UMassLODCollectorProcessor
{
	GENERATED_BODY()

	UPDMProcessor_LODCollector() { bAutoRegisterWithProcessingPhases = true; };

protected:
	virtual void ConfigureQueries() override;
};


//
// Oct tree entity collision and hash grid processing

UCLASS()
class PDRTSBASE_API UPDOctreeProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	UPDOctreeProcessor();
	void DebugDrawCells();

protected:	
	DECLARE_PROCESSOR_BODY

public:	
	FMassEntityQuery AddToHashGridQuery;
	FMassEntityQuery UpdateOctreeElementsQuery;
	FMassEntityQuery RemoveFromGridEntityQuery;
	
	UPROPERTY()
	class UPDRTSBaseSubsystem* RTSSubsystem = nullptr;

	static TAutoConsoleVariable<bool> CVarDrawCells;

	bool bSentChaosCommand = false;
};

UCLASS()
class PDRTSBASE_API UPDOctreeEntityObserver : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	UPDOctreeEntityObserver();

protected:	
	DECLARE_PROCESSOR_BODY

public:	
	FMassEntityQuery EntityQuery;

	UPROPERTY()
	class UPDRTSBaseSubsystem* RTSSubsystem = nullptr;

	UPROPERTY()
	float ElementGridSize = 0.0; //assigned to by default settings
};


UCLASS()
class PDRTSBASE_API UPDGridCellDeinitObserver : public UMassObserverProcessor
{
	GENERATED_BODY()

public:
	
	UPDGridCellDeinitObserver();

protected:	
	DECLARE_PROCESSOR_BODY

public:	
	FMassEntityQuery EntityQuery;

	UPROPERTY()
	class UPDRTSBaseSubsystem* RTSSubsystem = nullptr;
};

UCLASS()
class PDRTSBASE_API UPDCollisionSignalProcessor : public UMassSignalProcessorBase
{
	GENERATED_BODY()

public:
	UPDCollisionSignalProcessor();

protected:	
	DECLARE_PROCESSOR_BODY

	
public:	
	FMassEntityQuery WorldOctreeEntityQuery{};

	UPROPERTY()
	class UPDRTSBaseSubsystem* RTSSubsystem = nullptr;
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