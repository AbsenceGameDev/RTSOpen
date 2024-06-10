﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#pragma once

#include "CoreMinimal.h"
#include "PDRTSCommon.h"
#include "Actors/GodHandPawn.h"
#include "Actors/PDInteractActor.h"
#include "AI/Mass/RTSOMassFragments.h"
#include "Components/PDInventoryComponent.h"
#include "Interfaces/PDRTSBuildableGhostInterface.h"
#include "RTSOInteractableBuildingBase.generated.h"

struct FRTSOLightInventoryFragment;

USTRUCT(Blueprintable)
struct FRTSOBuildableInventories
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRTSOLightInventoryFragment> LightInventoriesPerGhostStage{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRTSOLightInventoryFragment LightInventoryAsMain{};	
};


/**
 * @brief An actor with a job tag tied to it, meant to be used for interactable buildings
 * @todo declare and define the OnInteract interface function
 */
UCLASS()
class RTSOPEN_API ARTSOInteractableBuildingBase
	: public APDInteractActor
	, public IPDRTSBuildableGhostInterface
{
	GENERATED_BODY()

public:
	/** @brief Sets default job to 'TAG_AI_Job_WalkToTarget' and enables the tickcomponent*/ 
	ARTSOInteractableBuildingBase();
	
	template<bool TIsGhost>
	void RefreshStaleSettings(){};
	
	/** @brief Only calls Super. Reserved for later use  */
	virtual void Tick(float DeltaTime) override;
	/** @brief Only calls Super. Reserved for later use  */
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	/** @brief adds 'JobTag' to a 'FGameplayTagContainer' and returns it */
	virtual FGameplayTagContainer GetGenericTagContainer_Implementation() const override;

	/* APDInteractActor Interface Start */
	virtual bool GetCanInteract_Implementation() const override;
	virtual void OnInteract_Implementation(const FPDInteractionParamsWithCustomHandling& InteractionParams, EPDInteractResult& InteractResult) const override;
	/* APDInteractActor Interface End */
	
	template<bool TIsGhost>
	void ProcessSpawn();
	bool WithdrawRecurringCostFromBankOrEntity(UPDInventoryComponent* Bank, FRTSOLightInventoryFragment* EntityInv, const int32& ImmutableStage);
	void ProcessIfWorkersRequired();
	void ProcessIfWorkersNotRequired();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "")
	void OnBuildingDestroyed();
	
	/* IPDRTSBuildableGhostInterface Interface Start */
	virtual void OnSpawnedAsGhost_Implementation(const FGameplayTag& BuildableTag, bool bInIsPreviewGhost, bool bInRequiresWorkersToBuild) override;
	virtual void OnSpawnedAsMain_Implementation(const FGameplayTag& BuildableTag)  override;
	virtual void TransitionFromGhostToMain_Implementation() override;
	void Internal_ProgressGhostStage(const bool bForceProgressThroughStage, const bool bChainAll);
	virtual void ProgressGhostStage_Implementation(const bool bChainAll) override;
	virtual bool AttemptFinalizeGhost_Implementation() override;
	/* IPDRTSBuildableGhostInterface Interface End */
	
	virtual FRTSOBuildableInventories& ReturnBuildableInventories();

private:
	void RefreshStaleSettings_Ghost();
	void RefreshStaleSettings_Main();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPDRTSBuildableCollisionSettings BuildableCollisionSettings{false};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPDRTSBuildableCollisionSettings GhostCollisionSettings{false};

	bool CurrentStateFinishedProgressing = false;
	bool RunningStateProgressFunction = true;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	FRTSOBuildableInventories BuildableInventories{};
	
	/** @brief JobTag associated with this actor */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	FGameplayTag JobTag{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	UMaterialInstance* MainMat = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	UMaterialInstance* GhostMat = nullptr;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	FGameplayTag InstigatorBuildableTag{};

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess="true"))
	bool bIsPreviewGhost = true; // Default to preview
	

	static inline FName BoxcompName = "Boxcomp"; 
	static inline FName MeshName = "Mesh"; 
};

template<>
void inline ARTSOInteractableBuildingBase::RefreshStaleSettings<true>()
{
	RefreshStaleSettings_Ghost();
};
template<>
void inline ARTSOInteractableBuildingBase::RefreshStaleSettings<false>()
{
	RefreshStaleSettings_Main();
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
