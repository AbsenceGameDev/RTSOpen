/* @author: Ario Amin @ Permafrost Development. @copyright: Full MIT License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags.h"
#include "PDRTSCameraManager.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct PDRTSBASE_API FPDInterval
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double Min = -89.0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double Max = 89.0;
};

USTRUCT(BlueprintType, Blueprintable)
struct PDRTSBASE_API FPDCameraManagerSettings : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FGameplayTag CameraMode{};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FPDInterval Pitch{-89.0, 89.0};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FPDInterval Yaw{0.0, 359.999};    
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double FOV = 90.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double NearClipPlane = 5.0;
    

    /** @brief 0 is Interpreted as Ortho view being disabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double OrthoWidth = 0.0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double CameraLagSpeed = 0.0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    double FadeAmount = -1.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FLinearColor FadeColour;
};

UCLASS()
class PDRTSBASE_API APDCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

public:
    APDCameraManager();
    
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = Camera)
    void SetCustomMode(FGameplayTag Tag);

    virtual void UpdateCamera(float DeltaTime) override;

protected:
    virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
    
    void ProcessTables();
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", Meta = (RowType = "/Script/RTSBase.PDCameraManagerSettings"))
    FDataTableRowHandle CurrentSettingsHandle;
    FPDCameraManagerSettings* SettingPtr = nullptr;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", Meta = (RequiredAssetDataTags = "RowStructure=/Script/RTSBase.PDCameraManagerSettings"))
    TArray<UDataTable*> CameraSettingsSources{};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FGameplayTag RequestedCameraState{};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Camera)
    FGameplayTag OldCameraState{};    

    // Associative maps
    TMap<FGameplayTag /*Type tag*/, FPDCameraManagerSettings* /*SettingsRow*/> TagToSettings;
    TMap<FGameplayTag /*Type tag*/, const UDataTable* /*Table*/> TagToTable;
    TMap<FGameplayTag /*Type tag*/, FName /*Rowname*/> TagToRowname;

    UPROPERTY()
    FVector CurrentLagVelocity{0.0,0.0,0.0};
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