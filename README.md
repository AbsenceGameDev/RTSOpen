# RTSOpen
RTSOpen is an source-available BSL(1.1) project for developing tools and systems for an RTS game base anyone can bootstrap their games on.
The license has custom usage rights witch permits anyone from using this in an commercial game project if appropriate credits are given and if the source code is not redistributed outside of compiled form.



## Notes
Updated the readme to better reflect the current state of the system, although still incomplete so will update the readme some more the comings days (4th May 2026)


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
>> #### Octrees
>> -
>> -
>
>> #### Hashgrids
>> -
>> -
>
>> #### Pinger subsystem
>> -
>> -
>
>> #### Builder subsystem
>> -
>> -
>
>> #### Builder subsystem
>> -
>> -
>
> ____________

> ## Conversation and Mission System
> STATUS: PARTIALLY DONE, BACKLOGGED
>
> ____________

> ## Tutorial System
> STATUS: BARELY IMPLEMENTED, BACKLOGGED
>
> ____________

# Game Module
> ## RTSOpen
> STATUS: PARTIALLY DONE, BACKLOGGED
>> ### Input stack subsystem
>
>> ### User settings subsystem
>
>> ### Tag Loader subsystem
>
>> ### Game UI
>> - Build system HUD buttons (mostly slate)
>
>> ### Main Menu
>> - Settings Menu
>> - Save/Load Menu
>> - Minimap (PARTIALLY DONE)
>> - Mission Menu (PARTIALLY DONE, BACKLOGGED)
>
> ____________

> ## RTSShaders
> STATUS: DONE
>> ### Global shader mapping
>> - Global Minimap splatter shader (used for splatting mass entities unto a RT texture, need to reuse for other data I want on the minimap RT texture)
>
> ____________
