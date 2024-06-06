/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "PDRTSSharedUI.h"

#include "PDRTSCommon.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/TileView.h"

void UPDBuildableEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UPDBuildableWrapper* Item = Cast<UPDBuildableWrapper>(ListItemObject);
	if (Item == nullptr) { return; }
	
	// 1. Read data
	BuildableTitle->SetText(Item->GetBuildableTitle());
	bCanBuild             = Item->GetCanBuild();
	DirectParentReference = Item->GetDirectParentReference();
	ParentMenuContextTag  = Item->GetParentContextTag();

	// 2. Set data-asset variable
	TArray<FDataTableRowHandle>& BuildContexts = DirectParentReference->GetCurrentBuildContexts();
	for (const FDataTableRowHandle& Context : BuildContexts)
	{
		const FString CtxtStr = FString::Printf(TEXT("UPDBuildWidgetBase(%s, %s)::SelectBuildContext"), *Context.RowName.ToString(), *GetName());
		const FPDBuildContext* LoadedContext = Context.GetRow<FPDBuildContext>(CtxtStr);
		if (LoadedContext == nullptr || LoadedContext->ContextTag.IsValid() == false || LoadedContext->ContextTag != ParentMenuContextTag)
		{
			/** @todo Output to log with warning or error level verbosity if the context or the tag was invalid */ 
			continue;
		}

		const UPDBuildableDataAsset* BuildDataAsset = LoadedContext->BuildablesData.FindRef(Item->GetBuildableTag()).DABuildAsset;
		if (BuildDataAsset != nullptr)
		{
			UMaterialInstance* MatInst = BuildDataAsset->Buildable_MaterialInstance;
			UTexture2D* Tex = BuildDataAsset->Buildable_Texture;
			if (MatInst != nullptr)
			{
				TextContentBorder->SetBrushFromMaterial(MatInst);
			}
			else if (Tex != nullptr) // Fallback to texture
			{
				TextContentBorder->SetBrushFromTexture(Tex);
			}
		}
	}
	
	
	// 3. Bind delegates
	TextContentBorder->OnMouseMoveEvent.BindDynamic(this, &UPDBuildableEntry::MouseMove); 
	TextContentBorder->OnMouseButtonDownEvent.BindDynamic(this, &UPDBuildableEntry::MouseButtonDown);
	TextContentBorder->OnMouseButtonUpEvent.BindDynamic(this, &UPDBuildableEntry::MouseButtonUp);
	TextContentBorder->OnMouseDoubleClickEvent.BindDynamic(this, &UPDBuildableEntry::MouseDoubleClick);
}

FEventReply UPDBuildableEntry::MouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	return FEventReply();/** @todo: write impl.*/
}

FEventReply UPDBuildableEntry::MouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	return FEventReply();/** @todo: write impl.*/
}

FEventReply UPDBuildableEntry::MouseButtonUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	return FEventReply();/** @todo: write impl.*/
}

FEventReply UPDBuildableEntry::MouseDoubleClick(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	return FEventReply();/** @todo: write impl.*/
}

void UPDBuildContextEntry::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	const UPDBuildContextWrapper* Item = Cast<UPDBuildContextWrapper>(ListItemObject);
	if (Item == nullptr) { return; }
	
	// 1. Read data
	ContextTitle->SetText(Item->GetBuildContextTitle());
	SelfContextTag        = Item->GetContextTag();
	DirectParentReference = Item->GetDirectParentReference();

	// 2. Set data-asset variable
	TArray<FDataTableRowHandle>& BuildContexts = DirectParentReference->GetCurrentBuildContexts();
	for (const FDataTableRowHandle& Context : BuildContexts)
	{
		const FString CtxtStr = FString::Printf(TEXT("UPDBuildWidgetBase(%s, %s)::SelectBuildContext"), *Context.RowName.ToString(), *GetName());
		const FPDBuildContext* LoadedContext = Context.GetRow<FPDBuildContext>(CtxtStr);
		if (LoadedContext == nullptr || LoadedContext->ContextTag.IsValid() == false)
		{
			/** @todo Output to log with warning or error level verbosity if the context or the tag was invalid */ 
			continue;
		}

		const UPDBuildContextDataAsset* BuildDataAsset = LoadedContext->ContextData.DABuildContextAsset;
		if (BuildDataAsset != nullptr)
		{
			UMaterialInstance* MatInst = BuildDataAsset->BuildContext_MaterialInstance;
			UTexture2D* Tex = BuildDataAsset->BuildContext_Texture;
			if (MatInst != nullptr)
			{
				TextContentBorder->SetBrushFromMaterial(MatInst);
			}
			else if (Tex != nullptr) // Fallback to texture
			{
				TextContentBorder->SetBrushFromTexture(Tex);
			}
		}
	}
	
	
	// 3. Bind delegates
	TextContentBorder->OnMouseMoveEvent.BindDynamic(this, &UPDBuildContextEntry::MouseMove); 
	TextContentBorder->OnMouseButtonDownEvent.BindDynamic(this, &UPDBuildContextEntry::MouseButtonDown);
	TextContentBorder->OnMouseButtonUpEvent.BindDynamic(this, &UPDBuildContextEntry::MouseButtonUp);
	TextContentBorder->OnMouseDoubleClickEvent.BindDynamic(this, &UPDBuildContextEntry::MouseDoubleClick);	
}

FEventReply UPDBuildContextEntry::MouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	return FEventReply();/** @todo: write impl.*/
}

FEventReply UPDBuildContextEntry::MouseButtonDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	ensure(DirectParentReference != nullptr);

	if (DirectParentReference->SelectedWidgetFlair == nullptr)
	{
		/** @todo: log about widget falir not being set.*/
	}

	// tell DirectParentReference about the cliked button 
	DirectParentReference->UpdateSelectedContext(SelfContextTag);
	
	return FEventReply();/** @todo: write impl.*/
}

FEventReply UPDBuildContextEntry::MouseButtonUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	return FEventReply();/** @todo: write impl.*/
}

FEventReply UPDBuildContextEntry::MouseDoubleClick(FGeometry MyGeometry, const FPointerEvent& MouseEvent)
{
	return FEventReply();/** @todo: write impl.*/
}

void UPDBuildWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	CurrentBuildableContexts = DefaultBuildableContexts;
	
	SelectedWidgetFlair = BuildWidgetFlairHandle.GetRow<FPDSharedBuildWidgetFlair>("");
}

void UPDBuildWidgetBase::OverwriteBuildContext(const FPDBuildContextParam& NewRowParams)
{
	CurrentBuildableContexts = NewRowParams.NewBuildableContext;
}

void UPDBuildWidgetBase::LoadBuildContexts()
{
	// Build the context menu entries (Category)
	BuildContexts->ClearListItems();

	FGameplayTag SelectedContextTag = FGameplayTag::EmptyTag;
	bool bLastSelectedContextStillValid = false;
	for (const FDataTableRowHandle& Context : CurrentBuildableContexts)
	{
		const FString CtxtStr = FString::Printf(TEXT("UPDBuildWidgetBase(%s, %s)::SelectBuildContext"), *Context.RowName.ToString(), *GetName());
		FPDBuildContext* LoadedContext = Context.GetRow<FPDBuildContext>(CtxtStr);
		if (LoadedContext == nullptr || LoadedContext->ContextTag.IsValid() == false)
		{
			/** @todo Output to log with warning or error level verbosity if the context or the tag was invalid */ 
			continue;
		}
		
		UPDBuildContextWrapper* DataWrapper = NewObject<UPDBuildContextWrapper>(this, UPDBuildContextWrapper::StaticClass());		

		FText SelectedName = LoadedContext->ContextReadableName.IsEmpty() ?
			FText::FromName(LoadedContext->ContextTag.GetTagName()) : LoadedContext->ContextReadableName;

		TSet<FGameplayTag> InnerBuildableTags;
		LoadedContext->BuildablesData.GetKeys(InnerBuildableTags); 
		
		DataWrapper->AssignData(SelectedName, LoadedContext->ContextTag, InnerBuildableTags, this);
		BuildContexts->AddItem(DataWrapper);
		
		SelectedContextTag = SelectedContextTag.IsValid() == false ? LoadedContext->ContextTag : SelectedContextTag;
		bLastSelectedContextStillValid &= LastSelectedContextTag == LoadedContext->ContextTag;
	}
	
	SelectedContextTag = bLastSelectedContextStillValid ? LastSelectedContextTag : SelectedContextTag;
	SelectBuildContext(SelectedContextTag);
}

void UPDBuildWidgetBase::SelectBuildContext(const FGameplayTag& NewSelectedContext)
{
	bool bRequestedContextWasValid = false;
	for (const FDataTableRowHandle& Context : CurrentBuildableContexts)
	{
		const FString CtxtStr = FString::Printf(TEXT("UPDBuildWidgetBase(%s, %s)::SelectBuildContext"), *Context.RowName.ToString(), *GetName());
		FPDBuildContext* LoadedContext = Context.GetRow<FPDBuildContext>(CtxtStr);
		if (LoadedContext == nullptr || LoadedContext->ContextTag != NewSelectedContext) { continue; }

		bRequestedContextWasValid = true;
		
		Buildables->ClearListItems();
		for (const TTuple<FGameplayTag, FPDBuildableData>& BuildData : LoadedContext->BuildablesData)
		{
			UPDBuildableWrapper* DataWrapper = NewObject<UPDBuildableWrapper>(this, UPDBuildableWrapper::StaticClass());		

			FGameplayTag BuildableTag = BuildData.Key;
			const FPDBuildableData& DABuild = BuildData.Value;

			constexpr bool bCanBuild = true; // @todo extrapolate bCanBuild based on available resources and such, possibly control via a virtual function and define in game module
			DataWrapper->AssignData(DABuild.ReadableName, bCanBuild, BuildableTag, LoadedContext->ContextTag, this);
			Buildables->AddItem(DataWrapper);
		}
		break;
	}
	
	LastSelectedContextTag = bRequestedContextWasValid ? NewSelectedContext : LastSelectedContextTag;
}

void UPDBuildWidgetBase::UpdateSelectedContext(const FGameplayTag& RequestToSelectTag)
{

	// Is it already selected?

	for (const FDataTableRowHandle& Context : CurrentBuildableContexts)
	{
		const FString CtxtStr = FString::Printf(TEXT("UPDBuildWidgetBase(%s, %s)::SelectBuildContext"), *Context.RowName.ToString(), *GetName());
		const FPDBuildContext* LoadedContext = Context.GetRow<FPDBuildContext>(CtxtStr);
		if (LoadedContext == nullptr || LoadedContext->ContextTag.IsValid() == false)
		{
			/** @todo Output to log with warning or error level verbosity if the context or the tag was invalid */ 
			continue;
		}
		
		// Resets any previous selected tints
		const TArray<UObject*>& ListItems = BuildContexts->GetListItems();
		for (UObject* const& Item : ListItems)
		{
			const UPDBuildContextEntry* AsContextEntry = BuildContexts->GetEntryWidgetFromItem<UPDBuildContextEntry>(Item);
			if (AsContextEntry == nullptr || AsContextEntry->IsValidLowLevelFast() == false)
			{
				continue;
			}

			const bool bShouldSelect = (RequestToSelectTag == AsContextEntry->SelfContextTag);
			FSlateBrush& Brush = AsContextEntry->TextContentBorder->Background;
			Brush.TintColor = bShouldSelect ? FSlateColor(SelectedWidgetFlair->SelectedContextTint) : FSlateColor(SelectedWidgetFlair->NotSelectedContextTint);
			AsContextEntry->TextContentBorder->SetBrush(Brush);	
		}
	}
	
	SelectBuildContext(RequestToSelectTag);
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
