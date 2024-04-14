/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interfaces/PDWorldManagementInterface.h"
#include "Subsystems/EngineSubsystem.h"
#include "PDInteractSubsystem.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FPDArrayListWrapper
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FRTSSavedInteractables> ActorInfo{};
};


UCLASS(BlueprintType, Blueprintable)
class PDINVENTORY_API UPDInteractSubsystem : public UEngineSubsystem, public IPDWorldManagementInterface
{
	GENERATED_BODY()
public:	
	virtual void RegisterWorldInteractable(UWorld* SelectedWorld, AActor* SelectedInteractable) override;

	virtual void TransferringWorld(UWorld* OldWorld, UWorld* TargetWorld);
	
public:	
	UPROPERTY(VisibleInstanceOnly, Category = "Interaction Subsystem")
	TMap<UWorld* /*SelectedWorld*/, FPDArrayListWrapper /*SelectedInteractables*/> WorldInteractables{};
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

