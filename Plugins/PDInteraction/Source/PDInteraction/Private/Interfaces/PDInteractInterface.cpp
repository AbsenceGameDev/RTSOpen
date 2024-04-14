/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#include "Interfaces/PDInteractInterface.h"
#include "PDInteractCommon.h"
#include "PDInteractSubsystem.h"

void IPDInteractInterface::OnInteract_Implementation(FPDInteractionParams& InteractionParams, EPDInteractResult& InteractResult) const
{
	InteractResult = EPDInteractResult::INTERACT_SUCCESS;
	return;
}

double IPDInteractInterface::GetMaxInteractionDistance_Implementation() const
{
	return DEFAULT_PEROBJECT_MAX_INTERACTION_DISTANCE;
}

double IPDInteractInterface::GetCurrentUsability_Implementation() const
{
	return 1.0;
}

void IPDInteractInterface::RegisterWorldInteractable(UWorld* SelectedWorld, AActor* SelectedInteractable)
{
	check(SelectedWorld != nullptr)
	check(SelectedInteractable != nullptr)
	check(SelectedInteractable->Implements<IPDInteractInterface>())
	check(GEngine != nullptr)
	check(GEngine->GetEngineSubsystem<UPDInteractSubsystem>() != nullptr)

	GEngine->GetEngineSubsystem<UPDInteractSubsystem>()->RegisterWorldInteractable(SelectedWorld, SelectedInteractable);
	bHasBeenRegisteredWithCurrentWorld = true;
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
