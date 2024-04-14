/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#include "Core/RTSOBaseGM.h"
#include "Core/RTSOBaseGI.h"

#include "PDInteractSubsystem.h"
#include "RTSOpenCommon.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

const FString ARTSOBaseGM::ROOTSAVE = "ROOTSAVE" ; 

void ARTSOBaseGM::BeginPlay()
{
	Super::BeginPlay();

	URTSOBaseGI* GI = Cast<URTSOBaseGI>(GetWorld()->GetGameInstance());
	if (GI == nullptr) { return; }

	MARK_PROPERTY_DIRTY_FROM_NAME(URTSOBaseGI, bGMReady, GI);
	GI->bGMReady = true;
	
	
}

void ARTSOBaseGM::OnGeneratedLandscapeReady_Implementation()
{
	// Load file data if it exists
}

void ARTSOBaseGM::LoadGame_Implementation()
{
	const bool bDoesExist = UGameplayStatics::DoesSaveGameExist(ROOTSAVE,0);

	if (bDoesExist)
	{
 		GameSave = Cast<URTSOpenSaveGame>(UGameplayStatics::LoadGameFromSlot(ROOTSAVE,0));
		return;
	}
	GameSave = Cast<URTSOpenSaveGame>(UGameplayStatics::CreateSaveGameObject(URTSOpenSaveGame::StaticClass()));
}


void ARTSOBaseGM::ClearSave_Implementation(const bool bRefreshSeed)
{
	check(GameSave != nullptr)

	GameSave->GameTime = 0;
	GameSave->Interactables.Empty();
	GameSave->Inventories.Empty();

	if (bRefreshSeed)
	{
		GameSave->Seeder = UKismetMathLibrary::MakeRandomStream(FMath::RandRange(0, INT_MAX));
	}
	
	bHasStartedSaveData = false;
}

void ARTSOBaseGM::SaveGame_Implementation()
{
	check(GameSave != nullptr)

	bHasSavedDataAsync = false;
	FAsyncSaveGameToSlotDelegate Dlgt = FAsyncSaveGameToSlotDelegate::CreateLambda(
		[This = this](const FString&, int, bool bResult)
		{
			if (This == nullptr) { return; }
			This->bHasSavedDataAsync = bResult; 
		});
	
	UGameplayStatics::AsyncSaveGameToSlot(GameSave, "ROOT_SAVE", 0, Dlgt);
	bHasStartedSaveData = true;
}

void ARTSOBaseGM::SaveInteractables_Implementation()
{
	check(GameSave != nullptr)
	check(GEngine->GetEngineSubsystem<UPDInteractSubsystem>() != nullptr)
	GameSave->Interactables.Empty();

	FPDArrayListWrapper& SaveInfo = *GEngine->GetEngineSubsystem<UPDInteractSubsystem>()->WorldInteractables.Find(GetWorld());
	for (FRTSSavedInteractables& _Actor: SaveInfo.ActorInfo)
	{
		// Update class if needed
		if (_Actor.ActorInWorld == nullptr)
		{
			_Actor.Usability = 0.0;
			continue;
		}

		// Update class if needed
		if (_Actor.ActorClass == nullptr)
		{
			_Actor.ActorClass = _Actor.ActorInWorld->GetClass();
		}
		
		// update location
		_Actor.Location = _Actor.ActorInWorld->GetActorLocation();
	}
	
	GameSave->Interactables.Append(SaveInfo.ActorInfo);

	SaveGame();
}

void ARTSOBaseGM::SaveResources_Implementation(const TMap<int32, FRTSSavedItems>& DataRef)
{
	check(GameSave != nullptr)
	GameSave->Inventories.Append(DataRef); // overwrite any previous value, this is low so only allow saving resources every 3 seconds

	const FLatentActionInfo DelayInfo{0,0, TEXT("SaveGame"), this};
	UKismetSystemLibrary::Delay(GetOwner(), 4.0f, DelayInfo);
}

/* Save current units, their locations and actor classes*/
void ARTSOBaseGM::SaveUnits_Implementation()
{
	check(GameSave != nullptr)

	// @todo possibly will be part of interactables, think on it a couple of hours
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
