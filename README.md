No dependencies at the moment.

## 3D Custom Collision

3d maps!
(I spent like 100-200h working on this so this better fucking catch on lol)

### Setup
Add 3dCollisionEditor to your map (you need all scripts in map/customCollision except 2dCollisionEditor). Don't copy from the map, I've already fixed a couple of important things.

### Important Limitations

Trigger load distance
Without going into unnecessary detail, being far away from the centre of the map is bad for various reasons. To remedy this I've made a hotkey, ctrl+x, which will warp you to the centre of the 2d coordinate system while keeping your 3d position the same. However because of the same limitation you can't do this too far out from the centre, and thus I've added an indicator to the bottom right that indicates how close to the edge you're getting. As a rule of thumb, whenever you see yellow just ctrl+x. If you're in the red you can't ctrl+x anymore.
If you go into play mode too far from the centre you're going to "lose your triggers" and everything will disappear. if this happens you can retrieve your triggers by going back to where you were when you lost them and then going back to the centre, but you should avoid this with this rule: ALWAYS CTRL+X BEFORE SAVING OR PLAYING, AND IF YOU DO LOSE YOUR TRIGGERS, DON'T SAVE BEFORE YOU'VE FOUND THEM AGAIN. Also to note, if you're making a map that mostly goes in 1 direction you might find that when you exit to the editor you're already in the red and finding the collision again is super annoying, so I'd recommend whenever you stop playing press tab 3 times to get back to the centre.

Lag
While I spent like 5 days just optimising this script, it will still get laggy if you make your map too big. I will discuss ways to minimise lag later, but for now just know to not expect to make super big maps, as the script just can't handle that many renderable things. A lot of the optimisations are for the mapmaker to utilize, so be sure to read the optimisation section at the end before you start making your map.

### Small Glossary

quad: a 3d 4 vertex shape with 4 triangular sides. This is not it's proper name but it's used so commonly I use this for abbreviation. This is the smallest building block of collision, similar to how in normal 3d contexts it's triangles. This is due to the need for everything to have volume.
layering/sorting: the act of putting all renderable things in the correct order to be drawn, the most significant cause of lag for the script
node cluster/NC: the main component used to build collision
a "flat": a billboard object, includes enemies and props

### Script Settings

Most of these are either self explanatory or the tooltip explains them, but a couple of notes:
* CollisionOrder and playAreaWidth/height settings are 2d optimisation settings, but since 99% of lag is caused by layering/nodecluster side enabling these are pretty much pointless, don't worry about them.
* Layer frames is the most significant way to reduce lag, though it can make things look janky and or broken if used excessively or the player stops rotating at just the wrong time. I would advice against putting this higher than 4 or 5.
* Qaud debug is highly recommended when building collision with nodeclusters.

### moving around

As long as middle mouse hotkeys are enabled in script, drag with middle mouse to turn, normal editor space+drag to move horisontally, shift+scroll to move "in and out" (very slow, meant for small scale movements, for big movements turn, move, turn).

### using the "triggers"

All objects in the script are added with a scripttrigger, though the moment you select the type of scripttrigger it should be the actual scripttrigger gets hidden and is replaced by a fake trigger. (this is due to the trigger load distance mentioned earlier). Importantly this means you can't interact with the triggers from the trigger menu. Instead you need to use the select menu. To move a trigger, drag it. To select a trigger, left click it. To delete a trigger, right click it. All triggers are 3d ones, which means that they are only visible in front of the camera and when you move them, they retain their distance from the camera and move in alignment with it.

### start pos

Just like in the normal game, if you place one when one already exists the original gets deleted. The position of the trigger decides starting pos, as well as starting pos in editor. To set starting rotation use script's startingRot. Important note: if you place the trigger such that the player snaps up the moment they spawn in this breaks things, as the first time the player rotates they will be teleported downwards. Due to this, make sure the start pos doesn't snap the player.

### end flag

With the end flag selected, shift left click the enemies you want to be dead for the map to end.´

### prop

To set up a prop you need to give it a sprite. sprite set names, sprite names and palettes can be found here:
https://github.com/cmann1/PropUtils/blob/master/files/sprite_reference/props1.png
note 1: you can give this any sprite you want it doesn't have to be a prop
note 2: if the sprite happens to be centred somewhere else than the centre, such as tree trunks, they will not work correctly. I'm sorry but I can not be fucked to fix this, if someone else wants to do it be my guest.
note 3: layering does not take the rotation value into account!!!

### enemies

The triggers starting with d3E are enemies. At the time of writing there are only 3, but you can add more really easily as long as you like how my implementations work (see extending section)

### text trigger

AVOID USING THESE!!!! Or at the very least add them as the very last thing and back up your world before adding them. Since they carry an actual entity (the text trigger), if you lose your triggers they are going to make a new one, meaning you'll have to rewrite the text, leave the old one god knows where and players might accidentally run into it.

In editor, when selected and you're in it's range it will move it's text trigger in the middle of the screen, you can then edit that text trigger's text.

### node cluster

This is the big one, this is how you make standard collision. The location of the trigger is arbitrary, it's only purpose is to select the cluster.
The NC consists of nodes (shown as pink squares, or blue when selected) and quads set between these nodes.
note: all nodes are always visible when the NC is selected, nodes behind the camera are translucent.

Keybinds (all of these only apply with the trigger selected)

shift left click on node: selects node
shift right click on node: deselects node
lclick drag: moves all selected nodes. if no nodes are selected, move entire NC
delete (the key): delete selected nodes
esc: deselects all nodes
+: add node at cursor
shift A: if no nodes are selected, add node at cursor. if exactly 4 nodes are selected, add quad between these nodes
shift D: delete closest quad under cursor
R: rotate NC. Follow with x, y or z to select axis. stop rotating with r or left click
S: scale NC. Use as is for uniform scaling, or follow with x y or z to select axis. Stop scaling with s or left click
Q: add/remove spikes on closest visible side
W: add/remove dust on closest visible side
E: activate/deactivate closest "visible" side (not visible when deactive obviously)
presets: currently keys 1-3 (more could be easily added, see extending section) allow you to choose a node preset after adding the NC. This is disabled the moment you change something about the NC (move a node, add a quad, ect.)
ctrl J: for every node of the NC, look through all other NCs' nodes and if one is within join distance (in script settings), set this node's position equal to the other's. note: this is a one time thing, if either is moved joining will have to be redone. Whenever you join 2 NCs, make sure to disable all the faces that are between them, this is important for maaaany reasons, from lag reduction, to layering correctness, to an active face inbetween looking stupid in 2d.

Building Collision
In order for your collision to be a continuous volume, every 2 quads that are next to each other must share 1 side. See this example clip of building a random small piece of collision for example: (I ran out of instant replay window so this start with me already having selected preset 1, sorry :p)
https://www.youtube.com/watch?v=gLF6Dj6cu3g
In order for the NC to disable the inside faces they need to be shared between 2 quads, so if you have a "rectangle face" that you're adding your next quad to, be sure that that quad shares a side with one of the 2 quads that make up the rectangle. To do this, enable "quad debug" in script settings to be able to see where they are.
NEVER overlap collision. It will use expensive checks, won't even layer correctly after those, and will look really stupid.
NEVER leave gaps. even the tiniest of slivers can be slipped through by turning parallel to them. Always join 2 nodeclusters where they are supposed to touch.
If and when you're building collision and suddenly it looks super weird and something is wrong I recommend just deleting all quads arounf the area and starting again, you most likely added a quad at an inappropriate place.

### NC optimisation

first the settings:

IsConvex: this is on by default because 99% of the time you should build your collision in such a way that this can be on, though it is completely possible to unknowingly violate this rule. What this setting does is that it disables layering between the quads of the cluster, which is incredibly important because all checks between quads in a node cluster would usually be slow short checks (explained below). But what this means is that if you build a nodecluster in a way where, if looked at the right way, 2 visible faces might overlap, those faces have a 50/50 chance of being the wrong way arund. If you are in a situation where you need to have this kind of collision, you should have 2 NCs instead and join them with ctrl+j.

simplerLayering: as the tooltip says, this is a just-in-case setting if an optimisation doesn't hold up for some reason. Consult me if you think you might need to turn this off.

LineComp: in short checks (explained below) there is a flaw which might cause incorrect layering. You should do everything in your power to solve this problem in any other way if it pops up such as moving the things far enough apart to not have short checks anymore, but if you must, this setting will enable extra checks to make sure the problem is gone. This is, however, THE LAGGIEST THING EVER, so please try to not use it, and even if you must, minimise the amount of line checks generated.

Different levels of checks
when layering happens, the main components of that are comparisons between 2 objects (quads or flats). These comparisons are divided into 4 cases and are quicker the further the 2 are apart (proportionally to their size). The 4 cases are: long, medium, short and line checks. here I'll illustrate their approximate speeds with some imaginary time units that I've pulled out of my ass, but hopefully they get the point across.

long checks: the 2 things are so far they could never touch. basically free, 0.1 time units
medium checks: the 2 things could touch, but at this rotation they don't. more expensive but still fast, 0.5 time units.
short checks: the 2 are touching, needs actual math to figure out, 10 time units. (these can be incorrect, see lineComp)
line checks: you enabled line checks for some reason, will fix the innacuracies of short checks but will also melt your cpu, 50 time units.

When building collision it's important to keep in mind what checks are happening, and to do this you can enable the "layer debug" option in script settings. with this setting when you turn the camera lines are drawn between 2 objects whenever they're compared with anything but long checks: yellow for medium, blue for short and white for line. 

### extending

The easiest things literally anyone can do is add enemies and node presets.

enemies: go to 3dFlats.cpp, scroll down, copy paste one of the classes, change the strings, thickness and possibly vertical offset. if you want your enemy to move be sure to pick something that inherits from d3MovingEnemyBase. If you want custom behaviour however you are more than welcome to dabble in overriding stuff from the base class to make your own beatiful monstrosity

presets: go to 3dNodeCluster.cpp, search for "presets", second match should be over some if statements that are the presets. observe the pattern, copy paste another one, and set the 3d coordinates. note: the keycode for the number key n is 0x3n c:

if you want to make something bigger new feel free to, if you want to utilise the editor tooling I recommend just adding another file, but if you want to start fresh with only the pure maths include 3dCC.cpp and get going, just look at the pre-existing implementations to learn what updates are needed for the objects and when (there are quite many, sorry but work should only be done when necesary :p)



## Custom Collision (2d)

Allows the creation of arbitrary collision.

setup:
* add map/customCollision/2dCollisionEditor.cpp to your map
* set up play area. this is the intended area of the map, no collision will work outside of this. The surface area of this is directly proportional to the amount of load-in lag.
* to set it up, choose playAreaCorner as the topleft corner of playArea, and choose width and height. (10kx10k is fine to start with).
* you can see it's size with the showPlayArea toggle, though for this and showCacheDebug you need to save and reload script for the visualisation to be correct. (sry I'm lazy xD)
* choose a colour for custom spikes and or dust if you want to use them (make sure to set the alpha)
* set dustPos, which is the place where dustblocks spawn for every missed custom dust after you end the level, would recommend out of sight.
* this is better to do later when you can see the impact but I'll just explain what it means here since it's the last thing in this menu:
collisionOrder determines the grid size that is used for optimizing collision. without going into too much detail, lower means less lag during gameplay (caps at like 4 or so) but a bigger lagspike at load (exponential) and vice versa.
you can see the size of the grid with showCacheDebug though if you don't fully understand what it means then good luck :p

building:
* add collsion by assigning a scriptTrigger to quadEntity
* you can set settings in the left menu, and you can move the corners by clicking on them in the world view, no need to use the variables c:
* layer and sub_layer are the drawing layer, they don't affect collision. so if you make something a bg layer object the visuals will be misaligned from collsion.
* concave collision won't work!!!!!!
* add eny entites you want to be able to interract with the collision to the extraGuys array in script settings

extending:
* For extending this I recommend taking a pretty good look at how it's structured to avoid weird behaviour, and make sure to call d2CQuad.UpdateCollision() whenever you change a quad's corners.

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


