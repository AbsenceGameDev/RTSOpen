/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/PDInteractInterface.h"

#include "PDInteractActor.generated.h"

constexpr double UNREALUNITS_PERMETRE = 100.0; /* UU == cm, 100 uu == 1m*/
constexpr double INVERSE_UU = 1 / UNREALUNITS_PERMETRE; /* UU == cm, 100 uu == 1m*/

class USceneComponent;
class UStaticMeshComponent;
class UBoxComponent;


UCLASS()
class PDINTERACTION_API APDInteractActor : public AActor, public IPDInteractInterface
{
	GENERATED_BODY()

public:
	APDInteractActor();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UFUNCTION() 
	void ResizeCollisionBounds(UStaticMeshComponent* NewMeshDummy = nullptr);
	void BindDelegates();

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

public:
	/** @brief Root of the actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	USceneComponent* Scenecomp = nullptr;

	/** @brief Mesh of the actor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	UStaticMeshComponent* Mesh = nullptr;

	/** @brief Box collision that acts as the interaction surface */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	UBoxComponent* Boxcomp = nullptr;
	
	/** @brief Value that controls how much we want to control ur padding when sizing the collision bounds  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction|Actors")
	double UniformCollisionPadding = UNREALUNITS_PERMETRE * 0.4;
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