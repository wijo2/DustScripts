#include "mathHelper.cpp";
#include "3dCC.cpp";

//this file contains basic tools useful for any editor implementation of the 3d custom collision.

class nthggg{}

//usually I don't like inheritance but this seems like a good place to use it
//since all enemies need an unique implementation anyways, this will also work for props
class d3FlatObjectBase : d3::d3FlatDrawable
{
	//from interface:
	//d2Math::Rect drawRect;
	//float depth;

	d3::d3Manager@ manager;
	d3::Renderable@ renderable;
	script@ script;

	[hidden] d3FakeTrigger fakeTrigger;
	[hidden] bool hasInit = false;

	void init(script@ s, entity@ self) 
	{
		@this.manager = @s.manager;
		@script = @s;
		if (!hasInit)
		{
			fakeTrigger = d3FakeTrigger();
			hasInit = true;
		}
		fakeTrigger.Init(self, s, manager);
		UpdateRotation();
		@renderable = @d3::Renderable(1);
		@renderable.flat = @this;
		manager.renderables.insertLast(renderable);
		s.flats.insertLast(this);
	}

	void editor_step()
	{
		fakeTrigger.EditorStep();
	}

	void UpdateRotation()
	{
		fakeTrigger.UpdateRotation();
		depth = fakeTrigger.ssp.z;
	}

	void on_remove()
	{
		int i = manager.renderables.findByRef(renderable);
		if (i != -1)
		{
			manager.renderables.removeAt(i);
		}
		i = script.flats.findByRef(this);
		if (i != -1)
		{
			script.flats.removeAt(i);
		}
		fakeTrigger.DeleteSelf();
	}
}

class d3EnemyBase : d3FlatObjectBase
{
	scripttrigger@ self;
	entity@ entity;
	[hidden] int entityId = -1;
	sprites@ sprites;
	string sprite;
	uint palette = 1;
	float rotation = 0;
	Vector2 scale = Vector2(1,1);
	uint frame = 0;

	void init(script@ s, scripttrigger@ self)
	{
		@this.self = self;
		self.editor_handle_size(0);
		@sprites = entity.get_sprites();
		d3FlatObjectBase::init(s, self.as_entity());
		fakeTrigger.colour = 0xFF0000FF;
		entity.layer(17);
	}

	void editor_step() override
	{
		d3FlatObjectBase::editor_step();
		frame = uint(get_scene().time_in_level()*12/1000) % sprites.get_animation_length(sprite);
		entity.x(fakeTrigger.ssp.x);
		entity.y(fakeTrigger.ssp.y);
	}
	void step()
	{
		frame = uint(get_scene().time_in_level()*12/1000) % sprites.get_animation_length(sprite);
		entity.x(fakeTrigger.ssp.x);
		entity.y(fakeTrigger.ssp.y);
	}

	void UpdateRotation() override
	{
		d3FlatObjectBase::UpdateRotation();
		rectangle@ sr = sprites.get_sprite_rect(sprite, frame);
		Vector2 pos = Vector2(fakeTrigger.ssp.x, fakeTrigger.ssp.y);
		drawRect = d2Math::Rect(pos.x + sr.left(), pos.y + sr.top(), pos.x + sr.right(), pos.y + sr.bottom());
		// script.debugDraw.insertLast(drawRect);
	}

	void Draw(scene@ s) override
	{
		// return;
		uint color = script.ApplyFog(0xFFFFFFFF, depth);
		sprites.draw_world(18, 1, sprite, frame, palette, fakeTrigger.ssp.x, 
					 fakeTrigger.ssp.y, rotation, scale.x, scale.y, color);
	}

	void on_remove() override
	{
		d3FlatObjectBase::on_remove();
		get_scene().remove_entity(entity);
	}
}

class d3FakeTrigger
{
	[hidden] Vector3 pos;
	Vector3 ssp;
	[hidden] uint colour = 0xFF490f70;

	[hidden] Vector2 oldCentre;
	[hidden] bool hasInit = false;

	FakeTrigger@ fakeTrigger;
	d3::d3Manager@ manager;

	void Init(entity@ trigger, script@ s, d3::d3Manager@ manager)
	{
		@fakeTrigger = @FakeTrigger(s, trigger, Vector2(trigger.x(), trigger.y()));
		@this.manager = @manager;
		fakeTrigger.colour = colour;
		if (!hasInit)
		{
			Vector2 camCen = manager.cam.igCoords;
			Vector2 ipos = fakeTrigger.pos - camCen;
			pos = manager.cam.CamToWorldPos(Vector3(ipos.x, ipos.y, 0));
			hasInit = true;
		}
		UpdateRotation();
	}

	void EditorStep()
	{
		fakeTrigger.colour = colour;
		Vector2 curPos = fakeTrigger.pos;
		Vector2 dif = curPos - oldCentre;
		if (dif != Vector2())
		{
			pos += manager.cam.CamToWorldDir(Vector3(dif.x, dif.y,0));
			oldCentre = fakeTrigger.pos;
			Vector2 camCen = manager.cam.igCoords;
			Vector3 camPos = Vector3(camCen.x, camCen.y, 0);
			Vector3 newPos = manager.cam.WorldToCamPos(pos) + camPos;
			ssp = newPos;
		}
	}

	void UpdateRotation()
	{
		Vector2 camCen = manager.cam.igCoords;
		Vector3 camPos = Vector3(camCen.x, camCen.y, 0);
		Vector3 newPos = manager.cam.WorldToCamPos(pos) + camPos;
		fakeTrigger.pos = Vector2(newPos.x, newPos.y);
		oldCentre = Vector2(newPos.x, newPos.y);
		ssp = newPos;
		if (newPos.z < 0) { fakeTrigger.size = 0; }
		else { fakeTrigger.size = 10; }
	}

	void DeleteSelf()
	{
		fakeTrigger.DeleteSelf();
	}
}

class FakeTrigger
{
	Vector2 pos;
	uint colour = 0xFF490f70;
	float size = 10;

	script@ script;
	input_api@ input;
	entity@ trigger;

	uint holdTime = 0;
	uint holdTimeR = 0;
	bool isHeld = false;
	bool isHeldR = false;

	Vector2 oldPos;

	bool centralise = true;

	FakeTrigger(){}
	FakeTrigger(script@ s, entity@ trigger, Vector2 pos)
	{
		@script = @s;
		@input = @get_input_api();
		this.pos = pos;
		s.fakeTriggers.insertLast(this);
		@this.trigger = @trigger;
	}

	FakeTrigger& opAssign(const FakeTrigger &inout o)
	{
		pos = o.pos;
		colour = o.colour;
		size = o.size;
		@script = @o.script;
		@input = @o.input;
		@trigger = @o.trigger;
		holdTime = o.holdTime;
		holdTimeR = o.holdTimeR;
		isHeld = o.isHeld;
		isHeldR = o.isHeldR;
		oldPos = o.oldPos;
		return this;
	}

	void DeleteSelf()
	{
		int i = script.fakeTriggers.findByRef(this);
		if (i != -1)
		{
			script.fakeTriggers.removeAt(i);
		}
	}

	void Draw(scene@ s)
	{
		s.draw_rectangle_world(21, 1, pos.x-size, pos.y-size, pos.x+size, pos.y+size, 0, colour);
	}

	void EditorStep()
	{
		if (script.editor.editor_tab() == "Select" 
	  && @script.editor.get_selected_trigger() == null)
		{
			if (!input.key_check_gvb(3) && isHeldR)
			{
				if (holdTime < 20)
				{
					if (@trigger != null)
					{
						get_scene().remove_entity(trigger.as_entity());
					}
				}
				holdTimeR = 0;
				isHeldR = false;
			}
			Vector2 mpos = Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
			if (input.key_check_pressed_gvb(3)
				&& mpos.x > pos.x-size && mpos.x < pos.x+size
				&& mpos.y > pos.y-size && mpos.y < pos.y+size)
			{
				isHeldR = true;
			}
			if (input.key_check_gvb(3) && isHeldR)
			{
				holdTimeR++;
			}
			else
			{
				holdTimeR = 0;
			}
			if (!input.key_check_gvb(2) && isHeld)
			{
				if (holdTime < 20)
				{
					if (@trigger != null)
					{
						script.editor.editor_tab("Triggers");
						script.editor.set_selected_trigger(trigger.as_entity());
					}
				}
				holdTime = 0;
				isHeld = false;
				oldPos = Vector2();
			}
			if (input.key_check_pressed_gvb(2)
				&& mpos.x > pos.x-size && mpos.x < pos.x+size
				&& mpos.y > pos.y-size && mpos.y < pos.y+size)
			{
				isHeld = true;
			}
			if (input.key_check_gvb(2) && isHeld)
			{
				holdTime++;
				if (oldPos == Vector2())
				{
					oldPos = mpos;
					return;
				}
				Vector2 dif = mpos - oldPos;
				pos += dif;
				oldPos = mpos;
			}
			else
			{
				holdTime = 0;
				oldPos = Vector2();
			}
		}
		else
		{
			holdTime = 0;
			oldPos = Vector2();
		}
	}
}

