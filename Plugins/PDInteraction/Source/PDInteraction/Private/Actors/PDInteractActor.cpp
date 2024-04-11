/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "Actors/PDInteractActor.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

APDInteractActor::APDInteractActor()
{
	PrimaryActorTick.bCanEverTick = false; // we don't need the interactable actors to tick on default

	Scenecomp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Scenecomp);
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Scenecomp);
	
	Boxcomp = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractShape"));
	Boxcomp->SetupAttachment(Scenecomp);


	constexpr ECollisionResponse NewResponse = ECollisionResponse::ECR_Block;
	// @todo will redo this, but for now fingers crossed the last 4 response channels (#18 and back) are not already being used
	Boxcomp->SetCollisionResponseToChannel(DEDICATED_INTERACT_CHANNEL_ALT18, NewResponse);
	Boxcomp->SetCollisionResponseToChannel(DEDICATED_INTERACT_CHANNEL_ALT17, NewResponse);
	Boxcomp->SetCollisionResponseToChannel(DEDICATED_INTERACT_CHANNEL_ALT16, NewResponse);
	Boxcomp->SetCollisionResponseToChannel(DEDICATED_INTERACT_CHANNEL_ALT15, NewResponse);	
	Boxcomp->SetCollisionResponseToChannel(DEDICATED_INTERACT_CHANNEL_ALT14, NewResponse);	
}


void APDInteractActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	BindDelegates();
}

void APDInteractActor::BeginPlay()
{
	Super::BeginPlay();
	ResizeCollisionBounds();
	
}

void APDInteractActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void APDInteractActor::ResizeCollisionBounds(UStaticMeshComponent* NewMeshDummy)
{
	if (NewMeshDummy != nullptr && NewMeshDummy->IsValidLowLevelFast() && Mesh != NewMeshDummy)
	{
		Mesh = NewMeshDummy;
	}
	
	const FVector AbsExtent = Mesh->Bounds.BoxExtent * 0.5f + (UniformCollisionPadding * FVector::OneVector);
	Boxcomp->SetBoxExtent(AbsExtent);
	Boxcomp->SetWorldRotation(FRotationMatrix::MakeFromX(FVector{1,0,0}).Rotator());
}

void APDInteractActor::BindDelegates()
{
#if WITH_EDITOR
	Mesh->OnStaticMeshChanged().AddUObject(this, &APDInteractActor::ResizeCollisionBounds);
#endif // WITH_EDITOR
	
}

#if WITH_EDITOR

void APDInteractActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	const FProperty* Property = PropertyChangedEvent.Property;
	const bool bIsPropertyOfCorrectType = Property != nullptr && Property->IsA(FDoubleProperty::StaticClass());
	if (bIsPropertyOfCorrectType == false)
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}
	
	const FName PropertyName = Property->GetFName();
	const bool bDoesPropertyHaveCorrectName = PropertyName.IsEqual(GET_MEMBER_NAME_CHECKED(APDInteractActor, UniformCollisionPadding));
	if (bDoesPropertyHaveCorrectName == false)
	{
		Super::PostEditChangeProperty(PropertyChangedEvent);
		return;
	}
	
	ResizeCollisionBounds();
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif // WITH_EDITOR

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
