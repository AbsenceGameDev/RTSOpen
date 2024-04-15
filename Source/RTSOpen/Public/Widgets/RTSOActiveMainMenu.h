/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "CommonButtonBase.h"
#include "RTSOActiveMainMenu.generated.h"

class UCommonTextBlock;
/**
 * 
 */
UCLASS()
class RTSOPEN_API URTSOMenuButton : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UPROPERTY(Meta = (BindWidget)) 
	UCommonTextBlock* ButtonTitle = nullptr;

	UPROPERTY(Meta = (BindWidget)) 
	UCommonActionWidget* ButtonIcon = nullptr;	
};


/**
 * 
 */
UCLASS()
class RTSOPEN_API URTSOActiveMainMenu : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(Meta = (BindWidget)) 
	UCommonTextBlock* GameTitle = nullptr;

	UPROPERTY(Meta = (BindWidget)) 
	URTSOMenuButton* BtnContinue = nullptr;

	UPROPERTY(Meta = (BindWidget)) 
	URTSOMenuButton* BtnNewGame = nullptr;

	UPROPERTY(Meta = (BindWidget)) 
	URTSOMenuButton* BtnExitGame = nullptr;	
	
	UPROPERTY()
	UCommonActivatableWidget* OwningStack = nullptr;
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
