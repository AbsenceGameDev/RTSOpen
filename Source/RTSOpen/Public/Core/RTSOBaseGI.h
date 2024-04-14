/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "RTSOBaseGI.generated.h"


class UPDTransitionWidget;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class RTSOPEN_API URTSOBaseGI : public UGameInstance
{
	GENERATED_BODY()

public:
	//
	// Game control

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void InitializeGame();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OpenLevel(TSoftObjectPtr<UWorld>& SoftWorldPtr);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnLevelLoaded();


	UFUNCTION()
	void Dispatch_OpenLevel();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartTransition();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndTransition();

	UFUNCTION()
	void OnGMReady();
	
	
	//
	// Widget
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowTransitionWidget();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TSubclassOf<UPDTransitionWidget> WidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	UPDTransitionWidget* ActiveTransitionWidget = nullptr;

	UPROPERTY()
	double StartTimeOffset;

	UPROPERTY(ReplicatedUsing = OnGMReady)
	bool bGMReady;

protected:
	TSoftObjectPtr<UWorld> PendingSoftWorldPtr;
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
