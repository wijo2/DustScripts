#include "mathHelper.cpp";

class nthggg{}
namespace d3e
{

	class d3Enemy
	{
		d2Math::Rect drawRect;
		float depth;
		void Draw(scene@ s){}
	}

	class d3PropBanner
	{
		d2Math::Rect drawRect;
		float depth;
		void Draw(scene@ s){}
	}
}

//not in d3e cause of the whole namespaces don't persist thing.
class d3FakeTrigger
{
	Vector3 pos;
	uint colour = 0xFF490f70;
	FakeTrigger fakeTrigger;

	d3::d3Manager@ manager;

	d2Math::Vector2 oldCentre;
	bool hasInit = false;

	void Init(entity@ trigger, script@ s, d3::d3Manager manager)
	{
		@this.manager = @manager;
		fakeTrigger = FakeTrigger(s, trigger, d2Math::Vector2(trigger.x(), trigger.y()));
		fakeTrigger.colour = colour;
		if (!hasInit)
		{
			puts("fake trigger init!");
			puts(pos);
			d2Math::Vector2 camCen = manager.cam.igCoords;
			d2Math::Vector2 ipos = fakeTrigger.pos - camCen;
			pos = manager.cam.CamToWorldPos(Vector3(ipos.x, ipos.y, 0));
			hasInit = true;
			puts("hasInit = true");
		}
		UpdateRotation();
	}

	void EditorStep()
	{
		d2Math::Vector2 curPos = fakeTrigger.pos;
		d2Math::Vector2 dif = curPos - oldCentre;
		if (dif != d2Math::Vector2())
		{
			pos += manager.cam.CamToWorldDir(Vector3(dif.x, dif.y,0));
			oldCentre = fakeTrigger.pos;
		}
	}

	void UpdateRotation()
	{
		d2Math::Vector2 camCen = manager.cam.igCoords;
		Vector3 camPos = Vector3(camCen.x, camCen.y, 0);
		Vector3 newPos = manager.cam.WorldToCamPos(pos) + camPos;
		fakeTrigger.pos = d2Math::Vector2(newPos.x, newPos.y);
		oldCentre = d2Math::Vector2(newPos.x, newPos.y);
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
	d2Math::Vector2 pos;
	uint colour = 0xFF490f70;
	float size = 10;

	script@ script;
	input_api@ input;
	entity@ trigger;

	uint holdTime = 0;
	uint holdTimeR = 0;
	bool isHeld = false;
	bool isHeldR = false;

	d2Math::Vector2 oldPos;

	FakeTrigger(){}
	FakeTrigger(script@ s, entity@ trigger, d2Math::Vector2 pos)
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
						get_scene().remove_entity(trigger);
					}
				}
				holdTimeR = 0;
				isHeldR = false;
			}
			d2Math::Vector2 mpos = d2Math::Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
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
						script.editor.set_selected_trigger(trigger);
					}
				}
				holdTime = 0;
				isHeld = false;
				oldPos = d2Math::Vector2();
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
				if (oldPos == d2Math::Vector2())
				{
					oldPos = mpos;
					return;
				}
				d2Math::Vector2 dif = mpos - oldPos;
				pos += dif;
				oldPos = mpos;
			}
			else
			{
				holdTime = 0;
				oldPos = d2Math::Vector2();
			}
		}
		else
		{
			holdTime = 0;
			oldPos = d2Math::Vector2();
		}
	}
}

