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
