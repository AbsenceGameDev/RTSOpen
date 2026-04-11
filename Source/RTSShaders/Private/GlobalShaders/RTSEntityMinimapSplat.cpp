/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#include "GlobalShaders/RTSEntityMinimapSplat.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"

// This looks messy but reduced the passes down from 136 passes to 28 passes for a 256x256 datatexture
// Note: I couldn't get it to load the parameters properly unless defined in their own structs, and thus the mess below. Inheritance caused issues and shared SHADER_PARAMETER_STRUCTs caused other issues.  
void FRTSMinimapSplat::BuildAndExecuteGraph(FRHICommandListImmediate& RHICmdList, UTextureRenderTarget2D* RenderTarget, const TRefCountPtr<FRDGPooledBuffer>& EntityInputPooledBuffer, const TRefCountPtr<IPooledRenderTarget>& ExternalPooledTexture, const TArray<FLinearColor>& InData, const FVector& RegionMin, const FVector& RegionSize)
{
	// constexpr uint32 GMaxBuffer1Dim = 256;
	// constexpr uint32 GMaxBufferElements = 256*256;

	FRDGBuilder GraphBuilder(RHICmdList);
	FRTSMinimapSplat::FParameters* AllocatedPassParameter = GraphBuilder.AllocParameters<FRTSMinimapSplat::FParameters>();
	// FRTSMinimapSplatFirstPass::FParameters* AllocatedPassParameter_FirstPass = GraphBuilder.AllocParameters<FRTSMinimapSplatFirstPass::FParameters>();
	// FRTSMinimapSplatInnerCheat::FParameters* AllocatedPassParameter_InnerCheat = GraphBuilder.AllocParameters<FRTSMinimapSplatInnerCheat::FParameters>();
	// FRTSMinimapSplatCopyToTexture::FParameters* AllocatedPassParameter_CopyToTexture = GraphBuilder.AllocParameters<FRTSMinimapSplatCopyToTexture::FParameters>();

	// GetStaticType();

	// Buffer UAV
	FRDGBufferRef DataBufferRDG = GraphBuilder.RegisterExternalBuffer(
		EntityInputPooledBuffer,
		*FString(TEXT("SortDataBuffer")),
		ERDGBufferFlags::MultiFrame
	);
	GraphBuilder.QueueBufferUpload(DataBufferRDG, TArrayView<const FLinearColor>(InData), ERDGInitialDataFlags::None);
	
	FRDGBufferUAVDesc UAVDesc(DataBufferRDG, EPixelFormat::PF_A32B32G32R32F);
	FRDGBufferUAVRef EntityUAV = GraphBuilder.CreateUAV(UAVDesc);
	AllocatedPassParameter->EntityData = EntityUAV;
	
	// Texture UAV
	FRHITexture2D* RhiTexture = RenderTarget->GetResource()->GetTexture2DRHI();
	FRDGTextureRef OutTextureRef = GraphBuilder.RegisterExternalTexture(
		ExternalPooledTexture, 
		*FString(TEXT("RT_MinimapEntityData")),
		ERDGTextureFlags::None
	); 

	FRDGTextureUAVDesc OutTextureUAVDesc(OutTextureRef);
	FRDGTextureUAVRef TextureBufferUAV = GraphBuilder.CreateUAV(OutTextureUAVDesc, ERDGUnorderedAccessViewFlags::None);
	AllocatedPassParameter->OutSortedEntityDataTexture = TextureBufferUAV;

	const uint32 GroupSize = 64; 
	
	AllocatedPassParameter->RegionMin = FVector2f{static_cast<float>(RegionMin.X), static_cast<float>(RegionMin.Y)};
	AllocatedPassParameter->RegionSize = FVector2f{static_cast<float>(RegionSize.X), static_cast<float>(RegionSize.Y)};
	AllocatedPassParameter->NumEntities = InData.Num();
	TShaderMapRef<FRTSMinimapSplat> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("RTSMinimapSplat"), ComputeShader, AllocatedPassParameter, FComputeShaderUtils::GetGroupCount(InData.Num(), GroupSize));	

	// // --- GLOBAL PASSES & CHEAT PASSES ---
	// // We start at Step 2048 because 2 through 1024 are already sorted
	// for (uint32 Step = 2048; Step <= GMaxBufferElements; Step <<= 1)
	// {
	// 	AllocatedPassParameter->Step = AllocatedPassParameter_InnerCheat->Step = Step;

	// 	// Global Passes
	// 	// These are stages where the comparison distance (Jump) is larger than the LDS capacity
	// 	for (uint32 Jump = Step >> 1; Jump > 1024; Jump >>= 1)
	// 	{
	// 			AllocatedPassParameter->Jump = Jump;
	// 			TShaderMapRef<FRTSMinimapSplat> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	// 			FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("BitonicGlobal_S%d_J%d", Step, Jump), ComputeShader, AllocatedPassParameter, FIntVector(NumGroups, 1, 1));
	// 	}
	
	// 	// Inner Cheat LDS Pass (handles remainder jumps of loop)
	// 	{ 
	// 		TShaderMapRef<FRTSMinimapSplatInnerCheat> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	// 		FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("RTSMinimapSplatInnerCheat%d", Step), ComputeShader, AllocatedPassParameter_InnerCheat, FIntVector(NumGroups, 1, 1));
	// 	}
	// } 

	// // --- FINAL COPY --- 
	// {
	// 	constexpr uint32 Uniform2DThreadCount = 8;
	// 	constexpr uint32 ThreadGroupCount = GMaxBuffer1Dim / Uniform2DThreadCount;
	// 	TShaderMapRef<FRTSMinimapSplatCopyToTexture> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
	// 	FComputeShaderUtils::AddPass(GraphBuilder, RDG_EVENT_NAME("RTSMinimapSplatCopyToTexture"), ComputeShader, AllocatedPassParameter_CopyToTexture, FIntVector(ThreadGroupCount, ThreadGroupCount, 1));
	// }

	GraphBuilder.Execute();
}

IMPLEMENT_GLOBAL_SHADER(FRTSMinimapSplat, "/Project/MinimapSplat.usf", "MinimapSplat", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FRTSMinimapSplatInnerCheat, "/Project/SortData.usf", "SortDataInnerCheat", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FRTSMinimapSplatFirstPass, "/Project/SortData.usf", "SortDataFirstPass", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FRTSMinimapSplatCopyToTexture, "/Project/SortData.usf", "CopyToTexture", SF_Compute);


/**
Business Source License 1.1

Parameters

Licensor:             Ario Amin (@ Permafrost Development)
Licensed Work:        RTSShaders (Source available on github)
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

The Business Source License (this document, or the “License”) is not an Shaders
Source license. However, the Licensed Work will eventually be made available
under an Shaders Source License, as stated in this License.

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