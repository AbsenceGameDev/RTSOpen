/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "PDRTSBaseSubsystem.h"
#include "SRTSOMiniMap.generated.h"

/** @brief  Minimap data-asset, for configuring certain assets being used by the minimap*/
UCLASS()
class RTSOPEN_API URTSOMinimapData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** @brief  Arrow texture to be applied for the player in the minimap */
	UPROPERTY(EditAnywhere, Category = "Minimap")
	UTexture2D* ArrowTexture = nullptr;
};

/** @brief  Minimap developer settings, for unified access to the configurable assets being used by the minimap */
UCLASS(Config = "Game", DefaultConfig)
class RTSOPEN_API URTSOMinimapDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	URTSOMinimapDeveloperSettings(){}
	
	/** @brief Default Minimap data */
	UPROPERTY(Config, EditAnywhere, Category = "Minimap")
	TSoftObjectPtr<URTSOMinimapData> DefaultMinimapData;
	
};

/** @brief  Minimap Slate widget, Select between radar-style @todo representative style + manual material or texture override */
class SRTSOMiniMap : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SRTSOMiniMap) {}
		SLATE_ARGUMENT(double, RadarSize)
		SLATE_ARGUMENT(double, RadarDistanceScale)
		SLATE_ARGUMENT(double, GenericMinimapIconRectSize)
		SLATE_ARGUMENT(double, FixedTextureScale)
		SLATE_ARGUMENT(FVector2D, RadarStartLocation)
		SLATE_ARGUMENT(FColor, EnemyColour)
		SLATE_ARGUMENT(FColor, FriendColour)
		SLATE_ARGUMENT(FColor, OwnedUnitsColour)
		SLATE_ARGUMENT(FColor, InteractableColour)
		SLATE_ARGUMENT(FColor, MissionColour)
		SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
		SLATE_ARGUMENT(EVerticalAlignment, VAlign)
		SLATE_ARGUMENT(URTSOMinimapData*, MiniMapData)
	SLATE_END_ARGS()

	/** @brief  Constructs the widget and caches relevant data */
	void Construct(const FArguments& InArgs);
	
	/** @brief  Ticks the radar/minimap */
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	/** @brief  Paints the radar/minimap visuals */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** @brief  Draw all nearby actors on the minimap*/
	void PaintRadarMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	
	/** @brief  Draw all nearby actors on the minimap*/
	void PaintActorsOnMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;

	/** @brief  Draw nearby entities on the minimap */
	void PaintEntitiesOnMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;	
	
	/** @brief  Draw self/local player on minimap*/
	void PaintOwnerOnMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const;
	
	/** @brief  Draws the actual radar, defines the visuals of it*/
	void PaintRadar(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, FLinearColor Color = FLinearColor::Yellow) const;

	/** @brief  Returns center screen location where the radar will be displayed*/
	FVector2D GetRadarCenter() const;

	/** @brief  Player world to screen transformation*/
	FVector2D WorldToScreen2D(FLEntityCompound& EntityCompound, APawn* OwnerPawn) const;
	FVector2D WorldToScreen2D(const AActor* ActorToPlace, APawn* OwnerPawn) const;
	FVector2D WorldToScreen2D(FVector& WorldLocation) const;

	/** @brief  Creates a new slate brush on the fly*/
	const TSharedPtr<FSlateBrush> MakeRadarBGBrush(double UniformBoxSize) const;


public:
	/** @brief  Cached pointer to the RTS subsystem */
	UPDRTSBaseSubsystem* RTSSubSystem = nullptr;
	/** @brief  Start screen location of the minimap (radar) */
	FVector2D RadarStartLocation = FVector2D(10.f, 10.f);
	/** @brief  Pixel (uniform) size for the radar  */
	double RadarSize                  = 200.0;
	/** @brief  Zoom scaling for the radar */
	double RadarDistanceScale         = 25.0;
	/** @brief  Height for the overlap ellipsoid  */
	double SphereHeight               = 300.0;
	/** @brief  Radius for the overlap ellipsoid  */
	double RadarTraceSphereRadius     = 4000.0;
	/** @brief  Size (uniform)  of generic minimap icons */
	double GenericMinimapIconRectSize = 3.0;
	/** @brief  Colour for mission related actors  */
	FColor MissionColour      {1,1,0};
	/** @brief  Colour for interactable actors  */
	FColor InteractableColour {0,1,1};
	/** @brief  Colour for enemy actors and units  */
	FColor EnemyColour        {1,0,0};
	/** @brief  Colour for owned actors and units  */
	FColor OwnedUnitsColour   {0,1,0};		
	/** @brief  Colour for friend actors and units  */
	FColor FriendColour       {0,0,1};

	/** @brief Cached minimap data pointer to retrieve some textures and possibly materials from */
	URTSOMinimapData* MiniMapData = nullptr;
	/** @brief Fixed scaling for used textures */
	const double FixedTextureScale = 0.02;
	
	/** @brief Found world actors with our actor trace */
	TArray<AActor*> OnWorldActors;
	/** @brief Hit results for our traces */
	TArray<FHitResult*> Results;
	/** @brief  Radar Alpha/Opacity */
	const double Alpha = 1.0;

	/** @brief  Slate instance data to pass inners into certain slate functions */
	struct SlateInstanceData
	{
		const TSharedPtr<FSlateBrush> ConstructedBackgroundBrush = nullptr;
		const TSharedPtr<FSlateBrush> OwnerIconBrush = nullptr;
		const TSharedPtr<FSlateBrush> GenericIconBrush = nullptr;
	};

	/** @brief  Current radar slate instance data */
	SlateInstanceData InstanceData;

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
