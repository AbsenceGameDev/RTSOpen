/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */
#include "Inv/PDInventoryComponent.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"


UPDInventoryComponent::UPDInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	
	ItemList.OwningInventory = this;

	// @todo sett polling to 0 times per second to enforce stateful replication
}

void UPDInventoryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams Parameters{};
	Parameters.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UPDInventoryComponent, ItemList, Parameters);
	
}

void UPDInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	ItemList.OwningInventory = this;
}

void UPDInventoryComponent::RequestUpdateItem(TEnumAsByte<EPDItemNetOperation> RequestedOperation, FGameplayTag& ItemTag, int32 Count)
{
	// If dedicated server, allow if possible
	
	// GetOwner()->GetNetMode() == NM_DedicatedServer





	// // @todo uncomment and finish after push
	// // Relevant authorative code
	// ItemList.Items.Emplace(); // add
	// ItemList.Items.MarkAsRemoved(); // Actually removing is expensive
	// ItemList.Items.Emplace(); // add
	//
	// GetOwner()->ForceNetUpdate();

	
}

void UPDInventoryComponent::OnDatumUpdated(FPDItemNetDatum* ItemNetDatum, EPDItemNetOperation Operation)
{
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
