/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "Subsystems/RTSOSettingsSubsystem.h"
#include "RTSOpenCommon.h"
#include "Engine/Engine.h"


void URTSOSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
   
   GenerateInitialSettingsEntries();
}

URTSOSettingsSubsystem* URTSOSettingsSubsystem::Get()
{
   return GEngine != nullptr ? GEngine->GetEngineSubsystem<URTSOSettingsSubsystem>() : nullptr;
}

PD::Settings::EType URTSOSettingsSubsystem::GetSettingsTypeFromCategory(const FGameplayTag& SettingsCategory)
{
	// Settings - Gameplay categories
	if (SettingsCategory == PD::Settings::TAG_Camera)
   {
      return PD::Settings::EType::Camera;
   }
	else if (SettingsCategory == PD::Settings::TAG_ActionLog)
   {
      return PD::Settings::EType::ActionLog;
   }
	else if (SettingsCategory == PD::Settings::TAG_Difficulty)
   {
      return PD::Settings::EType::Difficulty;
   }

	// Settings - Video categories
	else if (SettingsCategory == PD::Settings::TAG_Display)
   {
      return PD::Settings::EType::Display;
   }
	else if (SettingsCategory == PD::Settings::TAG_Effects)
   {
      return PD::Settings::EType::Effects;
   } 
	else if (SettingsCategory == PD::Settings::TAG_Graphics)
   {
      return PD::Settings::EType::Graphics;
   }

	// Settings - Audio categories
	else if (SettingsCategory == PD::Settings::TAG_Audio)
   {
      return PD::Settings::EType::Audio;
   }
	else if (SettingsCategory == PD::Settings::TAG_Audio_Volume)
   {
      return PD::Settings::EType::Audio_Volume;
   }

	// Settings - Control categories
	else if (SettingsCategory == PD::Settings::TAG_Controls)
   {
      return PD::Settings::EType::Controls;
   }
	else if (SettingsCategory == PD::Settings::TAG_Controls_UI)
   {
      return PD::Settings::EType::Controls_UI;
   }

	// Settings - Interface categories
	else if (SettingsCategory == PD::Settings::TAG_Interface)
   {
      return PD::Settings::EType::Interface;
   } 

   return PD::Settings::EType::Num;
}

void URTSOSettingsSubsystem::GenerateInitialSettingsEntries()
{
	TMap<FGameplayTag, PD::Settings::FDataSelector> SettingsCategories{};
	SettingsCategories.Append(PD::Settings::FGameplaySettingsDefaults::Camera);
	SettingsCategories.Append(PD::Settings::FGameplaySettingsDefaults::ActionLog);
	SettingsCategories.Append(PD::Settings::FGameplaySettingsDefaults::Difficulty);

	SettingsCategories.Append(PD::Settings::FVideoSettingsDefaults::Display);
	SettingsCategories.Append(PD::Settings::FVideoSettingsDefaults::Effects);
	SettingsCategories.Append(PD::Settings::FVideoSettingsDefaults::Graphics);
	
	SettingsCategories.Append(PD::Settings::FAudioSettingsDefaults::Base);
	
	SettingsCategories.Append(PD::Settings::FControlsSettingsDefaults::Game);
	SettingsCategories.Append(PD::Settings::FControlsSettingsDefaults::UI);
	
	SettingsCategories.Append(PD::Settings::FInterfaceSettingsDefaults::Base);

   for (TTuple<FGameplayTag, PD::Settings::FDataSelector> DataTuple : SettingsCategories)
   {
      FGameplayTag SettingsTag = DataTuple.Key;
      PD::Settings::FDataSelector DataSelector = DataTuple.Value;
      switch(DataSelector.ValueType)
      {
      case ERTSOSettingsType::Boolean:
         OnCheckBox(DataSelector.GetRef<bool>(), SettingsTag);
      continue;
      case ERTSOSettingsType::FloatSlider:
      case ERTSOSettingsType::FloatSelector:
         OnFloat(DataSelector.GetRef<double>(), SettingsTag);
      continue;
      case ERTSOSettingsType::IntegerSlider:
      case ERTSOSettingsType::IntegerSelector:
         OnInteger(DataSelector.GetRef<int32>(), SettingsTag);
      continue;
      case ERTSOSettingsType::EnumAsByte:
         OnByte(DataSelector.GetRef<uint8>(), SettingsTag);
      continue;
      case ERTSOSettingsType::String:
         OnString(DataSelector.GetRef<FString>(), SettingsTag);
      continue;
      case ERTSOSettingsType::Vector3:
         OnVector(DataSelector.GetRef<FVector>(), SettingsTag);
      continue;
      case ERTSOSettingsType::Vector2:
         OnVector2D(DataSelector.GetRef<FVector2D>(), SettingsTag);
      continue;
      case ERTSOSettingsType::Colour:
         OnColour(DataSelector.GetRef<FColor>(), SettingsTag);
      continue;
      case ERTSOSettingsType::Key:
         OnKey(DataSelector.GetRef<FRTSOSettingsKeyData>(), SettingsTag);
      continue;
      }
   }
}

void URTSOSettingsSubsystem::OnCheckBox(bool bNewState, const FGameplayTag &SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnCheckBox(%s)"), *SettingsTag.ToString())
   CheckBoxStates.FindOrAdd(SettingsTag, bNewState);
}
void URTSOSettingsSubsystem::OnFloat(float NewFloat, const FGameplayTag &SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnFloat(%s)"), *SettingsTag.ToString())
   DoubleStates.FindOrAdd(SettingsTag, NewFloat);   
}
void URTSOSettingsSubsystem::OnInteger(int32 NewInteger, const FGameplayTag &SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnInteger(%s)"), *SettingsTag.ToString())
   IntegerStates.FindOrAdd(SettingsTag, NewInteger);
}
void URTSOSettingsSubsystem::OnByte(uint8 NewByte, const FGameplayTag &SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnByte(%s)"), *SettingsTag.ToString())
   ByteStates.FindOrAdd(SettingsTag, NewByte);   
}
void URTSOSettingsSubsystem::OnString(FString NewString, const FGameplayTag& SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubssytem::OnString(%s)"), *SettingsTag.ToString())
   StringStates.FindOrAdd(SettingsTag, NewString);
}

void URTSOSettingsSubsystem::OnVector(FVector NewVector, const FGameplayTag& SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnVector(%s)"), *SettingsTag.ToString())
   VectorStates.FindOrAdd(SettingsTag, NewVector);   
}
void URTSOSettingsSubsystem::OnVector2D(FVector2D NewVector2d, const FGameplayTag& SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnVector2D(%s)"), *SettingsTag.ToString())
   Vector2dStates.FindOrAdd(SettingsTag, NewVector2d);   
}
void URTSOSettingsSubsystem::OnColour(FColor NewColour, const FGameplayTag& SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnColour(%s)"), *SettingsTag.ToString())
   ColourStates.FindOrAdd(SettingsTag, NewColour);   
}
void URTSOSettingsSubsystem::OnKey(FRTSOSettingsKeyData NewKeys, const FGameplayTag& SettingsTag)
{
   UE_LOG(PDLog_SettingsHandler, Warning, TEXT("URTSOSettingsSubsystem::OnKey(%s)"), *SettingsTag.ToString())
   KeyStates.FindOrAdd(SettingsTag, NewKeys);   
}

void* URTSOSettingsSubsystem::GetData(const FGameplayTag& SettingsTag)
{
   if (CheckBoxStates.Contains(SettingsTag)) {return CheckBoxStates.Find(SettingsTag);}
   if (DoubleStates.Contains(SettingsTag)) {return DoubleStates.Find(SettingsTag);}
   if (IntegerStates.Contains(SettingsTag)) {return IntegerStates.Find(SettingsTag);}
   if (ByteStates.Contains(SettingsTag)) {return ByteStates.Find(SettingsTag);}
   if (VectorStates.Contains(SettingsTag)) {return VectorStates.Find(SettingsTag);}
   if (Vector2dStates.Contains(SettingsTag)) {return Vector2dStates.Find(SettingsTag);}
   if (ColourStates.Contains(SettingsTag)) {return ColourStates.Find(SettingsTag);}
   if (StringStates.Contains(SettingsTag)) {return StringStates.Find(SettingsTag);}
   if (KeyStates.Contains(SettingsTag)) {return KeyStates.Find(SettingsTag);}

   return nullptr;
}

const FString URTSOSettingsSubsystem::SettingsLocalSaveFileName = "LocalSettingsData.dat";

bool URTSOSettingsSubsystem::SaveBinaryDataToFile(const FString& InFileName, const TArray<uint8>& InBinaryData)
{
    FString SaveDirectory = FPaths::ProjectPersistentDownloadDir();
    FString AbsoluteFilePath = FPaths::Combine(SaveDirectory, InFileName);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (PlatformFile.DirectoryExists(*SaveDirectory) == false)
    {
        if (PlatformFile.CreateDirectory(*SaveDirectory) == false)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create directory: %s"), *SaveDirectory);
            return false;
        }
    }

    if (FFileHelper::SaveArrayToFile(InBinaryData, *AbsoluteFilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Binary data saved successfully to: %s"), *AbsoluteFilePath);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save binary data to: %s"), *AbsoluteFilePath);
        return false;
    }
}

bool URTSOSettingsSubsystem::LoadBinaryDataFromFile(const FString& InFileName, TArray<uint8>& OutBinaryData)
{
    FString SaveDirectory = FPaths::ProjectPersistentDownloadDir();
    FString AbsoluteFilePath = FPaths::Combine(SaveDirectory, InFileName);

    if (FFileHelper::LoadFileToArray(OutBinaryData, *AbsoluteFilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Binary data loaded successfully from: %s"), *AbsoluteFilePath);
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load binary data from: %s (File might not exist or is empty)"), *AbsoluteFilePath);
        OutBinaryData.Empty(); 
        return false;
    }
}

bool URTSOSettingsSubsystem::SaveAllSettingspData()
{
	TArray<uint8> BinaryData;
	FMemoryWriter Writer(BinaryData, true);
	URTSOSettingsSubsystem* SettingsSubsystem = URTSOSettingsSubsystem::Get();
   Writer << SettingsSubsystem->CheckBoxStates;
   Writer << SettingsSubsystem->DoubleStates;
   Writer << SettingsSubsystem->IntegerStates;
   Writer << SettingsSubsystem->ByteStates;
   Writer << SettingsSubsystem->StringStates;
   Writer << SettingsSubsystem->VectorStates;
   Writer << SettingsSubsystem->Vector2dStates;
   Writer << SettingsSubsystem->ColourStates;
   Writer << SettingsSubsystem->KeyStates;
   return SaveBinaryDataToFile(URTSOSettingsSubsystem::SettingsLocalSaveFileName, BinaryData);
}

bool URTSOSettingsSubsystem::LoadAllSettingsData()
{
   TArray<uint8> BinaryData;
   if (LoadBinaryDataFromFile(URTSOSettingsSubsystem::SettingsLocalSaveFileName, BinaryData))
   {
      if (BinaryData.Num() > 0)
      {
      	URTSOSettingsSubsystem* SettingsSubsystem = URTSOSettingsSubsystem::Get();
         FMemoryReader Reader(BinaryData, true); 
         Reader << SettingsSubsystem->CheckBoxStates;
         Reader << SettingsSubsystem->DoubleStates;
         Reader << SettingsSubsystem->IntegerStates;
         Reader << SettingsSubsystem->ByteStates;
         Reader << SettingsSubsystem->StringStates;
         Reader << SettingsSubsystem->VectorStates;
         Reader << SettingsSubsystem->Vector2dStates;
         Reader << SettingsSubsystem->ColourStates;
         Reader << SettingsSubsystem->KeyStates;
         Reader.Close();
         return true;
      }
   }
	
    return false;
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
