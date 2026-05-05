# RTSOpen
RTSOpen is an source-available BSL(1.1) project for developing tools and systems for an RTS game base anyone can bootstrap their games on.
The license has custom usage rights witch permits anyone from using this in an commercial game project if appropriate credits are given and if the source code is not redistributed outside of compiled form.



## Notes
Updated the readme to better reflect the current state of the system, although still incomplete so will update the readme some more the coming days (5th May 2026)


# Custom Plugins (Base logic)

> ## Shared UI
> STATUS: DONE
>> #### Base Classes
>> - Button Base
>> - Dialog Base
>> - Floating-point and Integral Number Boxes
>
> ____________

> ## User Message System
> STATUS: PARTIALLY IMPLEMENTED, BACKLOGGED
>> #### Replication
>> - Network manager to keep load ligther when there are many recipients and senders
>> - 'Fast arrays'
>> - Subsystem to register network managers and to route messages being sent
>
> ____________

> ## Progression System
> STATUS: IN PROGRESS (Mostly done but never tested and fully iterated)
>> #### Datadriven Progression Base Module
>> - Configurable rulesets and a ruleset evaluator. Idea is so the designers cna implement custom progression rulesets or recreate existing well-known rulesets in their project
>> - Configurable Skills & Skilltrees
>> - Configurable level scaling and stat modifier rules
>> - Replicated progression component and progression datums (fastarrays again)
>> - Engine level Subsystem that maps our data from the datatable so we can access it at O(1) when searching for specific default data
>> - Base UMG widgets for developers to bootstrap from or to use as example for progression related widgets
>
>> #### PDProgress to GAS layer (Backlogged)
>> - TODO
>
>> #### PDProgress to MASS layer (Backlogged)
>> - TODO
>
> ____________


> ## Interaction System
> STATUS: MOSTLY DONE
>> - Configurable tracer for interactions or other purposes
>> - Base interact actor, Interact component, and Interact interface
>> - Base UMG widgets for developers to bootstrap from or to use as example for progression related widgets
>
> ____________

> ## Inventory System
> STATUS: MOSTLY DONE
>> #### Base
>> - Configurable item definitions via datatable
>> - Inventory Fragment (MassFragment) 
>> - Inventory Component (UObject)
>
>> #### Crafting
>> - Recipes
>> - Cost management
>
>> #### Replication
>> - Replicating items and item counts (fastarrays again) for the inventory component. Inventory Fragment is currently not being replicated iirc
>
> ____________

> ## RTSBase (Mass entity based system) 
> STATUS: MOSTLY DONE
> 
>> #### Octree Subsystem
>> - Generates custom octree nodes and assigns them to entities
>> - custom octree nodes that packs certain entity data
>> - Subsystem to help usage
>
>> #### Hashgrid Subsystem
>> - Calculates dynamic hashgrid cells
>> - Recalculates locations to dynamic hashgrid mapping
>
>> #### Pinger subsystem
>> - Scans hashgrid for entities to ping
>> - Pings JobTag to entities to target given actor (for now interactable buildings)
>
>> #### Builder subsystem
>> - Uses hashgrid to force world steps for build system
>> - Tracks built actors and their owners
>> - Caches build system recipes and has helper functions to get default build item datum
>
>> #### Mass & Statetree
>> - Custom Mass Tasks, Processors, Evaluators, Fragments, Traits
>> - Custom State tree behaviour schemas
>
>> #### Interfaces
>> - Ghost building interface, used by actors that wants to be managed by the builder system
>> - Builder Interface, used by actors that wants to be able to build other actors
>
> ____________

> ## Conversation and Mission System
> STATUS: PARTIALLY DONE, BACKLOGGED
>> - Uses Unreal Conversation
>> - Mission editor classes are halfway implemented in another repo, will move here when this is not backlogged and continue work then
>
> ____________

> ## Tutorial System
> STATUS: BARELY IMPLEMENTED, BACKLOGGED
>
> ____________

# Game Module
> ## RTSOpen
> STATUS: PARTIALLY DONE, BACKLOGGED
>> ### Core
>> #### Player
>>> - GodHand player pawn (Basic godhand concept. Uses 'IPDRTSBuilderInterface, IRTSOInputInterface, IRTSOConversationInterface')
>>> - Player Controller (Uses 'IRTSOInputInterface, IPDRTSBuilderInterface, IRTSOActionLogInterface')
>
>> #### Interaction - Game Module (Uses plugin base) 
>>> - Buildings
>>> - Resources
>>> - Conversation Handlers
>
>> #### Mass & StateTree - Game Module (Uses plugin base) 
>>> - Custom tasks that hooks into other systems, such as interaction or inventory
>>> - Custom fragments that uses data for other systems, such as interaction or inventory
>
>> #### Conversation and Mission system - Game Module (Uses plugin base) 
>>> - Tag based mission system. Tags serve as missions and as flags for objectives/sideobjectives states. Need to move this over to PDOpen and to wrap up the mission editor code in PDOpen and port it over here when done.
>>> - Custom conversation actor that uses one of the interaction systems intrfaces and has itneraction logic that handles checking and granting for mission tags
>
>> #### Mission/Quest UI - Game Module (Uses plugin base)  
>> STATUS: IN PROGRESS, BACKLOGGED 
>>> - Custom mission view widget
>>> - Slate core with UMG wrapper
>>> - Mission List logic
>>> @todo integrate with mission/quest plugin in PDOpen when getting back to that
>
>> #### Game Mode & Instance (IN PROGRESS, BACKLOGGED)
>>> - Handles login player controller flow and loads in their buildings if they have any
>>> - Handles level transitions
>>> - Handles Autosaving logic
>
>> #### HUD
>>> - Draws selection marquee
>>> - Minimap logic (Deprecated, moved to compute shaders) 
>
>> #### Main Menu
>>> - Settings Menu
>>> - Save/Load Menu
>>> - Minimap (PARTIALLY DONE)
>>> - Mission Menu (PARTIALLY DONE, BACKLOGGED)
>
>> #### User settings (IN PROGRESS, BACKLOGGED)
>>> - Mix of slate and UMG to generate widget slots for each settings type
>>> - Supports POD data settings types, String settings type, Enum/String selector settings types, vector settings types
>>> - Settings bindable to actual in-game data, as to make thigns more plug and play an avoid having hardocded edge cases for each binding
>
>> #### Save Editor (MOSTLY DONE, BACKLOGGED)
>>> - Mix of slate and UMG
>>> - Reads the current save data and allows for live modification within the game
>>> @note: Could be used as a crude modding tool as it allows adding and removing data from the savefile
>>> @todo: Update UI, looks horrendous. Alos while at it add import and export from custom binary format and also json format (With this many entities I might need to be creative with the Json structure so I can cram alot of data into few fields) 
>
>> ### Misc
>> #### Tag Loader
>>> - Injects user defined strings and generates tags at game startup. Meant ot be used to allowing mods that target different systems liek the inventory system, skill system etc
>
>> #### Game UI Misc
>>> - Build system HUD View buttons (mostly slate)
>>> - Conversation system HUD View Elements (mostly slate)
>
>> #### Input stack
>>> - Stacks input for IA actions, mainly due to a bug causing input data to be reset 
>
> ____________

> ## RTSShaders
> STATUS: DONE
>> ### Global shader mapping
>> - Global Minimap splatter shader (used for splatting mass entities unto a RT texture, need to reuse for other data I want on the minimap RT texture)
>
> ____________
