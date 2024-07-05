﻿/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "SaveEditor/SRTSOSaveEditor.h"

#include "Widgets/SBoxPanel.h"
#include "Textures/SlateIcon.h"
#include "Framework/Commands/UIAction.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SSplitter.h"
#include "Styling/AppStyle.h"
#include "GameplayTagContainer.h"

#define LOCTEXT_NAMESPACE "SRTSOSaveEditor"

/** @brief SRTSOSaveEditorSubmenu */

//
// SAVE EDITOR MAIN
void SRTSOSaveEditor::Construct(const FArguments& InArgs)
{
	FGameplayTag Submenu{};
	int32 MenuItem = INDEX_NONE;
	int32 NewValue = INDEX_NONE;
	
	ChildSlot
	[
		SNew(SSplitter)
		+ SSplitter::Slot()
		[
			SNew(SRTSOSaveEditorSubmenu)
				.SubmenuTag(FGameplayTag::EmptyTag)
				.OnValueChange(SRTSOSaveEditorSubmenu::FOnValueChange::CreateLambda(
					[&, Submenu, MenuItem, NewValue] ()
					{
						HandleValueChange(Submenu, MenuItem, NewValue);
					}))
		] 
		+ SSplitter::Slot()
		[
			SNew(SRTSOSaveEditorSubmenu)
				.SubmenuTag(FGameplayTag::EmptyTag)
				.OnValueChange(SRTSOSaveEditorSubmenu::FOnValueChange::CreateLambda(
					[&, Submenu, MenuItem, NewValue] ()
					{
						HandleValueChange(Submenu, MenuItem, NewValue);
					}))
		]
		+ SSplitter::Slot()
		[
			SNew(SRTSOSaveEditorSubmenu)
				.SubmenuTag(FGameplayTag::EmptyTag)
				.OnValueChange(SRTSOSaveEditorSubmenu::FOnValueChange::CreateLambda(
					[&, Submenu, MenuItem, NewValue] ()
					{
						HandleValueChange(Submenu, MenuItem, NewValue);
					}))
		]
	];
}

void SRTSOSaveEditor::HandleValueChange(FGameplayTag Submenu, int32 MenuItem, int32 NewValue) const
{
	// @todo Write impl
}


//
// SAVE EDITOR SUB-MENU
void SRTSOSaveEditorSubmenu::Construct(const FArguments& InArgs)
{
	SubmenuTag        = InArgs._SubmenuTag;
	OnValueChange    = InArgs._OnValueChange;

	ChildSlot
	[
		SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
					.HAlign(HAlign_Left)
					.AutoHeight()
					.Padding(0, 2)
				[
					MakeSubmenu()
				]
			]
		]
	];
}

TSharedRef<SWidget> SRTSOSaveEditorSubmenu::MakeSubmenu()
{
	TSharedPtr<SButton> EditButton;
	SAssignNew(EditButton, SButton)
		.ButtonStyle(FAppStyle::Get(), "NoBorder")
		// .OnClicked(FOnClicked::CreateSP(this, &SRTSOSaveEditorSubmenu::))
		.ContentPadding(0)
		.ToolTipText(NSLOCTEXT("SaveEditor", "SaveEditor_ModifyElem", "Modify the selected item"))
	[
		SNew(SImage)
			.Image(FAppStyle::GetBrush(TEXT("Icons.Search")))
	];

	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1, 0)
		.VAlign(VAlign_Center)
	[
		SNew(SButton)
			.ButtonStyle(FAppStyle::Get(), "NoBorder")
			.OnClicked(FOnClicked::CreateSP(this, &SRTSOSaveEditorSubmenu::OnUseSelectedItemClick))
			.ContentPadding(1.f)
			.ToolTipText(NSLOCTEXT("SaveEditor", "SaveEditor_SelectItem", "Selected an item in the editor"))
		[
			SNew(SImage)
				.Image(FAppStyle::GetBrush(TEXT("Icons.CircleArrowLeft")))
		]
	]
	+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(1, 0)
		.VAlign(VAlign_Center)
	[
		// @todo Write actual impl
		EditButton.ToSharedRef()
	];
}

FText SRTSOSaveEditorSubmenu::GetSubmenuTextValue() const
{
	FText SubmenuTitle = LOCTEXT("SelectSubmenu", "Select a Submenu...");
	// @todo Write impl

	return SubmenuTitle;
}

FReply SRTSOSaveEditorSubmenu::OnUseSelectedItemClick()
{
	// @todo Write impl

	return FReply::Handled();
}



#undef LOCTEXT_NAMESPACE


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

