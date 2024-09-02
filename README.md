No dependencies at the moment.

## Practise Script
Script made to help with practising hard maps.

How to set up:
* copy the desired map from levels to levels_src
* modify the checkpoints to your liking, I would heavily recommend adding a checkpoint to the very beginning (why is in oddities)
* add the script to the level
* fill in the checkpoints list with all the checkpoints of the level in order (starting from index 0) (putting show hitboxes on makes this a lot easier)
* set up the charts you want to have (might be easier after you have some data first) <br/>
note: for all colours except "colour of deaths" alpha is 0 by default so be sure to set it

How to use:
* start playing
* after you're done with the run (possibly including respawns, finished or not) tab back into editor and save the map, this will save the deaths to your map file

explanation of possibly non-clear features:
* "only keep x deaths" will get rid of older deaths such that the total amount is lower than x every time you go back to the editor, this is important as many thousands of deaths can start impacting game performance
* "p" in a chart means "%" because that is apparently not in the font
* "success rate at cp chart" shows what % of the runs that got to/started from that checkpoint beat it
* "highest cp reached chart" shows highest cp reached starting from this or an earlier cp without dying

oddities/possible confusions:
* if you tab back into editor while dying your data from that run will be lost so wait to respawn first
* you need to touch/respawn from a checkpoint not only for the script to know where you are but also for it to save anything due to technical reasons (this makes cp 0 a bit weird in the charts just ignore it if you're confused)
* as long as ^ is satisfied feel free to start where ever you please, the script will handle the rest
* completitions of the level are counted as deaths at checkpoint "total checkpoints + 1". for example this is what you'll see in highest cp reached when you beat the map

## Custom Collision

Allows the creation of arbitrary collision.

setup:
* add map/customCollision/2dCollisionEditor.cpp to your map
* set up play area. this is the intended area of the map, no collision will work outside of this. The surface area of this is directly proportional to the amount of load-in lag.
* to set it up, choose playAreaCorner as the topleft corner of playArea, and choose width and height. (10kx10k is fine to start with).
* you can see it's size with the showPlayArea toggle, though for this and showCacheDebug you need to save and reload script for the visualisation to be correct. (sry I'm lazy xD)
* choose a colour for custom spikes and or dust if you want to use them (make sure to set the alpha)
* this is better to do later when you can see the impact but I'll just explain what it means here since it's the last thing in this menu:
collisionOrder determines the grid size that is used for optimizing collision. without going into too much detail, lower means less lag during gameplay (caps at like 4 or so) but a bigger lagspike at load (exponential) and vice versa.
you can see the size of the grid with showCacheDebug though if you don't fully understand what it means then good luck :p

building:
* add collsion by assigning a scriptTrigger to quadEntity
* you can set settings in the left menu, and you can move the corners by clicking on them in the world view, no need to use the variables c:
* layer and sub_layer are the drawing layer, they don't affect collision. so if you make something a bg layer object the visuals will be misaligned from collsion.
* concave collision won't work!!!!!!

