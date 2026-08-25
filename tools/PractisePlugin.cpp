//dustScripts/tools/practisePlugin.cpp

class DataSaver : trigger_base
{
	script@ script;
	[hidden] private string data = "";

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		self.radius(0);
		script.recive_ds(this);
	}

	string get_data() { return data; }
	void set_data(string d) 
	{ 
		data = d; 
		save_data();
	}

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

	void on_editor_start()
	{
		puts("editor start, trying to retrive data");
		@ds = @DataSaver();
		bool isData = ds.on_editor_load();
		if (isData)
		{
			dsCreated = 1;
			puts("we have data!");
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
		}
	}

	void step(int idk)
	{
	}
}
