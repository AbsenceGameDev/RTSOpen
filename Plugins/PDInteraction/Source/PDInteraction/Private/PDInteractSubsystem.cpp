/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "PDInteractSubsystem.h"

#include "PDInteractCommon.h"
#include "Interfaces/PDInteractInterface.h"

void UPDInteractSubsystem::RegisterWorldInteractable_Implementation(UWorld* SelectedWorld, AActor* SelectedInteractable)
{
	check(SelectedWorld != nullptr)

	if (SelectedInteractable == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Called 'UPDInteractSubsystem::RegisterWorldInteractable' without valid actor"));
		return;
	}

	if (Cast<IPDInteractInterface>(SelectedInteractable) == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Called 'UPDInteractSubsystem::RegisterWorldInteractable' without valid interactable actor"));
		return;
	}
	
	const FRTSSavedInteractables Interactable{
		SelectedInteractable,
		SelectedInteractable->GetClass(), // Interact Actor class
		SelectedInteractable->GetActorLocation(), // Interact Actor location
		IPDInteractInterface::Execute_GetCurrentUsability(SelectedInteractable) // Interact Actor usability
		};
	
	WorldInteractables.FindOrAdd(SelectedWorld).ActorInfo.Emplace(Interactable);
}

void UPDInteractSubsystem::TransferringWorld(UWorld* OldWorld, UWorld* TargetWorld)
{
	check(OldWorld != nullptr)
	check(TargetWorld != nullptr)

	WorldInteractables.FindOrAdd(TargetWorld).ActorInfo = WorldInteractables.Find(OldWorld)->ActorInfo;
	WorldInteractables.Remove(OldWorld);
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