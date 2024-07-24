﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PDInteractInterface.h"

#include "PDInteractActor.generated.h"

/** @brief Constant to make some runtime calculations a bit faster */
constexpr double UNREALUNITS_PERMETRE = 100.0; /* UU == cm, 100 uu == 1m*/
/** @brief Constant to make some runtime calculations a bit faster */
constexpr double INVERSE_UU = 1 / UNREALUNITS_PERMETRE; /* UU == cm, 100 uu == 1m*/

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;

/**
 * @brief Base interactable actor.
 * Handles default behaviour for an interactable actor out-of-the box.
 * - Registers with the interactable subsystem upon begin-play
 * - Binds editor delegates
 * - If given: Handles processing custom interaction process function delegates and passes result downstream
 * - Handles building a base-line interaction message filling in necessary data from the actor
 * - Resizes collision bounds
 */
UCLASS()
class PDINTERACTION_API APDInteractActor
	: public AActor
	, public IPDInteractInterface
{
	GENERATED_BODY()

public:
	APDInteractActor();

	/** @brief Calls super then bind collision box delegate(s) */
	virtual void OnConstruction(const FTransform& Transform) override;
	/** @brief Registers the actor with the interaction subsystem */
	virtual void BeginPlay() override;
	/** @brief Deregisters the actor with the interaction subsystem */
	virtual void BeginDestroy() override;
	/** @brief Default setting Disabled. Only Calls Super. Reserved for later use */
	virtual void Tick(float DeltaTime) override;

	/** @brief Passes the parameters to the interaction interface which processes the custom process function, if any was given. and returns state or if it was unhandled */
	virtual void OnInteract_Implementation(const FPDInteractionParamsWithCustomHandling& InteractionParams, EPDInteractResult& InteractResult) const override;
	/** @brief Returns the interaction message. Returns actor-name and game-action from property 'ActorMessage' */
	UFUNCTION(BlueprintCallable)
	virtual const FPDInteractMessage& GetInteractionMessage() override;

	UFUNCTION(BlueprintCallable)
	virtual const FPDInteractionSettings& GetInteractionSettings() const override;
	
	virtual double GetInteractionTime_Implementation() const override { return GetInteractionSettings().InteractionTimeInSeconds; };
protected:
	/** @brief Function that resizes the collision bounds based on the property 'UniformCollisionPadding' */
	UFUNCTION() 
	void ResizeCollisionBounds(UStaticMeshComponent* NewMeshDummy = nullptr);
	/** @brief Binds delegate(s) to Mesh->OnStaticMeshChanged */
	void BindDelegates();

#if WITH_EDITOR
	/** @brief Calls 'ResizeCollisionBounds' if the expected property 'UniformCollisionPadding' was modified */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	/** @brief Friendly readable actor name */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	FPDInteractMessage ActorMessage;

	/** @brief Root of the actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	USceneComponent* Scenecomp = nullptr;

	/** @brief Mesh of the actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	UStaticMeshComponent* Mesh = nullptr;

	/** @brief Box collision that acts as the interaction surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	UBoxComponent* Boxcomp = nullptr;
	
	/** @brief Value that controls how much we want to control ur padding when sizing the collision bounds  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	double UniformCollisionPadding = UNREALUNITS_PERMETRE * 0.4;
	
	/** @brief Handle to the interaction settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType="/Script/PDInteraction.PDInteractionSettings"))
	FDataTableRowHandle InteractionSettingsHandle;
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