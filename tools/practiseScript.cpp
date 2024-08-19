//#include "lib/std.cpp";

//text transform stuff, would've put into a static class if I had one :c

string DeathArrToString(array<Death> arr)
{
	string result = "";
	for (uint i = 0; i < arr.length(); i++)
	{
		result += arr[i].ToString();
	}
	return result;
}

Death ParseDeathString(string ds)
{
	int firstSeperator = ds.findFirst("|");
	int secondSeperator = ds.substr(firstSeperator + 1, -1).findFirst("|") + firstSeperator + 1;
	int thirdSeperator = ds.findLast("|");

	string xs = ds.substr(1, firstSeperator - 1);
	string ys = ds.substr(firstSeperator + 1, secondSeperator - firstSeperator - 1);
	string startCps = ds.substr(secondSeperator + 1, thirdSeperator - secondSeperator - 1);	
	string deathCps = ds.substr(thirdSeperator + 1, ds.length() - thirdSeperator - 2);

	float x = parseFloat(xs);
	float y = parseFloat(ys);
	int startCp = parseInt(startCps);
	int deathCp = parseInt(deathCps);
	return Death(x, y, startCp, deathCp);
}

array<Death> ParseArrayString(string ips)
{
	array<Death> newArray;
	
	uint lastBlockStart = 0;
	for (uint i = 0; i < ips.length(); i++)
	{
		if (ips.substr(i, 1) == "}")
		{
	  		string ss = ips.substr(lastBlockStart, i - lastBlockStart + 1);
			newArray.insertLast(ParseDeathString(ss));
			lastBlockStart = i + 1;
		}
	}
	return newArray;
}


class script
{

[label:"checkpoints"] array<Checkpoint> cps;
[slider,min:0,max:0.9999,step:0.01|label:"opacity of deaths"] float deathTransparency;
[colour|label:"colour of deaths"] uint32 deathColour;
[label:"only keep x deaths"] uint deathsToKeep;

[option|label:""] string empty1;
[option|label:"charts"] string title2; 
[colour,alpha|label:"colour of charts"] uint32 chartColour;
[colour,alpha|label:"colour of stripes"] uint32 chartAltColour;
[colour,alpha|label:"colour of chart text"] uint32 chartTextColour;

[option|label:""] string empty2;
Chart successRate;
[option|label:"success rate at cp"] string title6; 
[label:"visible?"] bool successRateVisible;
[label:"position"|position,mode:world,layer:19,y:successRateY] float successRateX;
[hidden] float successRateY;
[slider,min:1,max:2000,step=0.1|label:"width"] float successRateWidth;
[slider,min:1,max:2000,step=0.1|label:"height"] float successRateHeight;

[option|label:""] string empty3;
Chart highestReached;
[option|label:"highest cp reached"] string title8; 
[label:"visible?"] bool highestReachedVisible;
[label:"position"|position,mode:world,layer:19,y:highestReachedY] float highestReachedX;
[hidden] float highestReachedY;
[slider,min:1,max:2000,step=0.1|label:"width"] float highestReachedWidth;
[slider,min:1,max:2000,step=0.1|label:"height"] float highestReachedHeight;

[option|label:""] string empty4;
Chart tsfc;
[option|label:"times started from cp"] string title3; 
[label:"visible?"] bool tsfcVisible;
[label:"position"|position,mode:world,layer:19,y:tsfcY] float tsfcX;
[hidden] float tsfcY;
[slider,min:1,max:2000,step=0.1|label:"width"] float tsfcWidth;
[slider,min:1,max:2000,step=0.1|label:"height"] float tsfcHeight;

[option|label:""] string empty5;
Chart timesVisited;
[option|label:"times cp has been visited"] string title5; 
[label:"visible?"] bool timesVisitedVisible;
[label:"position"|position,mode:world,layer:19,y:timesVisitedY] float timesVisitedX;
[hidden] float timesVisitedY;
[slider,min:1,max:2000,step=0.1|label:"width"] float timesVisitedWidth;
[slider,min:1,max:2000,step=0.1|label:"height"] float timesVisitedHeight;

[option|label:""] string empty6;
Chart timesDied;
[option|label:"times died to cp"] string title7; 
[label:"visible?"] bool timesDiedVisible;
[label:"position"|position,mode:world,layer:19,y:timesDiedY] float timesDiedX;
[hidden] float timesDiedY;
[slider,min:1,max:2000,step=0.1|label:"width"] float timesDiedWidth;
[slider,min:1,max:2000,step=0.1|label:"height"] float timesDiedHeight;

[hidden] array<Death> deathsSaved;
string currentString;

int currentCp = 0;
bool updateCp = false;
int startCp = 0;

bool deathRecorded = true;

dustman@ activePlayer;
sprites@ deathSprite;

script()
{
	puts("practiseScript working c:");
}

void Init()
{
	@deathSprite = @create_sprites();
	deathSprite.add_sprite_set("apple");

	if (is_playing())
	{
		RefreshEntities();
		controllable@ c = controller_controllable(uint(get_active_player()));
		if (@c != null)
		{
			@activePlayer = @c.as_dustman();
		}
	}
}

void SetupChartData()
{
	uint len = cps.length() + 1;
	array<float> tsfcArr(len);
	array<float> timesVisitedArr(len);
	array<float> successRateArr(len);
	array<float> highestReachedArr(len);
	array<float> deathsArr(len);

	array<string> names(len);
	
	for (uint i = 0; i < len; i++)
	{
		names[i] = i + "";
	}

	for (uint i = 0; i < deathsSaved.length(); i++)
	{
		Death@ d = deathsSaved[i];

		tsfcArr[d.startCp] += 1;

		if (uint(d.deathCp) < len) 
		{
			deathsArr[d.deathCp] += 1;
		}

		for (uint a = 0; a < len; a++)
		{
			if (d.startCp <= int(a) && d.deathCp >= int(a))
			{
				timesVisitedArr[a] += 1;
			}
		}

		for (uint a = d.startCp; a < len; a++)
		{
			if (highestReachedArr[a] < d.deathCp)
			{
				highestReachedArr[a] = d.deathCp;
			}
		}
	}
	
	for (uint i = 0; i < len; i++)
	{
		if (timesVisitedArr[i] == 0) 
		{
			successRateArr[i] = 0;
			continue;
		}
		successRateArr[i] = 1 - deathsArr[i] / timesVisitedArr[i];
	}

	tsfc = Chart(tsfcArr, names, false, 0, "times started from cp");
	timesVisited = Chart(timesVisitedArr, names, false, 0, "times visited cp");
	timesDied = Chart(deathsArr, names, false, 0, "times died to cp");
	successRate = Chart(successRateArr, names, true, 0, "cp success rate");
	highestReached = Chart(highestReachedArr, names, true, 0, "highest reached from cp");
}

void LoadFromMetadata()
{
	controllable@ c = controller_controllable(uint(get_active_player()));
	if (@c == null) { return; }
	string savedString = c.metadata().get_string("deathArray");
	array<Death> recoveredArray = ParseArrayString(savedString);
	if (recoveredArray.length() > 0)
	{
		deathsSaved = recoveredArray;
	}
}

void SaveMetadata()
{
	activePlayer.metadata().set_string("deathArray", currentString);
}

void checkpoint_load() 
{ 
	Init();
	SaveMetadata();
	startCp = currentCp;
}
void on_level_start() 
{ 
	Init(); 
	currentString = DeathArrToString(deathsSaved);
};

void on_editor_start() 
{ 
	LoadFromMetadata();
	if (deathsToKeep == 0) { deathsToKeep = 1000; }
	if (deathsSaved.length() > deathsToKeep)
	{
		deathsSaved.reverse();
		deathsSaved.resize(deathsToKeep);
		deathsSaved.reverse();
	}
	SetupChartData();
	Init(); 
}

void editor_var_changed(var_info@ info) { RefreshEntities(); }

void RefreshEntities()
{
	for (uint i = 0; i < cps.length(); i++)
	{
		cps[i].Init(this);
	}
}

void checkpoint_save() { updateCp = true; }

void step(int uselessGarbage)
{
	if (is_playing())
	{
		DoCpUpdate();
		CheckDead();
	}
}

void DoCpUpdate()
{
	if (updateCp)
	{
		updateCp = false;
		for (uint i = 0; i < cps.length(); i++)
		{
			if (cps[i].IsActiveCheckpoint())
			{
				currentCp = i + 1;
				if (startCp == 0)
				{
					startCp = i;
				}
				return;
			}
		}
		currentCp = 0;
	}
}

void CheckDead()
{
	if (@activePlayer == null) { return; }
	if (!activePlayer.dead() && deathRecorded)
	{
		deathRecorded = false;
	}
	if (activePlayer.dead() && !deathRecorded)
	{
		deathRecorded = true;
		Death d = Death(activePlayer.x(), activePlayer.y(), startCp, currentCp);
		currentString += d.ToString();
		SaveMetadata();
	}
}

void on_level_end()
{
	Death d = Death(activePlayer.x(), activePlayer.y(), startCp, cps.length() + 1);
	currentString += d.ToString();
	SaveMetadata();
}

void editor_draw(float dontcare)
{
	if (deathTransparency != 0)
	{
		for (uint i = 0; i < deathsSaved.length(); i++)
		{
			if (uint(deathsSaved[i].deathCp) < cps.length() + 1)
			{
				deathSprite.draw_world(21, 1, "idle", 0, 1, deathsSaved[i].x, deathsSaved[i].y, 0, 0.4, 0.4, deathColour - (uint(float(0xFF) * (1-deathTransparency)) << 24));
			}
		}
	}
	
	if (tsfcVisible)
	{
		tsfc.Draw(this, tsfcX, tsfcY, tsfcWidth, tsfcHeight, false, 0);
	}
	if (timesVisitedVisible)
	{
		timesVisited.Draw(this, timesVisitedX, timesVisitedY, timesVisitedWidth, timesVisitedHeight, false, 0);
	}
	if (timesDiedVisible)
	{
		timesDied.Draw(this, timesDiedX, timesDiedY, timesDiedWidth, timesDiedHeight, false, 0);
	}
	if (successRateVisible)
	{
		successRate.Draw(this, successRateX, successRateY, successRateWidth, successRateHeight, true, 0);
	}
	if (highestReachedVisible)
	{
		highestReached.Draw(this, highestReachedX, highestReachedY, highestReachedWidth, highestReachedHeight, false, 0);
	}
}


};


class Checkpoint
{

	script@ script;

	[label:"entity"|entity,flags,players] uint id;

	entity@ entity;
	float x;
	float y;

	void Init(script@ s)
	{
		@script = @s;
		@entity = @entity_by_id(id);
		x = entity.x();
		y = entity.y();
	}

	bool IsThisCheckpoint(float x2, float y2)
	{
		return x2 == entity.x() && y2 == entity.y();
	}

	bool IsActiveCheckpoint()
	{
		if (@script.activePlayer == null) { return false; }
		dustman@ d = @script.activePlayer;
		if (@d == null) { return false; }
		int pid = d.id();
		scene@ s = get_scene();
		return IsThisCheckpoint(s.get_checkpoint_x(pid), s.get_checkpoint_y(pid));
	}

};


class Death
{
	[text] float x;
	[text] float y;
	[text] int startCp;
	[text] int deathCp; 

	Death()
	{
		x = 0; y = 0; startCp = 0; deathCp = 0;
	}

	Death(float x, float y, int startCp, int deathCp)
	{
		this.x = x; this.y = y; this.startCp = startCp; this.deathCp = deathCp;
	}

	string ToString()
	{
		return "{" + x + "|" + y + "|" + startCp + "|" + deathCp +"}";
	}
};



class Chart
{
	array<float> data;
	array<string> names;
	float maxData = 0;
	float maxNameWidth = 0;
	float maxDataWidth = 0;
	float fontHeight = 0;
	string title;
	
	Chart() {}

	Chart(array<float> data, array<string> names, bool showAsPercent, int valueDecimals, string title = "")
	{
		if (data.length() != names.length()) { puts("data and name length contradiction!!!"); }
		this.data = data; this.names = names; this.title = title;
		for (uint i = 0; i < data.length(); i++)
		{
			if (data[i] > maxData)
			{
				maxData = data[i];
			}
		}
		textfield@ tf = create_textfield();
		tf.text("0");
		tf.set_font("Caracteres", 140);
		fontHeight = tf.text_height();
		for (uint i = 0; i < data.length(); i++)
		{
			tf.text(names[i]);
			if (tf.text_width() > maxNameWidth)
			{
				maxNameWidth = tf.text_width();
			}
			if (showAsPercent)
			{
				tf.text(formatFloat(data[i]*100, "", 0, valueDecimals) + "p");
			}
			else
			{
				tf.text(formatFloat(data[i], "", 0, valueDecimals));
			}
			if (tf.text_width() > maxDataWidth)
			{
				maxDataWidth = tf.text_width();
			}
		}
		if (maxDataWidth < maxNameWidth) maxDataWidth = maxNameWidth;
	}

	void Draw(script@ script, float x, float y, float width, float height, bool showAsPercent, int valueDecimals)
	{
		scene@ s = get_scene();	

		float w = width;
		if (data.length() != 0)
		{
			w = width / data.length();
		}

		float heightMulti = 0;
		if (maxData != 0) 
		{
			heightMulti = height / maxData;
		}
		float nameTextScaling = w / maxNameWidth * 0.8;
		float valueTextScaling = w / maxDataWidth * 0.8;
		float yc = y + height;	   

		textfield@ tf = create_textfield();
		tf.colour(script.chartTextColour);
		tf.set_font("Caracteres", 140);
		for (uint i = 0; i < data.length(); i++)
		{
			float xc = x + w*i;	   
			if (i % 2 == 0)
			{
				s.draw_rectangle_world(21, 1, xc, yc, xc + w, yc - data[i] * heightMulti, 0, script.chartColour);
			}
			else
			{
				s.draw_rectangle_world(21, 1, xc, yc, xc + w, yc - data[i] * heightMulti, 0, script.chartAltColour);
			}
			tf.text(names[i]);
			tf.draw_world(21, 1, xc + w/2, yc + w * 0.15 + fontHeight/2*nameTextScaling, nameTextScaling, nameTextScaling, 0);

			if (showAsPercent)
			{
				tf.text(formatFloat(data[i]*100, "", 0, valueDecimals) + "p");
			}
			else
			{
				tf.text(formatFloat(data[i], "", 0, valueDecimals));
			}
			tf.draw_world(21, 1, xc + w/2, yc - w * 0.15 - fontHeight/2*valueTextScaling - data[i] * heightMulti, valueTextScaling, valueTextScaling, 0);
		}
		s.draw_line_world(21, 1, x, y + height, x + width, y + height, height * 0.01, 0xFF000000);
		tf.text(title);
		tf.draw_world(21, 1, x + width/2, y - fontHeight*valueTextScaling - fontHeight * nameTextScaling * 1.5 / 2 - w/2, nameTextScaling * 1.5, nameTextScaling * 1.5, 0);
	}
}
