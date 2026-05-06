# RTSOpen
RTSOpen is a source-available BSL(1.1) project for developing tools and systems for an RTS game base anyone can bootstrap their games on.
The license has custom usage rights that permit *most people to use this in a commercial game project if appropriate credits are given and if the source code is not redistributed outside of compiled form.
> *Persons and studios with any access to finances above 100,000 USD per year need to read the additional grants section in the license for further details

## Notes
Updated the readme to better reflect the current state of the system, although it is still incomplete, so I will update the readme some more in the coming days (5th May 2026).

# Custom Plugins (Base logic)
> ____________
>
> ## ![Shared UI](https://img.shields.io/badge/Shared%20UI-AA4D2B?style=for-the-badge)
> ### STATUS: DONE
>> #### Base Classes
>> - Button Base
>> - Dialog Base
>> - Floating-point and Integral Number Boxes
>
> ____________

<br></br>

> ____________
>
> ## User Message System
> ### STATUS: PARTIALLY IMPLEMENTED, BACKLOGGED
>> #### Replication
>> - Network manager to keep load ligther when there are many recipients and senders
>> - 'Fast arrays'
>> - Subsystem to register network managers and to route messages being sent
>
> ____________

<br></br>

> ____________
>
> ## Progression System
> ### STATUS: IN PROGRESS (Mostly done but never tested and fully iterated)
>> #### Datadriven Progression Base Module
>> ##### STATUS: MOSTLY DONE
>> - Configurable rulesets and a ruleset evaluator. Idea is so the designers cna implement custom progression rulesets or recreate existing well-known rulesets in their project
>> - Configurable Skills & Skilltrees
>> - Configurable level scaling and stat modifier rules
>> - Replicated progression component and progression datums (fastarrays again)
>> - Engine level Subsystem that maps our data from the datatable so we can access it at O(1) when searching for specific default data
>> - Base UMG widgets for developers to bootstrap from or to use as example for progression related widgets
>> ____________
>> #### PDProgress to GAS layer
>> ##### STATUS: TODO, BACKLOGGED
>> - 
>> - 
>> ____________
>> #### PDProgress to MASS layer
>> ##### STATUS: TODO, BACKLOGGED
>> - 
>> - 
>> ____________
> ##### TODO: Need to finish up the ruleset code, it was mostly done but was a long time ago since I wrote it so I will need to investigate first 
> ##### TODO: Need to assess if this needs more substantial additions. Not very likely. I do vaguely remembe
> 
> ____________

<br></br>

> ____________
>
> ## Interaction System
> ### STATUS: MOSTLY DONE
>> - Configurable tracer for interactions or other purposes
>> - Base interact actor, Interact component, and Interact interface
>> - Base UMG widgets for developers to bootstrap from or to use as example for progression related widgets
> ##### TODO: Need to assess if this needs more substantial additions 
>
> ____________

<br></br>

> ____________
>
> ## Inventory System
> ### STATUS: MOSTLY DONE
>> #### Base
>> ##### STATUS: DONE
>> - Configurable item definitions via datatable
>> - Inventory Fragment (MassFragment) 
>> - Inventory Component (UObject)
>> ____________
>> #### Crafting
>> ##### STATUS: DONE
>> - Recipes
>> - Cost management
>> ____________
>> #### Replication
>> ##### STATUS: DONE
>> - Replicating items and item counts (fastarrays again) for the inventory component. Inventory Fragment is currently not being replicated iirc
>> ____________
> ##### TODO: Need to assess if this needs more substantial additions
> 
> ____________

<br></br>

> ____________
>
> ## RTSBase (Mass entity based system) 
> ### STATUS: MOSTLY DONE
> 
>> #### Camera Manager
>> ##### STATUS: DONE
>> - Applies different camera presets/setups using 'FPDCameraManagerSettings'
>> - Allows for custom presets/setups (Has a default base RTS preset )
>> ____________
>> #### Fog of War system 
>> ##### STATUS: IN PROGRESS
>> - Applies different camera presets/setups using 'FPDCameraManagerSettings'
>> - Allows for custom presets/setups (Has a default base RTS preset )
>> ____________
>> #### Octree Subsystem
>> ##### STATUS: DONE
>> - Generates custom octree nodes and assigns them to entities
>> - custom octree nodes that packs certain entity data
>> - Subsystem to help usage
>> ____________
>> #### Hashgrid Subsystem
>> ##### STATUS: DONE
>> - Calculates dynamic hashgrid cells
>> - Recalculates locations to dynamic hashgrid mapping
>> ____________
>> #### Pinger subsystem
>> ##### STATUS: DONE
>> - Scans hashgrid for entities to ping
>> - Pings JobTag to entities to target given actor (for now interactable buildings)
>> ____________
>> #### Builder subsystem
>> ##### STATUS: MOSTLY DONE 
>> - Uses hashgrid to force world steps for build system
>> - Tracks built actors and their owners
>> - Caches build system recipes and has helper functions to get default build item datum
>> ##### TODO: Need to assess if this needs more substantial additions 
>> ____________
>> #### Mass & Statetree
>> ##### STATUS: PARTIALLY DONE 
>> - Custom Mass Tasks, Processors, Evaluators, Fragments, Traits
>> - Custom State tree behaviour schemas
>> ##### TODO: Still need to revise some of the processors and test the vertex animations
>> ____________
>> #### Interfaces
>> - Ghost building interface, used by actors that wants to be managed by the builder system
>> - Builder Interface, used by actors that wants to be able to build other actors
>
> ____________

<br></br>

> ____________
>
> ## Conversation and Mission System
> ### STATUS: PARTIALLY DONE, BACKLOGGED
>> - Uses Unreal Conversation
>> - Mission editor classes are halfway implemented in another repo, will move here when this is not backlogged and continue work then
>> ##### TODO: Need to move this over to PDOpen and to wrap up the mission editor code in PDOpen and port it over here when done.
>
> ____________

<br></br>

> ____________
>
> ## Tutorial System
> ### STATUS: BARELY IMPLEMENTED, BACKLOGGED
>
> ____________

<br></br>

# Game Module
> ## RTSOpen
> ### STATUS: PARTIALLY DONE, BACKLOGGED

<br></br>

> ____________
>
> ### Core
> ##### STATUS: IN PROGRESS
> <br></br>
>> #### Player
>> ##### STATUS: MOSTLY DONE
>>> - GodHand player pawn (Basic godhand concept. Uses 'IPDRTSBuilderInterface, IRTSOInputInterface, IRTSOConversationInterface')
>>> - Player Controller (Uses 'IRTSOInputInterface, IPDRTSBuilderInterface, IRTSOActionLogInterface')
>> ##### TODO: Need to assess if this needs more substantial additions 
>> ____________
> <br></br>
>> #### Interaction - Game Module (Uses plugin base) 
>> ##### STATUS: MOSTLY DONE
>>> - Buildings
>>> - Resources
>>> - Conversation Handlers
>> ##### TODO: Need to assess if this needs more substantial additions 
>> ____________
> <br></br>
>> #### Mass & StateTree - Game Module (Uses plugin base) 
>> ##### STATUS: PARTIALLY DONE
>>> - Custom tasks that hooks into other systems, such as interaction or inventory
>>> - Custom fragments that uses data for other systems, such as interaction or inventory
>> ##### TODO: Need to assess if this needs more substantial additions. Very likely there is need for furhter tasks w.r.t specialized reource gathering or stacked build tasks 
>> ____________
> <br></br>
>> #### Conversation and Mission system - Game Module (Uses plugin base) 
>> ##### STATUS: PARTIALLY DONE
>>> - Tag based mission system. Tags serve as missions and as flags for objectives/sideobjectives states. 
>>> - Custom conversation actor that uses one of the interaction systems intrfaces and has itneraction logic that handles checking and granting for mission tags
>> ##### TODO: Need to move this over to PDOpen and to wrap up the mission editor code in PDOpen and port it over here when done.
>> ____________
> <br></br>
>> #### Mission/Quest UI - Game Module (Uses plugin base)  
>> ##### STATUS: IN PROGRESS, BACKLOGGED 
>>> - Custom mission view widget
>>> - Slate core with UMG wrapper
>>> - Mission List logic
>> ##### TODO: integrate with mission/quest plugin in PDOpen when getting back to that
>> ____________
> <br></br>
>> #### Game Mode & Instance
>> ##### STATUS: PARTIALLY DONE, BACKLOGGED
>>> - Handles login player controller flow and loads in their buildings if they have any
>>> - Handles level transitions
>>> - Handles Autosaving logic
>> ##### TODO: Single player and custom server matchmaking is the plan, and official servers I guess could run game with anti-cheat? not sure yet, but a login flow and loading logic is needed for the p2p part, the single player part stores the savefile on 
>> ##### TODO: Very likely I will need to add more login related code and loading more player data than just buildings () 
>> ____________
> <br></br>
>> #### HUD
>> ##### STATUS: PARTIALLY DONE
>>> - Draws selection marquee
>>> - Minimap logic (Deprecated, moved to compute shaders) 
>> ##### TODO: Need to assess if this needs more substantial additions. Very likely there is need for it later down the line
>> ____________
> <br></br>
>> #### Main Menu
>> ##### STATUS: MOSTLY DONE
>>> - Settings Menu
>>> - Save/Load Menu
>>> - Minimap (PARTIALLY DONE)
>>> - Mission Menu (PARTIALLY DONE, BACKLOGGED)
>> ##### TODO: Need to assess if this needs more substantial additions. Somewhat likely there is need for it later down the line
>> ____________
> <br></br>
>> #### User settings 
>> ##### STATUS: MOSTLY DONE, BACKLOGGED
>>> - Mix of slate and UMG to generate widget slots for each settings type
>>> - Supports POD data settings types, String settings type, Enum/String selector settings types, vector settings types
>>> - Settings bindable to actual in-game data, as to make thigns more plug and play an avoid having hardocded edge cases for each binding
>> ##### TODO: Need to fix a darn bug I notced when overriding the slate widget for certain overriden boolean members (specifically those that via a checkbox and control access to other variables)
>> ____________
> <br></br>
>> #### Save Editor 
>> ##### STATUS: MOSTLY DONE, BACKLOGGED
>>> - Mix of slate and UMG
>>> - Reads the current save data and allows for live modification within the game
>>> ##### NOTE: Could be used as a crude modding tool as it allows adding and removing data from the savefile
>>> ##### TODO: Update UI, looks horrendous. Alos while at it add import and export from custom binary format and also json format (With this many entities I might need to be creative with the Json structure so I can cram alot of data into few fields) 
>
> ____________

<br></br>

> ____________
>
> ### Misc
>> #### Tag Loader
>> ##### STATUS: DONE
>>> - Injects user defined strings and generates tags at game startup. Meant ot be used to allowing mods that target different systems liek the inventory system, skill system etc
>> ____________
> <br></br>
>> #### Game UI Misc
>> ##### STATUS: PARTIALLY DONE
>>> - Build system HUD View buttons (mostly slate)
>>> - Conversation system HUD View Elements (mostly slate)
>> ##### TODO: Need to assess if this needs more substantial additions. Somewhat likely there is need for it later down the line
>> ____________
> <br></br>
>> #### Input stack
>> ##### STATUS: DONE
>>> - Stacks input for IA actions, mainly due to a bug in 'Enhanced Input' causing input data to be reset 
>
> ____________

<br></br>

> ____________
>
> ## RTSShaders
> ### STATUS: PARTIALLY DONE
>> ### Global shader mapping
>> - Global Minimap splatter shader (used for splatting mass entities unto a RT texture, need to reuse for other data I want on the minimap RT texture)
>> ##### TODO: I have more texture shaders and compute shaders I want to write and make use of for other data visualization w.r.t. entities 
>
> ____________
