# RTSOpen
RTSOpen is an source-available BSL(1.1) project for developing tools and systems for an RTS game base anyone can bootstrap their games on.
The license has custom usage rights witch permits anyone from using this in an commercial game project if appropriate credits are given and if the source code is not redistributed outside of compiled form.



## TODO 

- PRIO 0 . CREATE PROGRESSION SYSTEM PLUGIN (PSEUDO-CODE/Design ideas)
- Stats (Progression meta-rules)

-- FPDRulesetOperatorStruct
- Properties:
- OpType           : (Example: 'EAddition', 'ESubtraction', 'EMultiplication', 'EDivision', 'EPower')
- OpTarget         : (Example: EThis, EOther, EStatic)
- OpFallbackValue  : (Example: If EThis or EOther, it will be ignored, if EStatic it will be the value used)
- PotentialInnerOp : FDataTableRowHandle RulesetHandle // Row Type: FPDRulesetOperatorStruct

-- FPDRulesetEvaluator
- Eval (const TArray<FPDRulesetOperatorStruct>& Ruleset,
const TValueType& PotentialSelfValue,
const TValueType& PotentialOtherValue,
const TValueType& PotentialStaticValue)

-- FPDRulesetTableRow
- FGameplayTag RuleSetTagID;
- TArray<FPDRulesetOperatorStruct> RuleSetOperation;
// For example: A Rule-Set TagID may be something like 'Progression.RuleSet.DnD'
// For example: A Rule-Set TagID may be something like 'Progression.RuleSet.ObsidianPoE'
// For example: A Rule-Set TagID may be something like 'Progression.RuleSet.Custom0'
// For example: A Rule-Set TagID may be something like 'Progression.RuleSet.Custom1'


-- FPDAllowedStackingContexts(Having )
- TMap<FGameplayTag (*StackingContextTagID*/, FDataTableRowHandle /* type: FPDRuleSetTableRow> ContextList */ >;

	// For example: A Stacking Context TagID may be something like 'Progression.StackingContext.Gear'

- Stats Overview
    - Enum  EPDProgressionBehaviourType { EClassic, EActionBased }
    - Enum  EPDProgressionType { EActiveEffect, EPassiveEffect, EStat}
    - If Actionbased,
        - Action Event Delegate signature: bool (void* OpaqueActionDataPacket){}
        - run an automated test-level before packaging to ensure an action event has been tied to the stat to increase it.

    - FPDStatsValue
        - TArray<int32> BaseValueRepresentations;
        - int32 BaseDivisor = 1;

    - FPDStatsRow
        - FGameplayTag ProgressionTag;
        - TMap<FGameplayTag /*StatTag*/, FGameplayTag /*RuleSetTag*/> RulesAffectedBy;
        - EPDProgressionBehaviourType BehaviourType;
        - EPDProgressionType ProgressionType;
        - FPDStatsValue ProgressionValueRepresentation;
        - int32 MaxLevel = 1;
        - UCurveFloat ExperienceCurve;

    - FPDStat : public FFastArraySerializerItem
        - FGameplayTag ProgressionTag;
        - int32 CurrentLevel = 0;
        - int32 CurrentExperience;

    - FPDStatList : public FFastArraySerializerItemList
        - TArray<FPDStat> Items;

    - FPDStatMapping
        - int32 Index = 0;
        - FGameplayTag Tag;
        - bool operator ==(const int32 OtherIndex) { return this->Index == OtherIndex; }
        - bool operator ==(const FGameplayTag& OtherTag) { return this->Tag == OtherTag; }

    - Skilltrees
        - Research tree is a skill tree which is applied to some other actor and controlled by the owning actor ID
        -

    - USTRUCT(Blueprintable) struct FPDProgressionClassRow : public FTableRowBase
      {
        UPROPERTY(EditAnywhere, BlueprintReadWrite)
        FGameplayTag Tag

      	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
      	TSet<FGameplayTag> DefaultStats;

      	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
      	TSet<FGameplayTag> DefaultActiveEffects;

      	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
      	TSet<FGameplayTag> DefaultPassiveEffects;

      	UPROPERTY(EditAnywhere, BlueprintReadWrite) 
      	TSet<FGameplayTag> GrantedTrees;
      };


	- UPDStatSubsystem
		- TMap<FGamplayTag, FPDProgressionClassRow*> ClassTypes
		- TMap<FGamplayTag, FPDStat> DefaultStatValues
		
	
	- FPDStatHandler
		- UFUNCTION(BlueprintNativeEvent) void DefaultFillStatList(const FGameplayTag& ClassTag);
		void DefaultFillStatList_Implementation(cons FGameplayTag& ClassTag)


		- UPROPERTY(Replicated) FPDStatList{};
		- TSet<FPDStatMapping> StatTagMappings;

		- IncreaseStatLevel()
		- IncreaseStatExperience()


- PRIO 0 - FINISH ENTITY IMPLEMENTATION,
  --- (TODO) PingSystem: UPDEntityPinger::Ping_Implementation: FallBackEntityTag = TAG_AI_Type_BuilderUnit_Novice ; // @todo, pass into here from somewhere else
  --- (TODO) BuildSystem: ARTSOBaseGM::PostLogin, //@todo For newly created characters randomly spawn a base. Transforms must be ensured to not overlap, possibly make use of the world hashgrid


- PRIO 0 : Currently the design is that on a server or single player, a save-game is used, on the server it is the source of truth of the world-state for the server and on single-player games it is the same,problem is that several UI's directly depend on the savegame now and in a server game it will not be available for the clients!!! Massive problem. Wil need to have some replication in place and an middleman of sorts 