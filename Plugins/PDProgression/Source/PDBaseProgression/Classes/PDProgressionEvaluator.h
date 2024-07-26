/* @author: Ario Amin @ Permafrost Development. @copyright: Full BSL(1.1) License included at bottom of the file  */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PDProgressionEvaluator.generated.h"

/* @brief @todo */
UENUM()
enum class EPDRulesetOpType : uint8
{
	EAddition,
	ESubtraction,
	EMultiplication,
	EDivision,
	EPower
};

/* @brief @todo */
UENUM()
enum class EPDRulesetOpTarget : uint8
{
	ESelf,
	EOther,
	EStatic
};


/* @brief @todo */
USTRUCT(Blueprintable)
struct PDBASEPROGRESSION_API FPDRulesetOperatorStruct : public FTableRowBase
{
	GENERATED_BODY()
	
	/** @brief The (arithmetic) operation type of this particular FPDRulesetOperatorStruct
	 *  @note Example: 'EAddition', 'ESubtraction', 'EMultiplication', 'EDivision', 'EPower */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPDRulesetOpType OpType;
	
	/** @brief The target, or x, of the operations on this particular FPDRulesetOperatorStruct
	 * @note  Example: EThis, EOther, EStatic */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPDRulesetOpTarget OpTarget;

	/** @brief If EPDRulesetOpTarget == EThis or EOther, then this will be ignored,
	 * @note If EStatic this will be the value used  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 OpFallbackValue;

	/**
	 * @brief Recursive rule to apply to the upper
	 * @details Using this will make teh evaluator implicitly treat the upper FPDRulesetOperatorStruct and this (and itself recursively)
	 * as if within a parenthesis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RowType="/Script/PDBaseProgression.PDRulesetOperatorStruct"))
	FDataTableRowHandle InnerRulesetHandle;
};

/* @brief @todo */
template<typename TValueType>
struct FPDRulesetEvaluator
{
	/* @brief @todo */
	TValueType Eval(
		const TArray<FPDRulesetOperatorStruct>& Ruleset, 
		const TValueType& PotentialSelfValue,
		const TValueType& PotentialOtherValue,
		const TValueType& PotentialStaticValue)
		{
			TValueType OperationResult{};
		
			const int32 RuleLimits = Ruleset.Num();
			TMap<int32, TArray<int32> /**/> IndexOrderOfOperations;
			
			// Tally Top-level operations
			for (int32 RuleIdx = 0;  RuleIdx < RuleLimits; RuleIdx++)
			{
				int32 SelectedPrecedence = 0;
				const FPDRulesetOperatorStruct& Rule = Ruleset[RuleIdx];
				switch (Rule.OpType)
				{
					case EPDRulesetOpType::EAddition: case EPDRulesetOpType::ESubtraction: 
						SelectedPrecedence = 0;
						break;
					case EPDRulesetOpType::EMultiplication: case EPDRulesetOpType::EDivision: 
						SelectedPrecedence = 1;
						break;
					case EPDRulesetOpType::EPower:
						SelectedPrecedence = 2;
						break;
				}
				

				TArray<int32>& InnerToplevelOperations = 
					IndexOrderOfOperations.Contains(SelectedPrecedence) 
					? *IndexOrderOfOperations.Find(SelectedPrecedence)
					: IndexOrderOfOperations.Emplace(SelectedPrecedence);
				InnerToplevelOperations.Emplace(RuleIdx);
			}

			// Process operations
			constexpr int32 MaxPrecedence = 2;

			TArray<int32> OpValues;
			OpValues.SetNum(RuleLimits);
			

			for (int32 PrecedenceLevel = MaxPrecedence; PrecedenceLevel >= 0; PrecedenceLevel--)
			{
				if (IndexOrderOfOperations.Contains(PrecedenceLevel) == false)
				{
					continue;
				}

				TArray<int32>& InnerToplevelOperations = *IndexOrderOfOperations.Find(PrecedenceLevel);

				// Tally all operation values
				for (const int32 RuleIndex : InnerToplevelOperations)
				{
					const FPDRulesetOperatorStruct& Rule = Ruleset[RuleIndex];

					
					switch(Rule.OpTarget)
					{
						case EPDRulesetOpTarget::ESelf:   OpValues[RuleIndex] = PotentialSelfValue; break;
						case EPDRulesetOpTarget::EOther:  OpValues[RuleIndex] = PotentialOtherValue; break;
						case EPDRulesetOpTarget::EStatic: OpValues[RuleIndex] = PotentialStaticValue; break;
					}
				}


				for (const int32 RuleIndex : InnerToplevelOperations)
				{
					// First index can't apply their operator to a previous rule. so it is ignored from this point
					if (RuleIndex == 0) 
					{
						continue;
					}
					const FPDRulesetOperatorStruct& PreviousRule = Ruleset[RuleIndex - 1];
					const FPDRulesetOperatorStruct& CurrentRule = Ruleset[RuleIndex];	

					TValueType PreviousOpBaseValue = OpValues[RuleIndex - 1];
					TValueType CurrentOpBaseValue = OpValues[RuleIndex];

					// Expand value of PreviousRule before handling the operator from the CurrentOpBaseValue
					TValueType ExpandedPreviousIndexValue = 0;
					if (PreviousRule.InnerRulesetHandle.IsNull())
					{
						TArray<FPDRulesetOperatorStruct> InnerRulesetRecursion;
						FPDRulesetOperatorStruct* PrevInnerRulesetPtr = PreviousRule.InnerRulesetHandle.GetRow<FPDRulesetOperatorStruct>("");
						if (PrevInnerRulesetPtr != nullptr) 
						{ 
							InnerRulesetRecursion.Emplace(*PrevInnerRulesetPtr);
							ExpandedPreviousIndexValue = FPDRulesetEvaluator::Eval(InnerRulesetRecursion, PotentialSelfValue, PotentialOtherValue, PotentialStaticValue);
						}
					}


					// Expand value of CurrentRule before handling the operator from the CurrentOpBaseValue
					TValueType ExpandedCurrentIndexValue = 0;
					if (CurrentRule.InnerRulesetHandle.IsNull())
					{
						TArray<FPDRulesetOperatorStruct> InnerRulesetRecursion;
						FPDRulesetOperatorStruct* CurrentInnerRulesetPtr = CurrentRule.InnerRulesetHandle.GetRow<FPDRulesetOperatorStruct>("");
						if (CurrentInnerRulesetPtr != nullptr) 
						{ 
							InnerRulesetRecursion.Emplace(*CurrentInnerRulesetPtr);
							ExpandedCurrentIndexValue = FPDRulesetEvaluator::Eval(InnerRulesetRecursion, PotentialSelfValue, PotentialOtherValue, PotentialStaticValue);
						}
												
					}
					
					const FPDRulesetOperatorStruct& Rule = Ruleset[RuleIndex];
					switch (Rule.OpType)
					{
						case EPDRulesetOpType::EAddition:
							OperationResult = ExpandedPreviousIndexValue + ExpandedCurrentIndexValue;
							break;
						case EPDRulesetOpType::ESubtraction: 
							OperationResult = ExpandedPreviousIndexValue - ExpandedCurrentIndexValue;
							break;
						case EPDRulesetOpType::EMultiplication:
							OperationResult = ExpandedPreviousIndexValue * ExpandedCurrentIndexValue;
							break;
						case EPDRulesetOpType::EDivision: 
							OperationResult = ExpandedPreviousIndexValue / ExpandedCurrentIndexValue;
							break;
						case EPDRulesetOpType::EPower: 
							OperationResult = FMath::Pow(ExpandedPreviousIndexValue, CurrentOpBaseValue);
							break;
					}
				}

			}
			return OperationResult;
		};	
};

/**
 * @brief
 * @note For example: A Rule-Set TagID may be something like 'Progression.RuleSet.DnD'
 * @note For example: A Rule-Set TagID may be something like 'Progression.RuleSet.ObsidianPoE'
 * @note For example: A Rule-Set TagID may be something like 'Progression.RuleSet.Custom0'
 * @note For example: A Rule-Set TagID may be something like 'Progression.RuleSet.Custom1' */
USTRUCT(Blueprintable)
struct PDBASEPROGRESSION_API FPDRulesetRow : public FTableRowBase
{
	GENERATED_BODY()

	/* @brief @todo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag RuleSetTagID;

	/* @brief @todo */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPDRulesetOperatorStruct> RuleSetOperation;
};

/** @brief For example: A Stacking Context TagID may be something like 'Progression.StackingContext.Gear' */
USTRUCT(Blueprintable)
struct PDBASEPROGRESSION_API FPDAllowedStackingContexts

{
	GENERATED_BODY()

	/** @brief Key: StackingContextTagID,  Value: FPDRuleSetTableRow  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (RequiredAssetDataTags="RowStructure=/Script/PDBaseProgression.FPDRuleSetRow"))
	TMap<FGameplayTag, FDataTableRowHandle> ContextList; 
	
};

/**
Business Source License 1.1

Parameters

Licensor:             Ario Amin (@ Permafrost Development)
Licensed Work:        RTSOpen (Source available on github)
                      The Licensed Work is (c) 2024 Ario Amin (@ Permafrost Development)
Additional Use Grant: You may make free use of the Licensed Work in a commercial product or service provided these three additional conditions as met; 
                      1. Must give attributions to the original author of the Licensed Work, in 'Credits' if that is applicable.
                      2. The Licensed Work must be Compiled before being redistributed.
                      3. The Licensed Work Source may be linked but may not be packaged into the product or service being sold
                      4. Must not be resold or repackaged or redistributed as another product, is only allowed to be used within a commercial or non-commercial game project.
                      5. Teams with yearly budgets larger than 100000 USD must contact the owner for a custom license or buy the framework from a marketplace it has been made available on.

                      "Credits" indicate a scrolling screen with attributions. This is usually in a products end-state

                      "Package" means the collection of files distributed by the Licensor, and derivatives of that collection
                      and/or of those files..   

                      "Source" form means the source code, documentation source, and configuration files for the Package, usually in human-readable format.

                      "Compiled" form means the compiled bytecode, object code, binary, or any other
                      form resulting from mechanical transformation or translation of the Source form.

Change Date:          2028-04-17

Change License:       Apache License, Version 2.0

For information about alternative licensing arrangements for the Software,
please visit: https://permadev.se/

Notice

The Business Source License (this document, or the “License”) is not an Open
Source license. However, the Licensed Work will eventually be made available
under an Open Source License, as stated in this License.

License text copyright (c) 2017 MariaDB Corporation Ab, All Rights Reserved.
“Business Source License” is a trademark of MariaDB Corporation Ab.

-----------------------------------------------------------------------------

Business Source License 1.1

Terms

The Licensor hereby grants you the right to copy, modify, create derivative
works, redistribute, and make non-production use of the Licensed Work. The
Licensor may make an Additional Use Grant, above, permitting limited
production use.

Effective on the Change Date, or the fourth anniversary of the first publicly
available distribution of a specific version of the Licensed Work under this
License, whichever comes first, the Licensor hereby grants you rights under
the terms of the Change License, and the rights granted in the paragraph
above terminate.

If your use of the Licensed Work does not comply with the requirements
currently in effect as described in this License, you must purchase a
commercial license from the Licensor, its affiliated entities, or authorized
resellers, or you must refrain from using the Licensed Work.

All copies of the original and modified Licensed Work, and derivative works
of the Licensed Work, are subject to this License. This License applies
separately for each version of the Licensed Work and the Change Date may vary
for each version of the Licensed Work released by Licensor.

You must conspicuously display this License on each original or modified copy
of the Licensed Work. If you receive the Licensed Work in original or
modified form from a third party, the terms and conditions set forth in this
License apply to your use of that work.

Any use of the Licensed Work in violation of this License will automatically
terminate your rights under this License for the current and all other
versions of the Licensed Work.

This License does not grant you any right in any trademark or logo of
Licensor or its affiliates (provided that you may use a trademark or logo of
Licensor as expressly required by this License).

TO THE EXTENT PERMITTED BY APPLICABLE LAW, THE LICENSED WORK IS PROVIDED ON
AN “AS IS” BASIS. LICENSOR HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS,
EXPRESS OR IMPLIED, INCLUDING (WITHOUT LIMITATION) WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT, AND
TITLE.

MariaDB hereby grants you permission to use this License’s text to license
your works, and to refer to it using the trademark “Business Source License”,
as long as you comply with the Covenants of Licensor below.

Covenants of Licensor

In consideration of the right to use this License’s text and the “Business
Source License” name and trademark, Licensor covenants to MariaDB, and to all
other recipients of the licensed work to be provided by Licensor:

1. To specify as the Change License the GPL Version 2.0 or any later version,
   or a license that is compatible with GPL Version 2.0 or a later version,
   where “compatible” means that software provided under the Change License can
   be included in a program with software provided under GPL Version 2.0 or a
   later version. Licensor may specify additional Change Licenses without
   limitation.

2. To either: (a) specify an additional grant of rights to use that does not
   impose any additional restriction on the right granted in this License, as
   the Additional Use Grant; or (b) insert the text “None”.

3. To specify a Change Date.

4. Not to modify this License in any other way.
 **/