/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#pragma once

#include "CoreMinimal.h"
#include "RTSOpenCommon.h"
#include "GameFramework/GameModeBase.h"
#include "RTSOpenGM.generated.h"

class URTSOpenSaveGame;

UENUM()
enum ERTSLoadscreenState
{
	LOADING_IN,
	LOADING_OUT,
};

UCLASS()
class RTSOPEN_API ARTSOpenGM : public AGameModeBase
{
	GENERATED_BODY()

public: // Method members
	virtual void BeginPlay() override;
	
	void AnimateLoadingScreen(ERTSLoadscreenState RequestedState);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGeneratedLandscapeReady();
	
	//
	// Saving/loading
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ClearSave(const bool bRefreshSeed = false);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveGame();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void LoadGame();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveInteractables();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveResources(const TMap<int32, FRTSSavedItems>& DataRef);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveUnits();
	
public: // Variable members
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	URTSOpenSaveGame* GameSave = nullptr;
	
	bool bHasStartedSaveData = false;
	bool bHasSavedDataAsync = false;
	const TMap<int32, FRTSSavedItems>* MapPointer = nullptr;

	static const FString ROOTSAVE;
};

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
