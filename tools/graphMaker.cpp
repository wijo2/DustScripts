//dustScripts/tools/graphMaker.cpp

//something is very wrong with this idk what but can't be fucked to figure it out rn

//the indentation is fucked up because clang doesn't like handles and since this is a throwaway script I don't happen to care c:
class script
{
bool started = false;

[text] float startOffset = 0;
[text] float endOffset = -96;
[text] uint divisions = 1000;

[label:"-1 for left, 1 for right"] int dir = -1;

[text] int runFrames = 100;
int framesHappened = 0;

float startHeight;

array<dustman@> dummies;

array<int> successes;

array<bool> applicable;

script()
{
	puts("graphMaker working c:");
	applicable.resize(divisions);
}

void step(int lol)
{
	if (framesHappened > runFrames) 
	{ 
		string r = "[";
		for (uint i = 0; i < successes.length(); i++)
		{
			if (i != 0)
			{
				r += "," + (startHeight + startOffset + float(successes[i])/divisions*abs(startOffset-endOffset));
			}
			else
			{
				r += "" + (startHeight + startOffset + float(successes[i])/divisions*abs(startOffset-endOffset));
			}
		}
		r += "]";
		puts(r);
		return; 
	}
	scene@ s = get_scene();

	if (!started)
	{
		dustman@ d = controller_controllable(uint(get_active_player())).as_dustman();
		//wall_grab
		if (d.state() == 12)
		{
			float cx = d.x();
			float cy = d.y();
			startHeight = cy;
			started = true;
			for(uint i = 0; i < divisions; i++)
			{
	   dustman@ dummy = create_entity("dust_kid").as_dustman();
				// if (dummy is d) { puts("w h a t"); }
				dummies.push_back(dummy);
				dummy.x(cx);
				dummy.y(cy + startOffset + (endOffset/divisions)*i);
				dummy.ai_disabled(true);
				dummy.auto_respawn(false);
				s.add_entity(dummy.as_entity());
			}
		}
	}

	if (started) 
	{
		framesHappened += 1;
		if (framesHappened == runFrames)
		{
			for(uint i = 0; i < dummies.length(); i++)
			{
				s.remove_entity(dummies[i].as_entity());
			}
			dummies.resize(0);

			puts("successes: " + successes.length());
			framesHappened += 10;
			return;
		}
		for(uint i = 0; i < dummies.length(); i++)
		{
			dustman@ dummy = dummies[i];
			dummy.y_intent(-1);
			dummy.x_intent(dir);
			if (dummy.y_speed() > 0) { applicable[i] = true; }
			//land
			if (abs(dummy.y_speed()) < 0.001 && applicable[i])
			{
				if (successes.find(i) == -1)
				{
					successes.push_back(i);
				}
				s.remove_entity(dummy.as_entity());
			}
		}
	}
}
}
