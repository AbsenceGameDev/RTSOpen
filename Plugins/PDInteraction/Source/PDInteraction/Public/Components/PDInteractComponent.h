/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "PDInteractCommon.h"
#include "Components/ActorComponent.h"
#include "PDInteractComponent.generated.h"

/**
 * @brief This component has two main functions: \n 1. Handling interaction logic. \n 2. Performing traces per frame, for downstream purposes
 */
UCLASS(ClassGroup=(Custom), Meta=(BlueprintSpawnableComponent))
class PDINTERACTION_API UPDInteractComponent : public UActorComponent
{
	GENERATED_UCLASS_BODY() // So ubt allows me to define the obj init. ctor myself

public:
	virtual void BeginPlay() override; 
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	virtual void BeginInteraction(); // Attempts an interaction with any currently traced interactable
	UFUNCTION(BlueprintCallable)
	virtual void EndInteraction();   // End a currently active interaction
	
	UFUNCTION(BlueprintCallable)
	const FPDTraceResult& GetTraceResult(bool bSearchForValidCachedResults) const;
	UFUNCTION(BlueprintCallable)
	const FPDTraceSettings& GetTraceSettings() const;
	UFUNCTION(BlueprintCallable)
	void SetTraceSettings(const FPDTraceSettings& NewSettings);
	UFUNCTION(BlueprintCallable)
	bool ContainsValidTraceResults() const;

	// Returns a list of all interactables
	UFUNCTION(BlueprintCallable)
	TArray<AActor*> GetAllInteractablesInRadius(double Radius = 500.0, bool bIgnorePerObjectInteractionDistance = false);
	
	// Meant to be used on the server for one-off for comparisons/validations
	UFUNCTION(BlueprintCallable)
	void TraceToTarget(const FVector& TraceEnd);
	void TraceToTarget(const FVector& TraceStart, const FVector& TraceEnd);
	void TraceToTarget(const FVector& TraceEnd, FCollisionQueryParams& TraceParams);
	void TraceToTarget(const FVector& TraceStart, const FVector& TraceEnd, FCollisionQueryParams& TraceParams);
	UFUNCTION(BlueprintCallable)
	FPDTraceResult TraceToTargetAndReset(const FVector& TraceEnd);
	FPDTraceResult TraceToTargetAndReset(const FVector& TraceStart, const FVector& TraceEnd);
	FPDTraceResult TraceToTargetAndReset(const FVector& TraceEnd, FCollisionQueryParams& TraceParams);
	FPDTraceResult TraceToTargetAndReset(const FVector& TraceStart, const FVector& TraceEnd, FCollisionQueryParams& TraceParams);

protected:
	UFUNCTION()
	virtual void Prerequisites();
	
	UFUNCTION(BlueprintCallable)
	void ClearTraceResults();
	
	UFUNCTION(BlueprintCallable)
	void PerformTrace();
	void PerformComparativeTraces(FVector& TraceStart, FVector& TraceEnd, FCollisionQueryParams& TraceParams, FHitResult& TraceHitResult, bool& bTraceResultFlag) const;
	void PerformSimpleTrace(const FVector& TraceStart, const FVector& TraceEnd, FCollisionQueryParams& TraceParams, FHitResult& TraceHitResult, bool& bTraceResultFlag) const;
	void TracePass(const FVector& TraceFromLocation, const FVector& TraceEnd, FCollisionQueryParams& TraceParams, FHitResult& TraceHitResult, bool& bTraceResultFlag) const;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE double GetMaxTraceDistance() const { return MaxTraceDistanceInUnrealUnits; }	
	
private:


public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxTraceDistanceInUnrealUnits = DEFAULT_TRACER_MAX_INTERACTION_DISTANCE; /**< @brief Needs to be large enough to hit objects of differing 'per-object' interaction distance limit*/
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double BoxTraceExtent = 10.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPDTraceSettings TraceSettings{};

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
	FPDTraceBuffer TraceBuffer{};

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite)
	TArray<AActor*> RuntimeIgnoreList{};
	
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