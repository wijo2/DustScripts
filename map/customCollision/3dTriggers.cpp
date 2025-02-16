//for non-drawn 3d entities

class d3StartPos : trigger_base
{
	scripttrigger@ self;
	script@ script;
	d3::d3Manager@ manager;

	[hidden] d3FakeTrigger fakeTrigger;
	[hidden] bool hasInit = false;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@manager = @s.manager;
		@this.self = @self;

		if (@s.startPos != null && !(s.startPos is this)&& !self.destroyed())
		{
			get_scene().remove_entity(s.startPos.self.as_entity());
		}
		if (!self.destroyed())
		{
			@s.startPos = @this;
		}
		if (!hasInit)
		{
			fakeTrigger = d3FakeTrigger();
			fakeTrigger.colour = 0xFF00FF00;
			hasInit = true;
		}
		fakeTrigger.Init(self.as_entity(), s, manager);
		self.editor_handle_size(0);
		UpdateRotation();
		script.startCoords = fakeTrigger.pos;
		s.firstFrame = true;
	}

	void editor_step()
	{
		fakeTrigger.EditorStep();
		script.startCoords = fakeTrigger.pos;
	}

	void UpdateRotation()
	{
		fakeTrigger.UpdateRotation();
	}

	void on_remove()
	{
		fakeTrigger.DeleteSelf();
	}
}

class d3EndFlag : trigger_base
{
	scripttrigger@ self;
	script@ script;
	d3::d3Manager@ manager;
	input_api@ input;

	[hidden] d3FakeTrigger fakeTrigger;
	[hidden] bool hasInit = false;

	//ids of scripttriggers
	[hidden] array<uint> attachedEntities;
	array<entity@> entityHandles;

	bool ended = false;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@manager = @s.manager;
		@this.self = @self;
		@input = @get_input_api();

		if (!hasInit)
		{
			fakeTrigger = d3FakeTrigger();
			fakeTrigger.colour = 0xFFFF0000;
			hasInit = true;
		}
		fakeTrigger.Init(self.as_entity(), s, manager);
		self.editor_handle_size(0);
		UpdateRotation();
		script.flags.insertLast(this);

		//loop backwards for safe deletion
		for(int i = attachedEntities.length()-1; i >= 0;  i--)
		{
			// puts("id: " + entityId);
			entity@ ent = @entity_by_id(attachedEntities[uint(i)]);
			if (@ent == null || @ent.as_scripttrigger() == null)
			{
				//this is going to happen on checkpoints if I ever add them
				//but since stuff doesn't persist in play mode anyways it's fine
				attachedEntities.removeAt(i);
				continue;
			}
			entityHandles.insertLast(ent);
		}
	}

	void editor_step()
	{
		fakeTrigger.EditorStep();

		if(script.editor.editor_tab() == "Triggers" 
			&& @script.editor.get_selected_trigger() != null
			&& script.editor.get_selected_trigger().is_same(self.as_entity())
			&& input.key_check_pressed_gvb(2) && input.key_check_gvb(10))
		{
			Vector2 mpos = Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
			for(uint i = 0; i < script.flats.length(); i++)
			{
				d3EnemyBase@ flat = cast<d3EnemyBase>(script.flats[i]);
				if (flat is null || flat.depth < -flat.thickness) { continue; }
				if (!flat.drawRect.PointInside(mpos)) { continue; }
				uint id = flat.self.id();
				int ind = attachedEntities.find(id);
				if (ind != -1)
				{
					attachedEntities.removeAt(ind);
					ind = entityHandles.findByRef(flat.self.as_entity());
					if (ind != -1) { entityHandles.removeAt(ind); }
				}
				else
				{
					entityHandles.insertLast(flat.self.as_entity());
					attachedEntities.insertLast(id);
				}
				break;
			}
		}
	}

	void step()
	{
		if (ended) { return; }
		bool end = entityHandles.length() != 0;
		for(uint i = 0; i < entityHandles.length(); i++)
		{
			if (@entityHandles[i] != null 
				&& !(cast<d3EnemyBase>(entityHandles[i].as_scripttrigger().get_object()).entity is null) 
			&& !(cast<d3EnemyBase>(entityHandles[i].as_scripttrigger().get_object()).entity.destroyed()))
			{
				end = false;
				break;
			}
		}
		if (end)
		{
			get_scene().end_level(fakeTrigger.ssp.x, fakeTrigger.ssp.y);
			ended = true;
		}
	}

	void editor_draw(float ok)
	{
		scene@ s = get_scene();
		if(script.editor.editor_tab() == "Triggers" 
			&& @script.editor.get_selected_trigger() != null
			&& script.editor.get_selected_trigger().is_same(self.as_entity()))
		{
			for(uint i = 0; i < script.flats.length(); i++)
			{
				d3EnemyBase@ flat = cast<d3EnemyBase>(script.flats[i]);
				if (flat is null || flat.depth < -flat.thickness) { continue; }
				if (attachedEntities.find(flat.self.id()) == -1) { continue; }
				s.draw_line_world(21, 1, fakeTrigger.ssp.x, fakeTrigger.ssp.y, 
						 flat.fakeTrigger.ssp.x, flat.fakeTrigger.ssp.y, 3, 0xFFFF0000);
			}	
		}
	}

	void UpdateRotation()
	{
		fakeTrigger.UpdateRotation();
	}

	void on_remove()
	{
		fakeTrigger.DeleteSelf();
		int i = script.flags.find(this);
		if (i != -1)
		{
			script.flags.removeAt(i);
		}
	}
}

class d3TextTrigger : trigger_base
{
	scripttrigger@ self;
	script@ script;
	d3::d3Manager@ manager;

	[hidden] d3FakeTrigger fakeTrigger;
	[hidden] bool hasInit = false;
	//no you don't get a visualisation fuck you deal with it
	[text|tooltip:"3d rangle of trigger"] float range = 1000;

	[hidden] int ttId = -1;
	entity@ tt;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@manager = @s.manager;
		@this.self = @self;

		if (!hasInit)
		{
			fakeTrigger = d3FakeTrigger();
			fakeTrigger.colour = 0xFFFFFF00;
			hasInit = true;
		}
		fakeTrigger.Init(self.as_entity(), s, manager);
		self.editor_handle_size(0);
		UpdateRotation();
		script.textTriggers.insertLast(this);

		if (ttId == -1)
		{
			@tt = create_entity("text_trigger");
			get_scene().add_entity(tt);
			ttId = tt.id();
		}
		else
		{
			@tt = @entity_by_id(uint(ttId));
			if (@tt == null)
			{
				@tt = create_entity("text_trigger");
				get_scene().add_entity(tt);
				ttId = tt.id();
			}
		}
	}

	void editor_step()
	{
		fakeTrigger.EditorStep();
		if (@tt == null) { return; }

		Vector2 camCen = manager.cam.igCoords;
		if(script.editor.editor_tab() == "Triggers" 
			&& @script.editor.get_selected_trigger() != null
	 		&& (script.editor.get_selected_trigger().is_same(self.as_entity())
				|| script.editor.get_selected_trigger().is_same(tt.as_entity())))
		{
			if ((manager.cam.centre - fakeTrigger.pos).Magnitude() < range)
			{
				tt.x(camCen.x);
				tt.y(camCen.y);
			}
			else
			{
				tt.x(camCen.x);
				tt.y(camCen.y + 5000);
			}
		}
		else
		{
			tt.x(camCen.x);
			tt.y(camCen.y + 5000);
		}
	}

	void step()
	{
		Vector2 camCen = manager.cam.igCoords;
		if ((manager.cam.centre - fakeTrigger.pos).Magnitude() < range)
		{
			tt.x(camCen.x);
			tt.y(camCen.y);
		}
		else
		{
			tt.x(camCen.x);
			tt.y(camCen.y + 5000);
		}
	}

	void UpdateRotation()
	{
		fakeTrigger.UpdateRotation();
	}

	void on_remove()
	{
		fakeTrigger.DeleteSelf();
		get_scene().remove_entity(tt);
		int i = script.textTriggers.findByRef(this);
		if (i != -1)
		{
			script.textTriggers.removeAt(i);
		}
	}
}
