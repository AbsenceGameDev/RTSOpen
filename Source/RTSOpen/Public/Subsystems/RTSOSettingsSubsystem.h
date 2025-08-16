﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "RTSOpenCommon.h"
#include "GameFramework/GameUserSettings.h"
#include "Subsystems/EngineSubsystem.h"
#include "RTSOSettingsSubsystem.generated.h"

DECLARE_LOG_CATEGORY_CLASS(PDLog_SettingsHandler, Log, All);


/**
 * @brief  Loads custom tags that may have been added by a player/user
*/
UCLASS()
class RTSOPEN_API URTSOSettingsSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	/** @brief Dispatches an initial reset of the value stack. as the inputmodifer that supplies it is always testrun by the engine when instantiating all input modifiers upon startup, causing it to have a bunch of garbage in it's stack upon a worlds begin-play */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** @brief TODO */
   static URTSOSettingsSubsystem* Get();
	/** @brief TODO */
   void GenerateInitialSettingsEntries();
	/** @brief TODO */
   UFUNCTION() void OnCheckBox(bool bNewState, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnFloat(float NewFloat, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnInteger(int32 NewInteger, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnByte(uint8 NewByte, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnString(FString NewString, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnVector(FVector NewVector, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnVector2D(FVector2D NewVector2d, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnColour(FColor NewColour, const FGameplayTag& SettingsTag);
	/** @brief TODO */
   UFUNCTION() void OnKey(FRTSOSettingsKeyData NewKeys, const FGameplayTag& SettingsTag);
	/** @brief TODO */
	UFUNCTION() void SetProcessFunctionForSettingsBoundVariable(const FGameplayTag& SettingsTag, FString InQualifiedFunctionPath);
	template<typename TSettingsType> 
   TSettingsType OnProcessFunctionForSettings(const TSettingsType& bNewValue, const FGameplayTag &SettingsTag);


   /** @brief TODO */
   template<typename TDataType>
   void OnStringData(TDataType NewDatum, const FGameplayTag& SettingsTag)
   {
      if constexpr (TIsDerivedFrom<bool, TDataType>::Value){OnCheckBox(NewDatum, SettingsTag);}
      else if constexpr (TIsDerivedFrom<double, TDataType>::Value){OnFloat(NewDatum, SettingsTag);}
      else if constexpr (TIsDerivedFrom<int32, TDataType>::Value){OnInteger(NewDatum, SettingsTag);}
      else if constexpr (TIsDerivedFrom<uint8, TDataType>::Value){OnByte(NewDatum, SettingsTag);}
      else if constexpr (TIsDerivedFrom<FVector2D, TDataType>::Value){OnVector2D(NewDatum, SettingsTag);}
      else if constexpr (TIsDerivedFrom<FVector, TDataType>::Value){OnVector(NewDatum, SettingsTag);}
      else if constexpr (TIsDerivedFrom<FString, TDataType>::Value){OnString(NewDatum, SettingsTag);}
   }

   struct FRTSOSettingsDataSelector;
   template<typename TDataType, typename TInDataSelectorType = FRTSOSettingsDataSelector>
   static bool AttemptApplyDataSelectorOnString(const TInDataSelectorType& DataSelector, int32 SelectedItemIdx, const FGameplayTag& SettingsTag)
   {
      const TArray<TDataType>* DataListPtr = DataSelector.template GetAssociatedStringDataList<TDataType>();
		if(DataListPtr && DataListPtr->IsValidIndex(SelectedItemIdx))
      {
   		const TDataType SelectedValue = (*DataListPtr)[SelectedItemIdx];
         URTSOSettingsSubsystem::Get()->OnStringData<TDataType>(SelectedValue, SettingsTag);
         return true;
      }
      return false;
   }

   void* GetData(const FGameplayTag& SettingsTag);

   static PD::Settings::EType GetSettingsTypeFromCategory(const FGameplayTag& SettingsCategory);

#pragma region SaveStatics
   UFUNCTION(BlueprintCallable, Category = "File Operations")
   static bool SaveBinaryDataToFile(const FString& InFileName, const TArray<uint8>& InBinaryData);

   UFUNCTION(BlueprintCallable, Category = "File Operations")
   static bool LoadBinaryDataFromFile(const FString& InFileName, TArray<uint8>& OutBinaryData);

   UFUNCTION(BlueprintCallable, Category = "File Operations")
   static bool SaveAllSettingspData();

   UFUNCTION(BlueprintCallable, Category = "File Operations")
   static bool LoadAllSettingsData();
#pragma endregion // SaveStatics

	/** @brief Map that associates settigns tags with ufunctions using their objects paths as strings */
   UPROPERTY() TMap<FGameplayTag, FString> SettingsProcessFunctionPaths;
	/** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, bool> CheckBoxStates;
	/** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, double> DoubleStates;
	/** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, int32> IntegerStates;
	/** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, uint8> ByteStates;
   /** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, FString> StringStates;
	/** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, FVector> VectorStates;
   /** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, FVector2D> Vector2dStates;
   /** @brief TODO */
   UPROPERTY() TMap<FGameplayTag, FColor> ColourStates;
   /** @brief <KBM, Gamepad, Other> */
   UPROPERTY() TMap<FGameplayTag, FRTSOSettingsKeyData> KeyStates;

   static const FString SettingsLocalSaveFileName;
};


UCLASS(Config = "Game", DefaultConfig)
class RTSOPEN_API URTSOSettingsDeveloperSettings : public UDeveloperSettings
{
   GENERATED_BODY()
public:
	UPROPERTY(Config, EditAnywhere)
	FName SettingsStringTablePath = ("/Game/Localized/StringTables/ST_Settings.ST_Settings");   
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
