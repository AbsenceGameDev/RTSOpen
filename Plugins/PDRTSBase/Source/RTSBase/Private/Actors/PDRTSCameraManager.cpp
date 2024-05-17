﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "Actors/PDRTSCameraManager.h"

#include "PDRTSCommon.h"

APDCameraManager::APDCameraManager()
{
	RequestedCameraState = OldCameraState = FGameplayTag{};
}

void APDCameraManager::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void APDCameraManager::BeginPlay()
{
	Super::BeginPlay();

	ProcessTables();
}

// Called by the engine
void APDCameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);

	const bool bAlreadyApplied = RequestedCameraState == OldCameraState;
	if (bAlreadyApplied) { return; }
	
	SetCustomMode(RequestedCameraState);
	OldCameraState  = RequestedCameraState;
}

void APDCameraManager::SetCustomMode_Implementation(FGameplayTag Tag)
{
	// If TagToSettings has a valid entry, then TagToTable & TagToRowName also have the same valid entries  
	if (TagToSettings.Contains(Tag) == false)
	{
		const FString BuildString =
			"APDCameraManager::SetCustomMode -- "
			"\n Trying to add set camera mode without a valid gameplay tag. Skipping processing entry";
		UE_LOG(PDLog_RTSBase, Error, TEXT("%s"), *BuildString);
		
		return;
	}
	
	// Set up camera for custom mode

	// Overwriting current selected settings
	CurrentSettingsHandle.DataTable = TagToTable.FindRef(Tag);
	CurrentSettingsHandle.RowName   = TagToRowname.FindRef(Tag);
	
	const FPDCameraManagerSettings& Settings = *TagToSettings.FindRef(Tag);

	// Set up camera for given custom modes
	ViewPitchMin = Settings.Pitch.Min;
	ViewPitchMax = Settings.Pitch.Max;
	ViewYawMin = Settings.Yaw.Min;
	ViewYawMax = Settings.Yaw.Max;

	SetFOV(Settings.FOV);

	bIsOrthographic = Settings.OrthoWidth >= 1.0f;
	SetOrthoWidth(Settings.OrthoWidth);
	
	const bool bSkipFadeSettings = Settings.FadeAmount <= (SMALL_NUMBER - 1.0);
	if (bSkipFadeSettings) { return; }
	
	FadeAmount = Settings.FadeAmount;
	FadeColor  = Settings.FadeColour;
}

void APDCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (SettingPtr != nullptr && SettingPtr->CameraLagSpeed > SMALL_NUMBER)
	{
		const FVector DesiredPos = OutVT.POV.Location;
		const FVector CurrentPos = OutVT.POV.Location + CurrentLagVelocity;

		const FVector NewCameraLagVelocity = (DesiredPos - CurrentPos) * SettingPtr->CameraLagSpeed;
		CurrentLagVelocity = NewCameraLagVelocity * DeltaTime  +  CurrentLagVelocity * (1.f - DeltaTime);

		OutVT.POV.Location = DesiredPos - CurrentLagVelocity;
	}
}


void APDCameraManager::ProcessTables()
{
	for (const UDataTable* Table : CameraSettingsSources)
	{
		if (Table == nullptr
			|| Table->IsValidLowLevelFast() == false
			|| Table->RowStruct != FPDCameraManagerSettings::StaticStruct())
		{
			continue;
		}

		TArray<FPDCameraManagerSettings*> Rows;
		Table->GetAllRows("", Rows);
		TArray<FName> RowNames = Table->GetRowNames();
		
		for (const FName& Name : RowNames)
		{
			FPDCameraManagerSettings* DefaultDatum = Table->FindRow<FPDCameraManagerSettings>(Name,"");
			check(DefaultDatum != nullptr) // This should never be nullptr

			const FGameplayTag& CameraMode = DefaultDatum->CameraMode;

			if (CameraMode.IsValid() == false)
			{
				FString BuildString = "APDCameraManager::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add settings on row (%s) Which does not have a valid gameplay tag. Skipping processing entry"), *Name.ToString());
				UE_LOG(PDLog_RTSBase, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			
			// @note If duplicates, ignore duplicate and output errors to screen and to log
			if (TagToSettings.Contains(CameraMode))
			{
				const UDataTable* RetrievedTable = *TagToTable.Find(CameraMode);
				
				FString BuildString = "APDCameraManager::ProcessTables -- "
				+ FString::Printf(TEXT("Processing table(%s)"), *Table->GetName()) 
				+ FString::Printf(TEXT("\n Trying to add setting(%s) which has already been added by previous table(%s)."),
						*CameraMode.GetTagName().ToString(), RetrievedTable != nullptr ? *RetrievedTable->GetName() : *FString("INVALID TABLE"));
				UE_LOG(PDLog_RTSBase, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			

			TagToSettings.Emplace(DefaultDatum->CameraMode) = DefaultDatum;
			TagToTable.Emplace(CameraMode) = Table;
			TagToRowname.Emplace(CameraMode) = Name;
		}
	}
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