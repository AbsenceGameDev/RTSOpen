/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "Core/RTSOBaseGI.h"
#include "Core/RTSOBaseGM.h"
#include "RTSOpenCommon.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"


void URTSOBaseGI::InitializeGame_Implementation()
{
	ARTSOBaseGM* GM = Cast<ARTSOBaseGM>(GetWorld()->GetAuthGameMode());
	if (GM == nullptr)
	{
		ActiveTransitionWidget = CreateWidget<UPDTransitionWidget>(GetWorld(), WidgetClass);
		return;
	}

	// Will only run on the server if multiplayer, which is intended
	GM->LoadGame();
}

void URTSOBaseGI::OpenLevel_Implementation(TSoftObjectPtr<UWorld>& SoftWorldPtr)
{
	StartTransition();

	PendingSoftWorldPtr = SoftWorldPtr;
	
	const FLatentActionInfo DelayInfo{0,0, TEXT("Dispatch_OpenLevel"), this};
	UKismetSystemLibrary::Delay(this, 1.0f, DelayInfo);
}


void URTSOBaseGI::OnLevelLoaded_Implementation()
{
	// empty in baseclass for now
}

void URTSOBaseGI::Dispatch_OpenLevel()
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, PendingSoftWorldPtr, true);
}

void URTSOBaseGI::OnGMReady()
{
	if (bGMReady == false) { return; }

	//
	// @todo initialize actor spawners?
	
	EndTransition();
	StartTimeOffset = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	// CreateWidget();  // @todo Create hud overlay and add to viewport 
}

void URTSOBaseGI::ShowTransitionWidget_Implementation()
{
	ActiveTransitionWidget->AddToViewport();
}

void URTSOBaseGI::StartTransition_Implementation()
{
	if (ActiveTransitionWidget == nullptr)
	{
		ActiveTransitionWidget = CreateWidget<UPDTransitionWidget>(GetWorld(), WidgetClass);
	}
	
	if (ActiveTransitionWidget->IsInViewport() == false)
	{
		ActiveTransitionWidget->AddToViewport();
	}

	ActiveTransitionWidget->StartTransition();
}

void URTSOBaseGI::EndTransition_Implementation()
{
	if (ActiveTransitionWidget == nullptr)
	{
		ActiveTransitionWidget = CreateWidget<UPDTransitionWidget>(GetWorld(), WidgetClass);
	}
	
	if (ActiveTransitionWidget->IsInViewport() == false)
	{
		ActiveTransitionWidget->AddToViewport();
	}

	ActiveTransitionWidget->EndTransition();	
}

void URTSOBaseGI::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(URTSOBaseGI, bGMReady, SharedParams);
	
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