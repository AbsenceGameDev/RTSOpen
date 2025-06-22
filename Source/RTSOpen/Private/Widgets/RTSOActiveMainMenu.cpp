/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */


#include "Widgets/RTSOActiveMainMenu.h"
#include "Subsystems/RTSOSettingsSubsystem.h"

#include "SlateFwd.h"
#include "CommonTabListWidgetBase.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/WidgetSwitcherSlot.h"

#include "Core/RTSOBaseGM.h"
#include "UI/PDNumberBoxes.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
// #include "Components/CanvasPanel.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/NamedSlot.h"
#include "Components/VerticalBox.h"
// #include "Components/TextBlock.h"
// #include "Components/Image.h"

void URTSOSaveGameDialog::DialogReplyContinue(const bool bSuccess) const
{
	// ARTSOBaseGM* GM = GetWorld() != nullptr ? GetWorld()->GetAuthGameMode<ARTSOBaseGM>() : nullptr;
	// if (GM == nullptr) { return; }

	if (bSuccess) { SaveSuccessCallback.ExecuteIfBound(FString::FromInt(SlotIdx), true); }
	else { SaveFailCallback.ExecuteIfBound(FString::FromInt(SlotIdx), true); }
	
}

// void URTSOSaveGameDialog::SetupDelegates()
// {
// 	YesButton->Hitbox->OnReleased.AddDynamic(this, &URTSOSaveGameDialog::Reply_Yes);
// 	NoButton->Hitbox->OnReleased.AddDynamic(this, &URTSOSaveGameDialog::Reply_No);
//
// 	if (DialogContent == nullptr) { return; }
// 	DialogContent->SetText(DialogMessage);
// }

// void URTSOSaveGameDialog::Reply_Yes()
// {
// 	DialogReplyContinue();
// 	RemoveFromParent();
// }
//
// void URTSOSaveGameDialog::Reply_No()
// {
// 	RemoveFromParent();
// }

void URTSOMenuWidget_SaveGame::NativePreConstruct()
{
	// BaseCanvas->AddChildToCanvas(MainBox);
	// MainBox->AddChildToVerticalBox(BannerCanvas);
	// BannerCanvas->AddChildToCanvas(BannerText);
	// BannerCanvas->AddChildToCanvas(BannerImage);
	// BannerCanvas->AddChildToCanvas(ExitButton);
	// MainBox->AddChildToVerticalBox(InnerBox);
	// InnerBox->AddChildToVerticalBox(Slot0);
	// InnerBox->AddChildToVerticalBox(Slot1);
	// InnerBox->AddChildToVerticalBox(Slot2);
	// InnerBox->AddChildToVerticalBox(Slot3);
	// InnerBox->AddChildToVerticalBox(Slot4);
	Super::NativePreConstruct();
}

void URTSOMenuWidget_SettingsEntry::NativePreConstruct()
{
	Super::NativePreConstruct();
	
	InputValueWidgetSwitcher->SetActiveWidgetIndex(bIsCheckBox);

	if (bIsCheckBox)
	{
		TScriptDelegate InDelegate;
		InDelegate.BindUFunction(this, TEXT("OnCheckBoxSet"));
		AsCheckBox->OnCheckStateChanged.AddUnique(InDelegate);
	}
}

void URTSOMenuWidget_SettingsEntry::OnCheckBoxSet(bool bNewState)
{
	OnCheckBoxStateChanged.Broadcast(bNewState);
}

void URTSOMenuWidget_SettingsEntry::NativeOnActivated()
{
	Super::NativeOnActivated();
}

void URTSOMenuWidget_SettingsCategory::NativeOnActivated()
{
	Super::NativeOnActivated();
	Refresh(EntryClass, EntryCategoryName, EntriesData);
}

void URTSOMenuWidget_SettingsCategory::NativePreConstruct()
{
	Super::NativePreConstruct();
	Refresh(EntryClass, EntryCategoryName, EntriesData);
}

void URTSOMenuWidget_SettingsCategory::Refresh(
	TSubclassOf<URTSOMenuWidget_SettingsEntry> InEntryClass,
	FString InEntryCategoryName, 
	const TMap<FGameplayTag, FRTSOSettingsDataSelector>& InEntriesData)
{
	if (InEntryClass == nullptr)
	{
		return;
	}
	
	EntryClass = InEntryClass;
	EntryCategoryName = InEntryCategoryName;
	EntriesData = InEntriesData;


	SettingsCategoryLabel->SetText(FText::FromString(EntryCategoryName));

	ItemContentBox->ClearChildren();

	for (auto& [SettingsTag, DataSelector] : InEntriesData)
	{
		URTSOMenuWidget_SettingsEntry* SettingsEntry = CreateWidget<URTSOMenuWidget_SettingsEntry>(this, EntryClass);
		SettingsEntry->SettingsEntryLabel->SetText(FText::FromString(SettingsTag.ToString()));
		SettingsEntry->SettingsEntryDescription->SetText(FText::FromString("TODO!"));

		switch (DataSelector.ValueType)
		{
		case ERTSOSettingsType::Boolean:
			if (SettingsEntry->AsCheckBox != nullptr && SettingsEntry->AsCheckBox->IsValidLowLevel())
			{
				SettingsEntry->bIsCheckBox = true;
				SettingsEntry->AsCheckBox->SetCheckedState(DataSelector.AsBool ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
				SettingsEntry->OnCheckBoxStateChanged.AddLambda(
					[SettingsTag](bool bNewState) 
					{
						URTSOSettingsSubsystem::Get()->OnCheckBox(bNewState, SettingsTag);
					});
			}
			break;
		case ERTSOSettingsType::FloatSelector:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedIncrementBox;
				SettingsEntry->RangedSelector->SelectedCount = DataSelector.AsDouble * PD::Settings::FloatUIMultiplier;
				SettingsEntry->RangedSelector->PostValueChanged.AddLambda(
					[SettingsTag](int32 NewValue) 
					{
						URTSOSettingsSubsystem::Get()->OnFloat(static_cast<double>(NewValue) / PD::Settings::FloatUIMultiplier, SettingsTag);
					});
				SettingsEntry->RangedSelector->ApplySettings(
					DataSelector.DoubleRange.Min * PD::Settings::FloatUIMultiplier,
					DataSelector.DoubleRange.Max * PD::Settings::FloatUIMultiplier);
			}
			
			break;
		case ERTSOSettingsType::FloatSlider:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedSlider;				
				SettingsEntry->RangedSelector->SelectedCount = DataSelector.AsDouble * PD::Settings::FloatUIMultiplier;
				SettingsEntry->RangedSelector->PostValueChanged.AddLambda(
					[SettingsTag](int32 NewValue) 
					{
						URTSOSettingsSubsystem::Get()->OnFloat(static_cast<double>(NewValue) / PD::Settings::FloatUIMultiplier, SettingsTag);
					});
				SettingsEntry->RangedSelector->ApplySettings(
					DataSelector.DoubleRange.Min * PD::Settings::FloatUIMultiplier,
					DataSelector.DoubleRange.Max * PD::Settings::FloatUIMultiplier);						
			}
			
			break;
		case ERTSOSettingsType::IntegerSelector:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedIncrementBox;
				SettingsEntry->RangedSelector->SelectedCount = DataSelector.AsInteger;
				SettingsEntry->RangedSelector->PostValueChanged.AddLambda(
					[SettingsTag](int32 NewValue) 
					{
						URTSOSettingsSubsystem::Get()->OnFloat(NewValue, SettingsTag);
					});
				SettingsEntry->RangedSelector->ApplySettings(
					DataSelector.IntegerRange.Min,
					DataSelector.IntegerRange.Max);	
			}
			
			break;
		case ERTSOSettingsType::IntegerSlider:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedSlider;
				SettingsEntry->RangedSelector->SelectedCount = DataSelector.AsInteger;
				SettingsEntry->RangedSelector->PostValueChanged.AddLambda(
					[SettingsTag](int32 NewValue) 
					{
						URTSOSettingsSubsystem::Get()->OnFloat(NewValue, SettingsTag);
					});
				SettingsEntry->RangedSelector->ApplySettings(
					DataSelector.IntegerRange.Min,
					DataSelector.IntegerRange.Max);					
			}
			
			break;
		case ERTSOSettingsType::Vector2:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedSlider;
				// TODO
			}
			
			break;
		case ERTSOSettingsType::Vector3:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedSlider;
				// TODO
			}
			
			break;
		case ERTSOSettingsType::Colour:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedSlider;
				// TODO
			}
			
			break;
		case ERTSOSettingsType::String:
			// TODO ;
			break;
		case ERTSOSettingsType::EnumAsByte:
			if (SettingsEntry->RangedSelector != nullptr && SettingsEntry->RangedSelector->IsValidLowLevel())
			{
				SettingsEntry->RangedSelector->CountTypeSelector = EPDSharedUICountTypeSelector::ERangedIncrementBox;

				//
				// TODO

				// SettingsEntry->RangedSelector->PostValueChanged.AddUObject(URTSOSettingsSubsystem::Get(), &URTSOSettingsSubsystem::OnByte, SettingsTag);
				// SettingsEntry->RangedSelector->ApplySettings(
				// 	DataSelector.IntegerRange.Min,
				// 	DataSelector.IntegerRange.Max);					
			}
			
			break;
		case ERTSOSettingsType::Key:
			// TODO
			break;
		}
		

		
		ItemContentBox->AddChildToVerticalBox(SettingsEntry);
	}	
	
}

void URTSOMenuWidget_BaseMenu::NativeOnActivated()
{
	Super::NativeOnActivated();
	// todo
}

void URTSOMenuWidget_BaseMenu::OnCategoryPressed(FName TabName)
{
	TArray<UWidget*> Children = SubmenuSelector->GetAllChildren();
	for (UWidget* ChildWidget : Children)
	{
		UPDGenericButton* AsGenericButton = Cast<UPDGenericButton>(ChildWidget);
		if (AsGenericButton == nullptr || AsGenericButton->IsValidLowLevelFast() == false)
		{
			continue;
		}

		if (TabName.Compare(FName(*AsGenericButton->TextBlock->GetText().ToString())) == false)
		{
			continue;
		}
		AsGenericButton->ChildWidget->SetVisibility(ESlateVisibility::Visible);
		InnerContent->SetContent(AsGenericButton->ChildWidget);
	}
}

void URTSOMenuWidget_BaseMenu::NativePreConstruct()
{
	Super::NativePreConstruct();
	// todo
}

void URTSOMenuWidget_Settings::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (SubmenuSelector != nullptr && SubmenuSelector->IsValidLowLevelFast())
	{
		SubmenuSelector->ClearChildren();
	}

	if (SubMenuClass == nullptr || SettingsCategoryClass == nullptr || TabButtonClass == nullptr || SettingsEntryClass == nullptr
		|| SubMenuClass->IsValidLowLevelFast() == false || SettingsCategoryClass->IsValidLowLevelFast() == false
		|| TabButtonClass->IsValidLowLevelFast() == false || SettingsEntryClass->IsValidLowLevelFast() == false) 
	{
		return;
	}

	for (PD::Settings::ERTSOSettingsGroups TopLevelSetting : PD::Settings::AllSettingsDefaults.TopLevelSettingTypes)
	{
		PD::Settings::FSettingsDefaultsBase* SettingsDefaultsBase = PD::Settings::AllSettingsDefaults.Get(TopLevelSetting);
		if (SettingsDefaultsBase == nullptr)
		{
			continue;
		}

		TMap<FString, TMap<FGameplayTag, FRTSOSettingsDataSelector>> SettingsCategories{};

		switch (SettingsDefaultsBase->TabName)
		{
		case PD::Settings::ERTSOSettingsGroups::Gameplay:
			{
				PD::Settings::FGameplaySettingsDefaults* GameplayDefaults =
				   static_cast<PD::Settings::FGameplaySettingsDefaults*>(SettingsDefaultsBase);
				SettingsCategories.Add("Camera") = GameplayDefaults->Camera;
				SettingsCategories.Add("Action Log") = GameplayDefaults->ActionLog;
				SettingsCategories.Add("Difficulty") = GameplayDefaults->Difficulty;
			}
			break;
		case PD::Settings::ERTSOSettingsGroups::Video:
			{
				PD::Settings::FVideoSettingsDefaults* VideoDefaults =
					static_cast<PD::Settings::FVideoSettingsDefaults*>(SettingsDefaultsBase);
				SettingsCategories.Add("Display") = VideoDefaults->Display;
				SettingsCategories.Add("Effects") = VideoDefaults->Effects;
				SettingsCategories.Add("Graphics") = VideoDefaults->Graphics;
			}
			break;
		case PD::Settings::ERTSOSettingsGroups::Audio:
			{
				SettingsCategories.Add("Base") = static_cast<PD::Settings::FAudioSettingsDefaults*>(SettingsDefaultsBase)->Base;
			}
			break;
		case PD::Settings::ERTSOSettingsGroups::Controls:
			{
				PD::Settings::FControlsSettingsDefaults* ControlsDefaults =
					static_cast<PD::Settings::FControlsSettingsDefaults*>(SettingsDefaultsBase);
				SettingsCategories.Add("Game") = ControlsDefaults->Game;
				SettingsCategories.Add("UI") = ControlsDefaults->UI;
			}
			break;
		case PD::Settings::ERTSOSettingsGroups::Interface:
			{
				SettingsCategories.Add("Base") = static_cast<PD::Settings::FInterfaceSettingsDefaults*>(SettingsDefaultsBase)->Base;
			}
			break;
		case PD::Settings::ERTSOSettingsGroups::Num:
			continue;
		}


		URTSOMenuWidget_BaseSubmenu* Submenu = CreateWidget<URTSOMenuWidget_BaseSubmenu>(this, SubMenuClass);
		for (auto&[SettingsCategoryName, CategoryData] : SettingsCategories)
		{
			URTSOMenuWidget_SettingsCategory* SettingsCategory = CreateWidget<URTSOMenuWidget_SettingsCategory>(this, SettingsCategoryClass);
			SettingsCategory->Refresh(SettingsEntryClass, SettingsCategoryName, CategoryData);
			Submenu->ContentBox->AddChildToVerticalBox(SettingsCategory);
		}
		Submenu->SetVisibility(ESlateVisibility::Collapsed);

		UPDGenericButton* SubmenuCategoryButton = CreateWidget<UPDGenericButton>(this, TabButtonClass);
		const FName TabName = PD::Settings::AllSettingsDefaults.GetTabName(TopLevelSetting);
		SubmenuCategoryButton->TextBlock->SetText(FText::FromString(TabName.ToString()));
		SubmenuCategoryButton->ChildWidget = Submenu;

		SubmenuCategoryButton->OnClicked().AddLambda(
			[&]()
			{
				OnCategoryPressed(TabName);
			}); 

		SubmenuSelector->AddChildToHorizontalBox(SubmenuCategoryButton);
		if (SubmenuSelector->GetChildrenCount() == DefaultViewIdx + 1)
		{
			OnCategoryPressed(TabName); // Select the first 
		}
	}

}

void URTSOMenuWidget::ClearDelegates() const
{
	ResumeButton->Hitbox->OnPressed.Clear();
	ResumeButton->Hitbox->OnReleased.Clear();

	SettingsButton->Hitbox->OnPressed.Clear();
	SettingsButton->Hitbox->OnReleased.Clear();

	SaveButton->Hitbox->OnPressed.Clear();
	SaveButton->Hitbox->OnReleased.Clear();

	LoadButton->Hitbox->OnPressed.Clear();
	LoadButton->Hitbox->OnReleased.Clear();

	QuitButton->Hitbox->OnPressed.Clear();
	QuitButton->Hitbox->OnReleased.Clear();

	SaveEditor->Hitbox->OnPressed.Clear();
	SaveEditor->Hitbox->OnReleased.Clear();
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
