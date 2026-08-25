//dustScripts/tools/testplug.cpp

//we save the data as string because we don't have templates
//so like this is the closest we can get, you do need to make your
//own (en/)decoder tho
class DataSaver : trigger_base
{
	script@ script;
	[hidden] private string data = "";

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		puts("ds init!");
		self.radius(0);
		script.recive_ds(this);
	}

	string get_data() { return data; }
	void set_data(string d) 
	{ 
		data = d; 
		save_data();
	}

	//call during gameplay when you want to save the data
	//(non-saved will get lost when editor anywhere)
	bool save_data()
	{
		controllable@ c = controller_controllable(uint(get_active_player()));
		if (@c != null) 
		{ 
			c.metadata().set_string("dataSaver", data);
			return true;
		}
		return false;
	}

	bool on_editor_load()
	{
		controllable@ c = controller_controllable(uint(get_active_player()));
		if (@c == null) { return false; }
		data = c.metadata().get_string("dataSaver");
		return data != "";
	}
}

class script
{
	editor_api@ eapi;
	input_api@ iapi;

	//starts at -2 and ++ each editor frame, create at 0 and set to 1 
	//this to make sure we don't create one when it exists already
	//(why does trigger init happen after editor step n.1???)
	int dsCreated = -2;
	DataSaver@ ds;

	bool printedData = false;

	script()
	{
		puts("--testplug--");
		@eapi = get_editor_api();
		@iapi = get_input_api();
		if (eapi is null)
		{
			puts("level");
		}
		else
		{
			puts("editor");
		}
	}

	void recive_ds(DataSaver@ newDs)
	{
		if (dsCreated != 1)
		{
			puts("recived (new) ds!");
			dsCreated = 1;
			@ds =  @newDs;
		}
		else
		{
			puts("recived (old) ds!");
			string data = ds.get_data();
			@ds =  @newDs;
			ds.set_data(data);
		}
	}

	//this will create the ds instance which will
	//recall the data from gameplay if it exists, if the trigger
	//has been created already we will make sure to replace the data
	//with the new data so that we don't roll back 
	//the data to the data saved to the level
	//if not this will be used in editor_step to create the trigger
	void on_editor_start()
	{
		puts("editor start, trying to retrive data");
		@ds = @DataSaver();
		bool isData = ds.on_editor_load();
		if (isData)
		{
			dsCreated = 1;
			puts("we have data!");
			puts(ds.get_data());
		}
		else { puts("no data :c"); }
	}

	void editor_step()
	{
		if (dsCreated < 0) { dsCreated++; }
		else if (dsCreated == 0) 
		{
			puts("creating ds!");
			dsCreated = 1;
			scripttrigger@ newTrigger = create_scripttrigger(ds);
			get_scene().add_entity(newTrigger.as_entity());
			ds.set_data("data from editor");
		}
		if (dsCreated == 1 && !printedData)
		{
			printedData = true;
			puts("editor step data dump");
			puts(ds.get_data());
		}
	}

	void step(int idk)
	{
		if (!printedData && dsCreated == 1)
		{
			printedData = true;
			puts("step data dump");
			puts(ds.get_data());
			ds.set_data("data from gameplay");
		}
		
	}
}
