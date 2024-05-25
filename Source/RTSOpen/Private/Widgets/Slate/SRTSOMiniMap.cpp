/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */
#include "Widgets/Slate/SRTSOMiniMap.h"

#include "PDRTSBaseSubsystem.h"
#include "Actors/GodHandPawn.h"
#include "Actors/RTSOController.h"
#include "AI/Mass/RTSOMassFragments.h"
#include "Chaos/DebugDrawQueue.h"
#include "ImageUtils.h"
#include "Kismet/KismetSystemLibrary.h"

void SRTSOMiniMap::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
	
	// Update positions?

	// todo move outside of the tick, some analogue to OnConstruction for slate widgets if there are any
	RTSSubSystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();	
}

int32 SRTSOMiniMap::OnPaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const int32 SuperResult =SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	// Update positions. Call 'PaintRadarMiniMap'
	PaintRadarMiniMap(OutDrawElements, AllottedGeometry, LayerId);
	
	return SuperResult;
}


// Paint functions
void SRTSOMiniMap::PaintRadarMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{

	PaintRadar(OutDrawElements, AllottedGeometry, LayerId ,FLinearColor::Gray);
	PaintOwnerOnMiniMap(OutDrawElements, AllottedGeometry, LayerId);
	PaintActorsOnMiniMap(OutDrawElements, AllottedGeometry, LayerId);
	PaintEntitiesOnMiniMap(OutDrawElements, AllottedGeometry, LayerId);
}

// Move overlap logic out of the actual slate class, perform before of after the onpaint function
void SRTSOMiniMap::PaintActorsOnMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	TArray<FLEntityCompound>* MinimapEntityBuffer = RTSSubSystem->OctreeUserQuery.CurrentBuffer.Find(UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_MINIMAP);
	if (MinimapEntityBuffer == nullptr) { return; }
	
	APawn* OwnerPawn = Cast<APawn>(RTSSubSystem->OctreeUserQuery.CallingUser);
	if (OwnerPawn == nullptr) { return; }

	TArray<AActor*>& MutableWorldActors = const_cast<TArray<AActor*>&>(OnWorldActors);
	
	MutableWorldActors.Empty();
	UKismetSystemLibrary::SphereOverlapActors(
		OwnerPawn,
		OwnerPawn->GetActorLocation(),
		RadarTraceSphereRadius,
		{UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldStatic)},
		nullptr, 
		{OwnerPawn},
		MutableWorldActors);
	
	const FVector2D RadarCenter = GetRadarCenter();
	const FLinearColor EnemyLinearColour = FLinearColor(EnemyColour.R, EnemyColour.G, EnemyColour.B, 0.0);
	const FLinearColor GenericActorLinearColour = FColor::Silver.ReinterpretAsLinear();
	FLinearColor OffsetAlpha(0,0,0, 0);
	
	const FGeometry DerivedGeometry = AllottedGeometry;
	TSharedPtr<FSlateBrush> MutableGenericIconBrush = InstanceData.GenericIconBrush;
	UE::Slate::FDeprecateVector2DParameter ImageSize{GenericMinimapIconRectSize};
	MutableGenericIconBrush->SetImageSize(ImageSize);	
	
	for (const AActor* It : OnWorldActors)
	{
		const FVector2D Location2D = WorldToScreen2D(It, OwnerPawn);

		//Clamp positions in between the minimap size so they are not out of bounds
		const float NewX = FMath::Clamp<float>(Location2D.X, -RadarSize / 2, RadarSize / 2 - GenericMinimapIconRectSize);
		const float NewY = FMath::Clamp<float>(Location2D.Y, -RadarSize / 2, RadarSize / 2 - GenericMinimapIconRectSize);

		FVector OwnerLocation = OwnerPawn->GetActorLocation();
		FVector EnemyLocation = It->GetActorLocation();

		const float Distance = FVector::Distance(OwnerLocation, EnemyLocation);

		// // @todo Decide what type of actors this is, if enemy use 'EnemyColour' and so forth
		// Contexts to consider:: Mission / Interactable :  {MissionColour; InteractableColour}
		// Relation to consider:: Enemy / OwnedUnits / Friend : {EnemyColour; OwnedUnitsColour; FriendColour;}

		const FLinearColor EntityColour = Cast<APawn>(It) != nullptr ? EnemyLinearColour: GenericActorLinearColour; 
		OffsetAlpha.A = Alpha / Distance * 4;

		// Paint box element
		const FSlateRenderTransform RT{{1.0},{static_cast<float>(GetRadarCenter().X + NewX), static_cast<float>(GetRadarCenter().Y + NewY)}};
		DerivedGeometry.ToPaintGeometry().SetRenderTransform(RT);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			DerivedGeometry.ToPaintGeometry(),
			InstanceData.ConstructedBackgroundBrush.Get(),
			ESlateDrawEffect::None,
			EntityColour + OffsetAlpha);
	}
}

// @todo move calculations out of the hud, keep cached data the HUD can quickly access during the draw call wittout having to run the octree iteration themselves
void SRTSOMiniMap::PaintEntitiesOnMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	TArray<FLEntityCompound>* MinimapEntityBuffer = RTSSubSystem->OctreeUserQuery.CurrentBuffer.Find(UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_MINIMAP);
	if (MinimapEntityBuffer == nullptr) { return; }
	
	APawn* OwnerPawn = Cast<APawn>(RTSSubSystem->OctreeUserQuery.CallingUser);
	if (OwnerPawn == nullptr) { return; }
	
	// @todo consider caching
	ARTSOController* RTSOController = OwnerPawn->GetController<ARTSOController>();
	if (RTSOController == nullptr || RTSOController->GetPawn() == nullptr) { return; }
	
	// Viewport halfsize
#if CHAOS_DEBUG_DRAW
	{
		FVector BoundsCenter = RTSSubSystem->OctreeUserQuery.QueryArchetypes.Find(UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_MINIMAP)->Location;
		FVector Extent = RTSSubSystem->OctreeUserQuery.QueryArchetypes.Find(UPDRTSBaseSubsystem::EPDQueryGroups::QUERY_GROUP_MINIMAP)->QuerySizes;
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugBox(BoundsCenter, Extent, FQuat::Identity, FColor::Silver, false, 0, 0, 10.0f);
		const FVector& TextLocation = BoundsCenter + FVector(0, 0, Extent.Z * 2);
		Chaos::FDebugDrawQueue::GetInstance().DrawDebugString(TextLocation,FString("Radar Octree Trace"), nullptr, FColor::Yellow, 0, true, 2);
	}
#endif

	const FLinearColor EnemyLinearColour = FLinearColor(EnemyColour.R, EnemyColour.G, EnemyColour.B, 0.0);
	const FLinearColor OwnerLinearColour = FLinearColor(OwnedUnitsColour.R, OwnedUnitsColour.G, OwnedUnitsColour.B, 0.0);
	FLinearColor OffsetAlpha(0,0,0, 0);

	// Copy some values we'll keep calling for the following loop
	const FVector PlayerLocation = OwnerPawn->GetActorLocation();
	const FVector2D RadarCenter  = GetRadarCenter();
	const int32 CallerID         = RTSOController->GetActorID();


	const FGeometry DerivedGeometry = AllottedGeometry;
	TSharedPtr<FSlateBrush> MutableGenericIconBrush = InstanceData.GenericIconBrush;
	UE::Slate::FDeprecateVector2DParameter ImageSize{GenericMinimapIconRectSize};
	MutableGenericIconBrush->SetImageSize(ImageSize);	
	
	// Iterate all found entities and draw boxes on screen to display them
	for (FLEntityCompound& EntityCompound : *MinimapEntityBuffer)
	{
		FVector EntityLocation = EntityCompound.Location;
		const FVector2D Location2D = WorldToScreen2D(EntityCompound, OwnerPawn);

		//Clamp positions in between the minimap size so they are not out of bounds
		const float NewX = FMath::Clamp<float>(Location2D.X, -RadarSize / 2, RadarSize / 2 - GenericMinimapIconRectSize);
		const float NewY = FMath::Clamp<float>(Location2D.Y, -RadarSize / 2, RadarSize / 2 - GenericMinimapIconRectSize);

		const float Distance = FVector::Distance(PlayerLocation, EntityLocation);
		
		//// @todo colour based on selection ID and/or owner ID, currently only colours green if owning and red otherwise, with no further disctions being made
		// Contexts to consider:: Mission / Interactable :  {MissionColour; InteractableColour}
		// Relation to consider:: Enemy / OwnedUnits / Friend : {EnemyColour; OwnedUnitsColour; FriendColour;}
		
		const FLinearColor EntityColour = CallerID != EntityCompound.OwnerID ? EnemyLinearColour: OwnerLinearColour; 
		OffsetAlpha.A = Alpha / Distance * 4;

		// Paint box element
		const FSlateRenderTransform RT{{1.0},{static_cast<float>(RadarCenter.X + NewX), static_cast<float>(RadarCenter.Y + NewY)}};
		DerivedGeometry.ToPaintGeometry().SetRenderTransform(RT);
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			DerivedGeometry.ToPaintGeometry(),
			InstanceData.ConstructedBackgroundBrush.Get(),
			ESlateDrawEffect::None,
			EntityColour + OffsetAlpha);
	}
}

void SRTSOMiniMap::PaintOwnerOnMiniMap(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId) const
{
	const FGeometry DerivedGeometry = AllottedGeometry;
	const FSlateRenderTransform RT{{1.0},{static_cast<float>(GetRadarCenter().X), static_cast<float>(GetRadarCenter().Y)}};
	DerivedGeometry.ToPaintGeometry().SetRenderTransform(RT);
	
	//Clamp positions in between the minimap size so they are not out of bounds
	if (MiniMapData == nullptr || MiniMapData->ArrowTexture == nullptr)
	{
		// Draw Background Colour @todo replace with background material 
		static_cast<TSharedPtr<FSlateBrush>>(InstanceData.ConstructedBackgroundBrush) = MakeRadarBGBrush(GenericMinimapIconRectSize);

		// Paint box element		
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			DerivedGeometry.ToPaintGeometry(),
			InstanceData.ConstructedBackgroundBrush.Get(),
			ESlateDrawEffect::None,
			FLinearColor::White);
		return;
	}

	const float TextureWidth = MiniMapData->ArrowTexture->GetSurfaceWidth();
	const float TextureHeight = MiniMapData->ArrowTexture->GetSurfaceHeight();

	// Draw Background Colour @todo replace with background material
	TSharedPtr<FSlateBrush> MutableOwnerIconBrush = InstanceData.OwnerIconBrush;

	UE::Slate::FDeprecateVector2DParameter ImageSize{TextureWidth,TextureHeight};
	MutableOwnerIconBrush->SetImageSize(ImageSize);
	MutableOwnerIconBrush->ImageType = ESlateBrushImageType::FullColor;
	MutableOwnerIconBrush->SetResourceObject(MiniMapData->ArrowTexture);
			
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		DerivedGeometry.ToPaintGeometry(),
		InstanceData.OwnerIconBrush.Get(),
		ESlateDrawEffect::None,
		FLinearColor::White);	
}

void SRTSOMiniMap::PaintRadar(FSlateWindowElementList& OutDrawElements, const FGeometry& AllottedGeometry, int32 LayerId, FLinearColor Color) const
{
	const double PosX = RadarStartLocation.X;
	const double PosY = RadarStartLocation.Y;
	const double HalfSize = RadarSize * 0.5;

	// Outer box screen coordinated
	TArray<FVector2D> Axes;
	Axes.Add(FVector2D{PosX - HalfSize, PosY + HalfSize}); // Fwd Left 
	Axes.Add(FVector2D{PosX + HalfSize, PosY + HalfSize}); // Fwd Right
	Axes.Add(FVector2D{PosX - HalfSize, PosY - HalfSize}); // Bwd Left
	Axes.Add(FVector2D{PosX + HalfSize, PosY - HalfSize}); // Bwd Right
	
	// Actually Draw Outer Box
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		Axes,
		ESlateDrawEffect::None,
		FLinearColor(1,1,0,.5),
		true,
		2.0
		);

	// Draw Horizontal Radar Line
	TArray<FVector2D> RadarHLine;
	RadarHLine.Emplace(FVector2D{PosX + HalfSize, PosY});
	RadarHLine.Emplace(FVector2D{PosX - HalfSize, PosY});
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		RadarHLine,
		ESlateDrawEffect::None,
		FLinearColor(1,1,0,.5),
		true,
		2.0
		);
	
	// Draw Vertical Radar Line
	TArray<FVector2D> RadarVLine;
	RadarVLine.Emplace(FVector2D{PosX + HalfSize, PosY});
	RadarVLine.Emplace(FVector2D{PosX - HalfSize, PosY});
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		RadarVLine,
		ESlateDrawEffect::None,
		FLinearColor(1,1,0,.5),
		true,
		2.0
		);
	
	// Draw Background Colour @todo replace with background material 
	static_cast<TSharedPtr<FSlateBrush>>(InstanceData.ConstructedBackgroundBrush) = MakeRadarBGBrush(RadarSize);
	
	FSlateDrawElement::MakeBox(
		OutDrawElements,
		LayerId,
		AllottedGeometry.ToPaintGeometry(),
		InstanceData.ConstructedBackgroundBrush.Get(),
		ESlateDrawEffect::None,
		FLinearColor(0,0,0,0.3));
}

FVector2D SRTSOMiniMap::GetRadarCenter() const
{
	return FVector2D(RadarStartLocation.X + RadarSize / 2, RadarStartLocation.Y + RadarSize / 2);
}

FVector2D SRTSOMiniMap::WorldToScreen2D(FLEntityCompound& EntityCompound, APawn* OwnerPawn) const
{
	if (OwnerPawn == nullptr || EntityCompound.EntityHandle.IsValid() == false)
	{
		return FVector2D(0, 0);
	}

	FVector ActorPosition3D = OwnerPawn->GetTransform().InverseTransformPosition(EntityCompound.Location);
	return WorldToScreen2D(ActorPosition3D);
}

FVector2D SRTSOMiniMap::WorldToScreen2D(const AActor* Actor, APawn* OwnerPawn) const
{
	if (OwnerPawn == nullptr || Actor == nullptr) { return FVector2D(0, 0); }
	
	FVector ActorPosition3D = OwnerPawn->GetTransform().InverseTransformPosition(Actor->GetActorLocation());
	return WorldToScreen2D(ActorPosition3D);
}

FVector2D SRTSOMiniMap::WorldToScreen2D(FVector& WorldLocation) const
{
	WorldLocation = FRotator(0.f, -90.f, 0.f).RotateVector(WorldLocation);
	WorldLocation /= RadarDistanceScale;
	return FVector2D(WorldLocation);	
}

const TSharedPtr<FSlateBrush> SRTSOMiniMap::MakeRadarBGBrush(double UniformBoxSize) const
{
	static const FName RadarBGName = TEXT("RadarBackground");
	UE::Slate::FDeprecateVector2DParameter ImageSize{UniformBoxSize};

	TSharedPtr<FSlateBrush> SlateBrush = MakeShared<FSlateBrush>();
	SlateBrush->DrawAs = ESlateBrushDrawType::Box;
	SlateBrush->Margin = FMargin();
	SlateBrush->Tiling = ESlateBrushTileType::Both;
	SlateBrush->ImageType = ESlateBrushImageType::Linear;
	SlateBrush->SetImageSize(ImageSize);
	SlateBrush->TintColor = FLinearColor(0, 0, 0, 0.3f);
	
	return SlateBrush;
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
