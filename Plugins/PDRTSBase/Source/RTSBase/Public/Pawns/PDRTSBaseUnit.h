/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Pawn.h"
#include "Engine/StreamableManager.h"
#include "PDRTSBaseUnit.generated.h"

struct FPDWorkUnitDatum;
class UBehaviorTree;
class UFloatingPawnMovement;
class UCapsuleComponent;
class AActor;

UCLASS()
class PDRTSBASE_API APDRTSBaseUnit : public APawn
{
	GENERATED_BODY()

public:
	APDRTSBaseUnit();


	void PlayWorkAnimation(float Delay);
	void _PlayMontage(UAnimMontage* Montage, float Length);
	void _StopMontage(UAnimMontage* Montage = nullptr, bool bInterrupted = false) const;
	// void AddResources(); // In game module apply the inventory component
	void ResetState();
	void RequestAction(AActor* NewTarget, FGameplayTag RequestedJob);
	
protected:
	virtual void BeginPlay() override;
	void AssignTask(const FGameplayTag& JobTag);
	void LoadJobAsync(FPDWorkUnitDatum* JobDatum);
	void OnAssetsLoaded();


public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UFloatingPawnMovement* WorkerMovement = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UDecalComponent* HitDecal = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCapsuleComponent* Capsule = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMeshComponent* BodyMesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* WorkerTool = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	UStaticMesh* PendingToolMesh = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	UBehaviorTree* CurrentBT = nullptr;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	UAnimMontage* CurrentMontage = nullptr;


	TSharedPtr<FStreamableHandle> LatestJob_AsyncLoadHandle;
	AActor* TargetRef;

	static const FName SlotGroup_Default;
	static const FName BBKey_TargetRef;
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