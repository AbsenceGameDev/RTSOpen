/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "RTSOpenGI.h"

#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "RTSOpen/RTSOpenCommon.h"
#include "RTSOpen/RTSOpenGM.h"


void URTSOpenGI::InitializeGame_Implementation()
{
	ARTSOpenGM* GM = Cast<ARTSOpenGM>(GetWorld()->GetAuthGameMode());
	if (GM == nullptr)
	{
		ActiveTransitionWidget = Cast<UPDTransitionWidget>(CreateWidget(nullptr, WidgetClass));
		return;
	}

	// Will only run on the server if multiplayer, which is intended
	GM->LoadGame();
}

void URTSOpenGI::OpenLevel_Implementation(TSoftObjectPtr<UWorld> SoftWorldPtr)
{
	StartTransition();

	PendingSoftWorldPtr = SoftWorldPtr;
	
	const FLatentActionInfo DelayInfo{0,0, TEXT("Dispatch_OpenLevel"), this};
	UKismetSystemLibrary::Delay(this, 1.0f, DelayInfo);
}


void URTSOpenGI::OnLevelLoaded_Implementation()
{
	// empty in baseclass for now
}

void URTSOpenGI::Dispatch_OpenLevel()
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, PendingSoftWorldPtr, true);
}

void URTSOpenGI::OnGMReady()
{
	if (bGMReady == false) { return; }

	//
	// @todo initialize actor spawners?
	
	EndTransition();
	StartTimeOffset = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();

	CreateWidget(); 
}

void URTSOpenGI::ShowTransitionWidget_Implementation()
{
	ActiveTransitionWidget->AddToViewport();
}

void URTSOpenGI::StartTransition_Implementation()
{
	if (ActiveTransitionWidget == nullptr)
	{
		ActiveTransitionWidget = Cast<UPDTransitionWidget>(CreateWidget(nullptr, WidgetClass));
	}
	
	if (ActiveTransitionWidget->IsInViewport() == false)
	{
		ActiveTransitionWidget->AddToViewport();
	}

	ActiveTransitionWidget->StartTransition();
}

void URTSOpenGI::EndTransition_Implementation()
{
	if (ActiveTransitionWidget == nullptr)
	{
		ActiveTransitionWidget = Cast<UPDTransitionWidget>(CreateWidget(nullptr, WidgetClass));
	}
	
	if (ActiveTransitionWidget->IsInViewport() == false)
	{
		ActiveTransitionWidget->AddToViewport();
	}

	ActiveTransitionWidget->EndTransition();	
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