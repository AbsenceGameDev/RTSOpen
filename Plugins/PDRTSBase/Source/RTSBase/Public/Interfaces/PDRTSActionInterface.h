/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PDRTSActionInterface.generated.h"

UENUM()
enum class ERTSActionMode : uint8
{
	Build,
	Harvest,
};

/** @brief Boilerplate */
UINTERFACE(MinimalAPI) class UPDRTSActionInterface : public UInterface { GENERATED_BODY() };

/**
 * @brief This interface will be placed on pawns or characters we want to perform an action from.
 * @note It is abstracted in the plugin to allow anything else to hook into it in the game layer
 */
class PDRTSBASE_API IPDRTSActionInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Action|Interface")
	void StartAction(ERTSActionMode ActionMode, int32 ActionID) const;
	virtual void StartAction_Implementation(ERTSActionMode ActionMode, int32 ActionID) const
	{
		// Perform actions
		return;
	}

	UFUNCTION(BlueprintNativeEvent, CallInEditor, Category = "Action|Interface")
	void StopAction(ERTSActionMode ActionMode, int32 ActionID) const;
	virtual void StopAction_Implementation(ERTSActionMode ActionMode, int32 ActionID) const
	{
		// Perform actions
		return;
	}	
public:
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