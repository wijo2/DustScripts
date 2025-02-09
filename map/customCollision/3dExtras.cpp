//here goes stuff like enemies and banner props once I get around to it
#include "mathHelper.cpp";

class nthggg{}
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

	FakeTrigger(script@ s, entity@ trigger, d2Math::Vector2 pos)
	{
		@script = @s;
		@input = @get_input_api();
		this.pos = pos;
		s.fakeTriggers.insertLast(this);
		@this.trigger = @trigger;
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
