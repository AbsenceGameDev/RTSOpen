/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "PDRTSSharedUI.generated.h"

struct FPDBuildWorker;
class UImage;
class UTextBlock;
struct FPDBuildContext;

DECLARE_LOG_CATEGORY_CLASS(PDLog_RTSBaseUI, Log, All);

/** @brief Tile view object type for loading buildable data */
UCLASS(Blueprintable)
class UPDBuildContextWrapper : public UObject
{
	GENERATED_BODY()
public:
	
	/** @brief Self-explanatory */
	void AssignData(
		const FText& InBuildableTitle,
		const FGameplayTag& InContextTag,
		const TArray<FDataTableRowHandle>& InInnerBuildableTags,
		class UPDBuildWidgetBase* InDirectParentReference)
	{
		BuildContextTitle = InBuildableTitle;
		ContextTag = InContextTag;
		InnerBuildableTags = InInnerBuildableTags;
		DirectParentReference = InDirectParentReference;
	}

	/** @brief Immutable access to private member BuildContextTitle */
	UFUNCTION(BlueprintCallable)
	const FText& GetBuildContextTitle() const { return BuildContextTitle; };

	/** @brief Immutable access to private member InnerBuildableTags */
	UFUNCTION(BlueprintCallable)
	const FGameplayTag& GetContextTag() const { return ContextTag; };	
	
	/** @brief Immutable access to private member InnerBuildableTags */
	UFUNCTION(BlueprintCallable)
	const TArray<FDataTableRowHandle>& GetBuildableTags() const { return InnerBuildableTags; };	

	/** @brief Mutable access to private member ptr DirectParentReference */
	UFUNCTION(BlueprintCallable)
	class UPDBuildWidgetBase* GetDirectParentReference() const { return DirectParentReference; };

private:
	/** @brief Assigned by 'AssignData', retrieved by 'GetBuildableTitle'  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	FText BuildContextTitle{};

	/** @brief Assigned by 'AssignData', retrieved by 'GetBuildableTag'  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	FGameplayTag ContextTag{};
	
	/** @brief Assigned by 'AssignData', retrieved by 'GetBuildableTag'  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	TArray<FDataTableRowHandle> InnerBuildableTags;

	/** @brief Assigned by 'AssignData', retrieved by 'GetDirectParentReference'  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	class UPDBuildWidgetBase* DirectParentReference = nullptr;
};

/** @brief Tile view object type for loading buildable data */
UCLASS(Blueprintable)
class UPDBuildableWrapper : public UObject
{
	GENERATED_BODY()
public:
	
	/** @brief Self-explanatory */
	void AssignData(
		const FText& InBuildableTitle,
		const bool bInCanBuild,
		const FGameplayTag& InBuildableTag,
		const FGameplayTag& InParentContextTag,
		class UPDBuildWidgetBase* InDirectParentReference)
	{
		BuildableTitle = InBuildableTitle;
		BuildableTag = InBuildableTag;
		ParentContextTag = InParentContextTag;
		bCanBuild = bInCanBuild;
		DirectParentReference = InDirectParentReference;
	}

	/** @brief Immutable access to private member BuildableTitle */
	UFUNCTION(BlueprintCallable)
	const FText& GetBuildableTitle() const { return BuildableTitle; };
	
	/** @return private bCanBuild value */
	UFUNCTION(BlueprintCallable)
	bool GetCanBuild() const { return bCanBuild; };

	/** @brief Immutable access to private member BuildableTag */
	UFUNCTION(BlueprintCallable)
	const FGameplayTag& GetBuildableTag() const { return BuildableTag; };	

	/** @brief Direct access to private member DirectParentReference */
	UFUNCTION(BlueprintCallable)
	class UPDBuildWidgetBase* GetDirectParentReference() const { return DirectParentReference; };

	/** @brief Immutable access to private member ParentMenuContextTag */
	UFUNCTION(BlueprintCallable)
	const FGameplayTag& GetParentContextTag() const { return ParentMenuContextTag; };
	
private:
	/** @brief Assigned by 'AssignData', retrieved by 'GetBuildableTitle'. Is passed into a tileview or listview entry upon creation  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	FText BuildableTitle{};

	/** @brief Assigned by 'AssignData', retrieved by 'GetCanBuild''. Is passed into a tileview or listview entry upon creation   */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	bool bCanBuild = true;

	/** @brief Assigned by 'AssignData', retrieved by 'GetBuildableTag''. Is passed into a tileview or listview entry upon creation   */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	FGameplayTag BuildableTag;
	/** @brief Assigned by 'AssignData', retrieved by 'GetParentContextTag''. Is passed into a tileview or listview entry upon creation   */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	FGameplayTag ParentContextTag;
	
	/** @brief Assigned by 'AssignData', retrieved by 'GetDirectParentReference''. Is passed into a tileview or listview entry upon creation   */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	class UPDBuildWidgetBase* DirectParentReference = nullptr;

	FGameplayTag ParentMenuContextTag{};
};

UCLASS()
class PDRTSBASE_API UPDBuildableEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION() void OnPressed();
	UFUNCTION() void OnHovered();
	UFUNCTION() void OnUnhovered();
	UFUNCTION() void OnClicked();
	UFUNCTION() void OnReleased();

	/** @brief Border for the 'TextContent' text-block widget.*/
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UBorder* TextContentBorder = nullptr;	
	
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* BuildableTitle = nullptr;

	UPROPERTY(BlueprintReadWrite)
	bool bCanBuild = true;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FGameplayTag BuildableTag;

	/** @brief Hitbox, what our mouse events actually interacts with */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UButton* Hitbox = nullptr;
	
	/** @brief Assigned by 'AssignData', retrieved by 'DirectParentReference'  */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn=true, AllowPrivateAccess="true"))
	class UPDBuildWidgetBase* DirectParentReference = nullptr;
	
	FGameplayTag ParentMenuContextTag{};
};

UCLASS()
class PDRTSBASE_API UPDBuildContextEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()
public:

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	UFUNCTION() void OnPressed();
	UFUNCTION() void OnHovered();
	UFUNCTION() void OnUnhovered();
	UFUNCTION() void OnClicked();
	UFUNCTION() void OnReleased();

	/** @brief Border for the 'TextContent' text-block widget.*/
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UBorder* TextContentBorder = nullptr;	
	
	UPROPERTY(Meta = (BindWidget))
	UTextBlock* ContextTitle = nullptr;

	/** @brief Assigned by 'AssignData', retrieved by 'DirectParentReference'  */
	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn=true))
	class UPDBuildWidgetBase* DirectParentReference = nullptr;
	
	/** @brief Hitbox, what our mouse events actually interacts with */
	UPROPERTY(BlueprintReadWrite, Meta=(BindWidget))
	class UButton* Hitbox = nullptr;	
	
	FGameplayTag SelfContextTag{};
};

USTRUCT(Blueprintable)
struct FPDBuildContextParam
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/PDRTSBase.PDBuildContext"))
	TArray<FDataTableRowHandle> NewBuildableContext;
};

UCLASS()
class PDRTSBASE_API UPDBuildWidgetBase : public UUserWidget
{
	GENERATED_BODY()
public:

	virtual void NativeConstruct() override;

	/** @brief  Overwrites 'CurrentBuildableContext' */
	UFUNCTION(BlueprintCallable)
	void OverwriteBuildContext(const FPDBuildContextParam& NewRowParams);

	/** @brief  Loads 'CurrentBuildableContexts' data into the widget */
	UFUNCTION(BlueprintCallable)
	virtual void LoadBuildContexts();
	
	/** @brief  Selects the given context and loads its data into the widget, given that it is part of the CurrentBuildableContexts array */
	UFUNCTION(BlueprintCallable)
	virtual void SelectBuildContext(const FGameplayTag& NewSelectedContext);
	
	/** @return 'CurrentBuildableContext' */
	UFUNCTION(BlueprintCallable)
	virtual TArray<FDataTableRowHandle>& GetCurrentBuildContexts() { return CurrentBuildableContexts; }

	/** @return 'DefaultBuildableContext' */
	UFUNCTION(BlueprintCallable)
	virtual TArray<FDataTableRowHandle>& GetDefaultBuildContexts() { return DefaultBuildableContexts; }

	/** @return 'DefaultBuildableContext' */
	UFUNCTION(BlueprintCallable)
	virtual const FGameplayTag& GetLastSelectedContextTag() { return LastSelectedContextTag; }

	UFUNCTION(BlueprintCallable)
	void UpdateSelectedContext(const FGameplayTag& RequestToSelectTag);
	void SelectBuildable(const FGameplayTag& NewSelectedBuildable);

	UFUNCTION(BlueprintCallable)
	void UpdateSelectedBuildable(const FGameplayTag& RequestToSelectTag, const bool bSelect);

	UFUNCTION(BlueprintCallable)
	void SpawnWorkerBuildMenu(const FPDBuildWorker& BuildWorker);
	UFUNCTION(BlueprintCallable)
	void BeginCloseWorkerBuildMenu();
	UFUNCTION(BlueprintCallable)
	void EndCloseWorkerBuildMenu();

	UFUNCTION(BlueprintCallable)
	void OpenWorkerContextMenu();	
	UFUNCTION(BlueprintCallable)
	void BeginCloseWorkerContext();
	UFUNCTION(BlueprintCallable)
	void EndCloseWorkerContext();		

	/** @brief Tileview which will display our 'UPDBuildableEntry's */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	class UTileView* Buildables = nullptr;

	/** @brief Tileview which will display our contexts themselves */
	UPROPERTY(BlueprintReadWrite, Meta = (BindWidget))
	class UTileView* BuildContexts = nullptr;

	/** @brief Widget animation to play when opening the widget  */
	UPROPERTY(Transient, BlueprintReadWrite, Meta = (BindWidgetAnim))
	class UWidgetAnimation* OpenWidget = nullptr;	

	/** @brief Widget animation to play when closing the widget  */
	UPROPERTY(Transient, BlueprintReadWrite, Meta = (BindWidgetAnim))
	class UWidgetAnimation* CloseWidget = nullptr;

	/** @brief Widget animation to play when opening a widget context menus  */
	UPROPERTY(Transient, BlueprintReadWrite, Meta = (BindWidgetAnim))
	class UWidgetAnimation* OpenContextMenu = nullptr;	

	/** @brief Widget animation to play when closing one a widget context menu  */
	UPROPERTY(Transient, BlueprintReadWrite, Meta = (BindWidgetAnim))
	class UWidgetAnimation* CloseContextMenu = nullptr;			
	
	/** @brief DefaultBuildContext  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/PDRTSBase.PDBuildContext"))
	TArray<FDataTableRowHandle> DefaultBuildableContexts;

	/** @brief DefaultBuildContext  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType = "/Script/PDRTSBase.PDSharedBuildWidgetFlair"))
	FDataTableRowHandle BuildWidgetFlairHandle;
	
	/** @brief DefaultBuildContext  */
	struct FPDSharedBuildWidgetFlair* SelectedWidgetFlair = nullptr;

	/** @brief DefaultBuildContext  */
	bool bIsMenuVisible = false;

private:
	/** @brief Currently selected build context  */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Meta = (AllowPrivateAccess="true", RowType = "/Script/PDRTSBase.PDBuildContext"))
	TArray<FDataTableRowHandle> CurrentBuildableContexts;

	FGameplayTag LastSelectedContextTag{};
	FGameplayTag LastSelectedBuildableTag{};
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
