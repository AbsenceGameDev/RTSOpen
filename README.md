# RTSOpen
RTSOpen is a source-available BSL(1.1) project for developing tools and systems for an RTS game base anyone can bootstrap their games on.
The license has custom usage rights that permit *most people to use this in a commercial game project if appropriate credits are given and if the source code is not redistributed outside of compiled form.
> *Persons and studios with any access to finances above 100,000 USD per year need to read the additional grants section in the license for further details

![beta](https://shieldcn.dev/badge/status-beta-blue.svg?variant=outline) ![version](https://shieldcn.dev/badge/version-0.1.0-blue.svg?variant=secondary)  ![license](https://shieldcn.dev/badge/license-BSL%201.1-green.svg?variant=outline)


## Notes
Updated the readme to better reflect the current state of the system, although it is still incomplete, so I will update the readme some more in the coming days (6th May 2026).

# <img src="https://img.shields.io/badge/Custom%20Base%20Plugins-blueviolet?style=for-the-badge" width="1000" height="200" />

### <img src="https://img.shields.io/badge/Shared%20UI-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_inprog_backlogged]
| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" /> |  |  |  |
| :---        | :---        |    :---   |          :--- |
| Sections | ![badge](https://shieldcn.dev/badge/Button-Base-violet.png) | ![badge](https://shieldcn.dev/badge/Dialog-Base-violet.png)  | ![badge](https://shieldcn.dev/badge/NumberBoxes-Base-violet.png) |
| Status    |   ![status_done] |  ![status_done]  |  ![status_mostly_done_backlogged] |
| Description |       |       | <h6>Integral & Floating point number boxes</h6> |

______
<br></br>

### <img src="https://img.shields.io/badge/User%20Message%20System-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_mostly_done_backlogged]
| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" /> |  |  |  |
| :---        | :---        |    :---   |          :--- |
| Sections    | ![badge](https://shieldcn.dev/badge/Network-Managers-violet.png) | ![badge](https://shieldcn.dev/badge/Network-Fast%20Arrays-violet.png) | ![badge](https://shieldcn.dev/badge/Network-Subsystem-violet.png) |
| Status    |   ![status_mostly_done_backlogged] |  ![status_mostly_done_backlogged]  |  ![status_mostly_done_backlogged] |
| Description | <h6>Keep network load ligther when there are many recipients and senders</h6> | | <h6>Registers network managers to route RX/TX messages</h6> |

______
<br></br>

### <img src="https://img.shields.io/badge/Progression%20System-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_mostly_done]

| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |  |  |  |  |
| :---        | :---        |    :---   |          :--- |    :---   |          :--- |           :--- |
| Sections    | ![badge](https://shieldcn.dev/badge/Rulesets-violet.png) | ![badge](https://shieldcn.dev/badge/Skills-violet.png) | ![badge](https://shieldcn.dev/badge/Levels-violet.png)  | ![badge](https://shieldcn.dev/badge/Replication-violet.png) | ![badge](https://shieldcn.dev/badge/Subsystem-violet.png) | ![badge](https://shieldcn.dev/badge/Widgets-violet.png)  | 
| Status      | ![status_mostly_done] | ![status_done] | ![status_done] | ![status_done] | ![status_done] | ![status_done] |
| Description | <h6> Configurable rulesets and a ruleset evaluator. Designers can implement custom progression rulesets or recreate existing well-known rulesets </h6> | <h6> Configurable Skills & Skilltrees </h6> | <h6> Configurable level scaling and stat modifier rules </h6> | <h6> Replicated progression component and progression datums (fastarrays again) </h6> | <h6> Engine level Subsystem that maps our data from the datatable so we can access it at O(1) when searching for specific default data </h6> | <h6> Base UMG widgets for developers to bootstrap from or to use as example for progression related widgets </h6> |

| <img src="https://shieldcn.dev/badge/GAS%20Layer.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto%; height:40px;" />  |   |  | <img src="https://shieldcn.dev/badge/MASS%20Layer.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |
| :---        | :---        | :--- | :---        | :---        |
| Sections    | ![badge](https://shieldcn.dev/badge/Routing%20Layer-violet.png) |  | Sections    | ![badge](https://shieldcn.dev/badge/Routing%20Layer-violet.png) |
| Status      |  ![status_todo_backlogged] |  | Status      |  ![status_todo_backlogged] | 
| Description |             |  | Description |             | 

> ##### TODO: Need to finish up the ruleset code, it was mostly done but was a long time ago since I wrote it so I will need to investigate first 
> ##### TODO: Need to assess if this needs more substantial additions. Not very likely. I do vaguely remembe

______
<br></br>

### <img src="https://img.shields.io/badge/Interaction%20System-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_mostly_done]
| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" /> |  |  |  |
| :---        | :---        |    :---   |          :--- |
| Sections    | ![badge](https://shieldcn.dev/badge/Tracers-violet.png) | ![badge](https://shieldcn.dev/badge/Objects-violet.png) | ![badge](https://shieldcn.dev/badge/Widgets-violet.png)  |
| Status      |  ![status_done] | ![status_done] | ![status_done] |
| Description |   <h6>Configurable tracer for interactions or other purposes</h6>   |   <h6>Base interact actor, Interact component, and Interact interface</h6>   |   <h6>Base UMG widgets for developers to bootstrap from or to use as example for progression related widgets</h6>   |

> ##### TODO: Need to assess if this needs more substantial additions 

______
<br></br>


### <img src="https://img.shields.io/badge/Inventory%20System-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_mostly_done]

<!-- Inventory: Core -->
| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |  |
| :---        | :---        |    :---   |          :--- |
| Sections    | ![badge](https://shieldcn.dev/badge/Inventory-Items-violet.png) | ![badge](https://shieldcn.dev/badge/Inventory-Fragment-violet.png) | ![badge](https://shieldcn.dev/badge/Inventory-Component-violet.png)  |
| Status      |  ![status_done] | ![status_done] | ![status_done] |
| Description |   <h6>Configurable item definitions via datatable</h6>   |   <h6>MassFragment base, can interact with inventory components</h6>   |   <h6>UObject component base, can interact with inventory fragments</h6>   |

<!-- Inventory: Crafting & Replication -->
| <img src="https://shieldcn.dev/badge/Crafting.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |  | <img src="https://shieldcn.dev/badge/Replication.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |
| :---        | :---        |    :---   | :--- | :---        | :---        |
| Sections    | ![badge](https://shieldcn.dev/badge/Recipes-violet.png) | ![badge](https://shieldcn.dev/badge/Cost%20management-violet.png) |  | Sections    | ![badge](https://shieldcn.dev/badge/Item%20Replication-violet.png) |
| Status      |  ![status_done] | ![status_done] |  | Status      |  ![status_done] | 
| Description |   <h6>N/A</h6>   |   <h6>N/A</h6>   |   | Description |   <h6>Replicating items and item counts (fastarrays again) for the inventory component.<br>Inventory Fragment is currently not being replicated iirc</h6>   |   

> ##### TODO: Need to assess if this needs more substantial additions

______
<br></br>


### <img src="https://img.shields.io/badge/RTS%20Base-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_mostly_done]

| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |  |  |  |
| :---        | :---        |    :---   |          :--- | :---        |    :---   |
| Sections    | ![badge](https://shieldcn.dev/badge/Subsystem-Octree-violet.png)  | ![badge](https://shieldcn.dev/badge/Subsystem-Hashgrid-violet.png) | ![badge](https://shieldcn.dev/badge/Subsystem-Pinger-violet.png) | ![badge](https://shieldcn.dev/badge/Subsystem-Builder-violet.png)  | ![badge](https://shieldcn.dev/badge/Mass%20Statetree-violet.png) |
| Status      |  ![status_done] | ![status_done] | ![status_done] | ![status_mostly_done] | ![status_inprog] |
| Description |   <h6> - Generates custom octree nodes and assigns them to entities <br> - custom octree nodes that packs certain entity data <br> - Subsystem to help usage</h6>   |   <h6> - Calculates dynamic hashgrid cells <br> - Recalculates locations to dynamic hashgrid mapping</h6>   |   <h6> - Scans hashgrid for entities to ping <br> - Pings JobTag to entities to target given actor (for now interactable buildings)</h6>   |   <h6> - Uses hashgrid to force world steps for build system <br> - Tracks built actors and their owners <br> - Caches build system recipes and has helpers</h6>   |   <h6> - Custom Mass Tasks, Processors, Evaluators, Fragments, Traits <br> - Custom State tree behaviour schema</h6>   |  



| <img src="https://shieldcn.dev/badge/Ext.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |  |
| :---        | :---        |    :---   |          :--- |
| Sections    | ![badge](https://shieldcn.dev/badge/Manager-Camera-violet.png) | ![badge](https://shieldcn.dev/badge/Subsystem-FoW-violet.png) | ![badge](https://shieldcn.dev/badge/Interfaces-violet.png)  |
| Status      |  ![status_done] | ![status_inprog] | ![status_done] |
| Description |   <h6> - Applies different camera presets/setups using 'FPDCameraManagerSettings' <br> - Allows for custom presets/setups (Has a default base RTS preset ) </h6>   |   <h6> - UPDATEME <br> - UPDATEME</h6>   |   <h6> - Ghost building interface, used by actors that wants to be managed by the builder system <br> - Builder Interface, used by actors that wants to be able to build other actors </h6> |   

______
<br></br>


### <img src="https://img.shields.io/badge/Mission%20System-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_inprog_backlogged]
| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |  |
| :---        | :---        |    :---   |          :--- |
| Sections    | ![badge](https://shieldcn.dev/badge/Unreal%20Conversation-violet.png) | ![badge](https://shieldcn.dev/badge/Mission%20Editor-violet.png) |
| Status      |  ![status_done] | ![status_inprog_backlogged] |
| Description |   <h6>Bootstrapped from Unreal Conversation </h6>   |   <h6>Mission editor classes are halfway implemented in another repo, will move here when this is not backlogged and continue work then</h6>   |
> ##### TODO: Need to move this over to PDOpen and to wrap up the mission editor code in PDOpen and port it over here when done.

______
<br></br>

### <img src="https://img.shields.io/badge/Tutorial%20System-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
### ![status_todo_backlogged]
| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |
| :---        | :---        |    :---   |
| Sections    | ![badge](https://shieldcn.dev/badge/Manager-violet.png) | ![badge](https://shieldcn.dev/badge/Widgets-violet.png) | 
| Status      |  ![status_todo_backlogged] | ![status_todo_backlogged] | 
| Description |   <h6></h6>   |   <h6></h6>   |  

______
<br></br>


<!-- Thoughts/Pseudotable:
| <img src="https://shieldcn.dev/badge/Core.png?variant=ghost&size=lg&color=3e4f6c&labelOpacity=1" style="width:auto; height:40px;" />  |  |  |  |
| :---        | :---        |    :---   |          :--- |
| Sections    | ![badge](https://shieldcn.dev/badge/XXXXX-XXXXXX-violet.png) | ![badge](https://shieldcn.dev/badge/XXXXX-XXXXXX-violet.png) | ![badge](https://shieldcn.dev/badge/XXXXX-XXXXXX-violet.png)  |
| Status      |  ![status_done] | ![status_done] | ![status_done] |
| Description |   <h6></h6>   |   <h6></h6>   |   <h6></h6>   |
-->


# <img src="https://img.shields.io/badge/Game%20Base%20Modules%20%20-blueviolet?style=for-the-badge" width="1000" height="200px" />

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20Player-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_mostly_done]
>> - GodHand player pawn (Basic godhand concept. Uses 'IPDRTSBuilderInterface, IRTSOInputInterface, IRTSOConversationInterface')
>> - Player Controller (Uses 'IRTSOInputInterface, IPDRTSBuilderInterface, IRTSOActionLogInterface')
> ##### TODO: Need to assess if this needs more substantial additions 

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20Interaction-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_mostly_done]
>> - Buildings
>> - Resources
>> - Conversation Handlers
> ##### TODO: Need to assess if this needs more substantial additions 

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20Mass-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_inprog]
>> - Custom tasks that hooks into other systems, such as interaction or inventory
>> - Custom fragments that uses data for other systems, such as interaction or inventory
> ##### TODO: Need to assess if this needs more substantial additions. Very likely there is need for furhter tasks w.r.t specialized reource gathering or stacked build tasks 

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20Missions-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_inprog]
>> - Tag based mission system. Tags serve as missions and as flags for objectives/sideobjectives states. 
>> - Custom conversation actor that uses one of the interaction systems intrfaces and has itneraction logic that handles checking and granting for mission tags
>> - Custom mission view widget
>> - Slate core with UMG wrapper
>> - Mission List logic
> ##### TODO: integrate with mission/quest plugin in PDOpen when getting back to that
> ##### TODO: Need to move this over to PDOpen and to wrap up the mission editor code in PDOpen and port it over here when done.

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20Game-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_inprog_backlogged]
>> - Handles login player controller flow and loads in their buildings if they have any
>> - Handles level transitions
>> - Handles Autosaving logic
> ##### TODO: Single player and custom server matchmaking is the plan, and official servers I guess could run game with anti-cheat? not sure yet, but a login flow and loading logic is needed for the p2p part, the single player part stores the savefile on 
> ##### TODO: Very likely I will need to add more login related code and loading more player data than just buildings () 

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20HUD-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_inprog]
>> - Draws selection marquee
>> - Minimap logic (Deprecated, moved to compute shaders) 
> ##### TODO: Need to assess if this needs more substantial additions. Very likely there is need for it later down the line

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20Main%20Menu-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_mostly_done]
>> - Settings Menu
>> - Save/Load Menu
>> - Minimap (PARTIALLY DONE)
>> - Mission Menu (PARTIALLY DONE, BACKLOGGED)
> ##### TODO: Need to assess if this needs more substantial additions. Somewhat likely there is need for it later down the line

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20User%20Settings-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_mostly_done_backlogged]
>> - Mix of slate and UMG to generate widget slots for each settings type
>> - Supports POD data settings types, String settings type, Enum/String selector settings types, vector settings types
>> - Settings bindable to actual in-game data, as to make thigns more plug and play an avoid having hardocded edge cases for each binding
> ##### TODO: Need to fix a darn bug I notced when overriding the slate widget for certain overriden boolean members (specifically those that via a checkbox and control access to other variables)

<br></br>

> ## <img src="https://img.shields.io/badge/Core-%20Save%20Editor-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_mostly_done_backlogged]
>> - Mix of slate and UMG
>> - Reads the current save data and allows for live modification within the game
>> ##### NOTE: Could be used as a crude modding tool as it allows adding and removing data from the savefile
>> ##### TODO: Update UI, looks horrendous. Alos while at it add import and export from custom binary format and also json format (With this many entities I might need to be creative with the Json structure so I can cram alot of data into few fields) 

<br></br>

> ## <img src="https://img.shields.io/badge/Misc-%20Tag%20Loader-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_done]
>> - Injects user defined strings and generates tags at game startup. Meant ot be used to allowing mods that target different systems liek the inventory system, skill system etc

<br></br>

> ## <img src="https://img.shields.io/badge/Misc-%20Game%20UI-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ##### ![status_inprog]
>> - Build system HUD View buttons (mostly slate)
>> - Conversation system HUD View Elements (mostly slate)
> ##### TODO: Need to assess if this needs more substantial additions. Somewhat likely there is need for it later down the line

<br></br>

> ## <img src="https://img.shields.io/badge/Misc-%20Input%20Stack-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ## ![status_done]
>> - Stacks input for IA actions, mainly due to a bug in 'Enhanced Input' causing input data to be reset 

<br></br>

> ## <img src="https://img.shields.io/badge/RTSShaders-%20Global-AA4D2B?style=for-the-badge" style="width:auto; height:50px;" />
> ### ![status_inprog]
>> ### Global shader mapping
>> - Global Minimap splatter shader (used for splatting mass entities unto a RT texture, need to reuse for other data I want on the minimap RT texture)
>> ##### TODO: I have more texture shaders and compute shaders I want to write and make use of for other data visualization w.r.t. entities 


<!---
     Shared UI
User Message System
Progression  System
Interaction System
 Inventory System
     RTSBase 
  Mission System
 Tutorial  System

  Core - Player
  Core - Interaction
  Core - Mission
  Core - 

Misc - UI
Misc - Input Stack
Misc - Tag Loader

RTSShaders
--->


[status_done]: https://img.shields.io/badge/STATUS-%20DONE-success?style=for-the-badge
[status_todo]: https://img.shields.io/badge/STATUS-%20TODO-inactive?style=for-the-badge
[status_inprog]: https://img.shields.io/badge/STATUS-%20IN%20PROGRESS-yellow?style=for-the-badge
[status_mostly_done]: https://img.shields.io/badge/STATUS-%20MOSTLY%20DONE-yellowgreen?style=for-the-badge

[status_todo_backlogged]: https://img.shields.io/badge/STATUS-%20BACKLOGGED:%20TODO-inactive?style=for-the-badge
[status_inprog_backlogged]: https://img.shields.io/badge/STATUS-%20BACKLOGGED:%20IN%20PROGRESS-important?style=for-the-badge
[status_mostly_done_backlogged]: https://img.shields.io/badge/STATUS-%20BACKLOGGED:%20MOSTLY%20DONE-informational?style=for-the-badge