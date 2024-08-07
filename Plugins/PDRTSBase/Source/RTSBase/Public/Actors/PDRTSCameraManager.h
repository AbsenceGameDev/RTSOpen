﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "PDRTSCameraManager.generated.h"

/** @brief A simple struct which is acting as an intervals boundaries*/
USTRUCT(BlueprintType, Blueprintable)
struct PDRTSBASE_API FPDInterval
{
    GENERATED_BODY()

	/** @brief Minimum value of interval boundary */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double Min = -89.0;
    
	/** @brief Maximum value of interval boundary */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double Max = 89.0;
};

/** @brief Settings table row type for the PDCameraManager.
 * - Contains the Camera Mode tag
 * - Contains a Pitch interval range
 * - Contains a Yaw interval range
 * - Controls the Cameras FOV
 * - Controls the Cameras near clip plane
 * - Custom camera lag
 * 
 */
USTRUCT(BlueprintType, Blueprintable)
struct PDRTSBASE_API FPDCameraManagerSettings : public FTableRowBase
{
    GENERATED_BODY()

	/** @brief Tag which represents camera mode. Similar to using an enum but more scalable*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FGameplayTag CameraMode{};

	/** @brief The current allowed pitch interval, Use to clamp look rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FPDInterval Pitch{-89.0, 89.0};

	/** @brief The current allowed yaw interval, Use to clamp look rotation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FPDInterval Yaw{0.0, 359.999};    
    
	/** @brief Active/Current FOV */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double FOV = 90.0;

	/** @brief Active/Current NearClipPlane */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double NearClipPlane = 5.0;
    
    /** @brief 0 is Interpreted as Ortho view being disabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double OrthoWidth = 0.0;
    
	/** @brief Custom camera lag. Unused. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double CameraLagSpeed = 0.0;

	/** @brief Revise if actually wanted */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double FadeAmount = -1.0;

	/** @brief Revise if actually wanted */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FLinearColor FadeColour;
};

/** @brief The camera manager class. Gets settings from a settings datatable-row handle */
UCLASS()
class PDRTSBASE_API APDCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:
    APDCameraManager();
    
	/** @brief Reserved for later use. Only calls Super::OnConstruction. */
    virtual void OnConstruction(const FTransform& Transform) override;
	/** @brief Calls Super::BeginPlay then proceeds to call 'ProcessTables' */
    virtual void BeginPlay() override;

	/** @brief Sets the given mode and loads the settings for that mode */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Camera)
    void SetCustomMode(FGameplayTag Tag);

	/** @brief Updates the RequestedCameraState and calls 'SetCustomMode' if it has changed */
    virtual void UpdateCamera(float DeltaTime) override;

protected:
	/** @brief Updates the view-target with custom camera lag applied */
    virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
    
	/** @brief Process all camera settings tables */
    void ProcessTables();
    
public:
	/** @brief Camera settings handle. Points to the camera settings entry we want to apply to this manager. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", Meta = (RowType = "/Script/PDRTSBase.PDCameraManagerSettings"))
    FDataTableRowHandle CurrentSettingsHandle;
	/** @brief Pointer to the actual settings after it has been resolved by 'CurrentSettingsHandle' */
    FPDCameraManagerSettings* SettingPtr = nullptr;
    
	/** @brief Potential Sources for camera settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", Meta = (RequiredAssetDataTags = "RowStructure=/Script/PDRTSBase.PDCameraManagerSettings"))
    TArray<UDataTable*> CameraSettingsSources{};

	/** @brief Camera State: Requested*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FGameplayTag RequestedCameraState{};

	/** @brief Camera State: Previous */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    FGameplayTag OldCameraState{};    

    /* Associative maps */
	/** @brief Map to associate a type tag with the manager settings it points to in the table it was sourced from */
    TMap<FGameplayTag /*Type tag*/, FPDCameraManagerSettings* /*SettingsRow*/> TagToSettings;
	/** @brief Map to associate a type tag with the table it was sourced from */
    TMap<FGameplayTag /*Type tag*/, const UDataTable* /*Table*/> TagToTable;
	/** @brief Map to associate a type tag with the rowname of the entry it was sourced from */
    TMap<FGameplayTag /*Type tag*/, FName /*Rowname*/> TagToRowname;

	/** @brief Current lag velocity value. Use for the custom camera lag */
    UPROPERTY()
    FVector CurrentLagVelocity{0.0,0.0,0.0};
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