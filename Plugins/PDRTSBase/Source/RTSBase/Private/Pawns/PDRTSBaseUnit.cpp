/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#include "Pawns/PDRTSBaseUnit.h"

#include "AIController.h"
#include "GameplayTagContainer.h"
#include "PDRTSBaseSubsystem.h"
#include "PDRTSCommon.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/DecalComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavAreas/NavArea_Obstacle.h"

const FName APDRTSBaseUnit::SlotGroup_Default = FName("DefaultGroup");
const FName APDRTSBaseUnit::BBKey_TargetRef = FName("Target");

APDRTSBaseUnit::APDRTSBaseUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WorkerCollision"));
	SetRootComponent(Capsule);
	Capsule->SetCapsuleHalfHeight(90.0);
	Capsule->SetCapsuleRadius(30.0);
	Capsule->bDynamicObstacle = true;
	Capsule->SetAreaClassOverride(UNavArea_Obstacle::StaticClass());
	Capsule->SetGenerateOverlapEvents(true);
	Capsule->SetCollisionObjectType(ECC_WorldDynamic);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionResponseToAllChannels(ECR_Overlap);
	
	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(Capsule);

	WorkerTool = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WorkerTool"));
	WorkerTool->SetupAttachment(BodyMesh, "hand_rSocket");
	Capsule->SetGenerateOverlapEvents(true);
	Capsule->SetCollisionObjectType(ECC_WorldDynamic);
	Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Capsule->SetCollisionResponseToAllChannels(ECR_Block);
	
	HitDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("HitDecal"));
	HitDecal->SetupAttachment(Capsule);
	
	WorkerMovement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("WorkerMovement"));
	WorkerMovement->MaxSpeed = 300.0f;
}

void APDRTSBaseUnit::PlayWorkAnimation(float Delay)
{
	_PlayMontage(CurrentMontage, Delay);
	if (WorkerTool == nullptr) { return; }

	WorkerTool->SetStaticMesh(PendingToolMesh);
	WorkerTool->SetVisibility(true);
}

void APDRTSBaseUnit::_PlayMontage(UAnimMontage* Montage, float Length)
{
	BodyMesh->GetAnimInstance()->Montage_Play(Montage, 1.0, EMontagePlayReturnType::Duration);

	FOnMontageEnded EndDelegate = FOnMontageEnded::CreateUObject(this, &APDRTSBaseUnit::_StopMontage);
	BodyMesh->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, Montage);
	
	const FLatentActionInfo DelayInfo{0,0, TEXT("_StopMontage"), this};
	UKismetSystemLibrary::Delay(this, Length, DelayInfo);
}

void APDRTSBaseUnit::_StopMontage(UAnimMontage* Montage, bool bInterrupted) const
{
	BodyMesh->GetAnimInstance()->Montage_StopGroupByName(0.0, SlotGroup_Default);
	WorkerTool->SetVisibility(false);
}

void APDRTSBaseUnit::ResetState()
{
	AssignTask(TAG_AI_Job_Idle);
}

void APDRTSBaseUnit::RequestAction(AActor* NewTarget, FGameplayTag RequestedJob)
{
	if (NewTarget == nullptr) { return; }

	TargetRef = NewTarget;
	AssignTask(RequestedJob);

	// @todo Call GM and update unit save data here?
}

void APDRTSBaseUnit::BeginPlay()
{
	Super::BeginPlay();

	UCapsuleComponent* InCap = Capsule;
	AddActorWorldOffset(FVector{0.0, 0.0, Capsule->GetScaledCapsuleHalfHeight()});

	//
	// Begin tasks?
	
	
	//
	// Switch job?
	AssignTask(TAG_AI_Job_Idle);
}

void APDRTSBaseUnit::AssignTask(const FGameplayTag& JobTag)
{
	UPDRTSBaseSubsystem* RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	if (RTSSubsystem == nullptr) { return; }

	const FPDWorkUnitDatum* WorkUnitDatum = RTSSubsystem->GetWorkEntry(JobTag);
	if (WorkUnitDatum == nullptr) { return; }

	// 1. Hide previous tools mesh
	if (WorkerTool != nullptr) { WorkerTool->SetVisibility(false); }
	// 2. Stop montage
	UAnimInstance* BodyInstance = BodyMesh != nullptr ? BodyMesh->GetAnimInstance() : nullptr;
	if (BodyInstance != nullptr) { BodyInstance->Montage_StopGroupByName(0.0, SlotGroup_Default); }

	AAIController* WorkerContoller = GetController<AAIController>();
	if (WorkerContoller != nullptr) { WorkerContoller->StopMovement(); }
	
	// Display new tools mesh ?
}


void APDRTSBaseUnit::LoadJobAsync(FPDWorkUnitDatum* JobDatum)
{
	AAIController* AIController = GetController<AAIController>();
	UPDRTSBaseSubsystem* RTSSubsystem = GEngine->GetEngineSubsystem<UPDRTSBaseSubsystem>();
	if (RTSSubsystem == nullptr || AIController == nullptr) { return; }
	
	TArray<FSoftObjectPath> ToStream;
	JobDatum->Montage;
	ToStream.Add(JobDatum->Montage.ToSoftObjectPath());
	ToStream.Add(JobDatum->BT.ToSoftObjectPath());
	ToStream.Add(JobDatum->WorkToolMesh.ToSoftObjectPath());
	
	// Getting our StreamableManager
	LatestJob_AsyncLoadHandle =
		RTSSubsystem->DataStreamer.RequestAsyncLoad(ToStream,
		FStreamableDelegate::CreateUObject(this,
			&APDRTSBaseUnit::OnAssetsLoaded
		)
	);
}

void APDRTSBaseUnit::OnAssetsLoaded()
{
	// empty for now
	AAIController* AIController = GetController<AAIController>();
	if (AIController == nullptr) { return; }

	TArray<UObject*> Objects;
	LatestJob_AsyncLoadHandle.Get()->GetLoadedAssets(Objects);


	for (UObject* JobRelatedObject : Objects)
	{
		UBehaviorTree* AsBT = Cast<UBehaviorTree>(JobRelatedObject);
		if (AsBT != nullptr) 
		{ 
			CurrentBT = AsBT;
			AIController->RunBehaviorTree(CurrentBT);

			if (TargetRef == nullptr) { continue; }

			AIController->GetBlackboardComponent()->SetValueAsObject(BBKey_TargetRef, TargetRef);
			continue;
		}
		
		UStaticMesh* AsToolMesh = Cast<UStaticMesh>(JobRelatedObject);
		if (AsToolMesh != nullptr) { PendingToolMesh = AsToolMesh; }

		UAnimMontage* AsMontage = Cast<UAnimMontage>(JobRelatedObject);
		if (AsMontage != nullptr) { CurrentMontage = AsMontage; }
	}
}

void APDRTSBaseUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APDRTSBaseUnit::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
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