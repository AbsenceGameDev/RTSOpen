/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "PDInteractCommon.h"
#include "Net/PDItemNetDatum.h"

#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/SaveGame.h"
#include "CommonActivatableWidget.h"
#include "CommonButtonBase.h"
#include "RTSOpenCommon.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FRTSSavedItems
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPDItemNetDatum> Items;
};

USTRUCT(BlueprintType, Blueprintable)
struct FRTSSavedWorldUnits
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag UnitType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	double Health = 0.0;	
	
};

UCLASS(BlueprintType, Blueprintable)
class RTSOPEN_API URTSOpenSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	FRandomStream Seeder{0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	double GameTime = 0.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TArray<FRTSSavedInteractables> Interactables;

	/** @brief userid tied to some account id?, in singleplayer keep only one entry */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameInstance|Widgets")
	TMap<int32, FRTSSavedItems> Inventories;

};

UCLASS(BlueprintType, Blueprintable)
class UPDTransitionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartTransition();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndTransition();
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget|BaseTransition", Meta = (BindWidget))
	UWidgetAnimation* TransitionAnimation = nullptr;
};

UCLASS(BlueprintType, Blueprintable)
class URTSHudWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
};


UCLASS(BlueprintType, Blueprintable)
class URTSInteractWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget|Base", Meta = (BindWidget))
	UCommonButtonBase* InteractButton = nullptr;
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