/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "Actors/PDRTSCameraManager.h"

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
	// If TagToSettings has a valid entry, then TagToTable & TagToRowname also have the same valid entries  
	if (TagToSettings.Contains(Tag) == false)
	{
		FString BuildString =
			"APDCameraManager::SetCustomMode -- "
			"\n Trying to add set camera mode without a valid gameplay tag. Skipping processing entry";
		UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);
		
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
				UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);

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
				UE_LOG(LogTemp, Error, TEXT("%s"), *BuildString);

				// @todo Write some test cases and some data validation to handle this properly, the logs will do for now  
				continue;
			}
			

			TagToSettings.Emplace(DefaultDatum->CameraMode) = DefaultDatum;
			TagToTable.Emplace(CameraMode) = Table;
			TagToRowname.Emplace(CameraMode) = Name;
		}
	}
}

/*
 * @copyright Permafrost Development (MIT license) 
 * Authors: Ario Amin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/