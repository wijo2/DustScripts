#include "2dCC.cpp";

class CollisionOverride : callback_base
{

	d2::CollisionManager@ manager;

	CollisionOverride(d2::CollisionManager@ manager)
	{
		@this.manager = @manager;
	}

	void CollisionCallback(controllable@ ec, tilecollision@ tc, int side, bool moving, float snap_offset, int arg)
	{
		rectangle@ colRect = ec.collision_rect();
		d2Math::IntRect CIR;
		//this is mimicing the ray based collision
		if (@ec.as_dustman() != null)
		{
			switch (side)
			{
				case 0:
					CIR	= d2Math::IntRect(
						int(colRect.left() + ec.x() - snap_offset), 
						int(colRect.top()*5/8 + ec.y()), 
						int(ec.x() + colRect.right()/4), 
						int(colRect.top()*3/8 + ec.y())
					);	
				break;
				case 1:
					CIR	= d2Math::IntRect(
						int(ec.x() + colRect.left()/4), 
						int(colRect.top()*5/8 + ec.y()), 
						int(colRect.right() + ec.x() + snap_offset), 
						int(colRect.top()*3/8 + ec.y())
					);	
				break;

				case 2:
					CIR = d2Math::IntRect(
						int(colRect.left()/2 + ec.x()), 
						int(colRect.top() + ec.y() - snap_offset), 
						int(colRect.right()/2 + ec.x()), 
						int(ec.y() + colRect.top()*3/8)
					);
				break;
				case 3:
					CIR = d2Math::IntRect(
						int(colRect.left()/2 + ec.x()), 
						int(colRect.top()*5/8 + ec.y()), 
						int(colRect.right()/2 + ec.x()), 
						int(ec.y() + snap_offset)
					);
				break;
			}
		}
		else
		{
			CIR = d2Math::IntRect(
				int(colRect.left() + ec.x()),
				int(colRect.top() + ec.y()),
				int(colRect.right() + ec.x()),
				int(colRect.bottom() + ec.y())
			);
		}

		// CIR.Draw(get_scene(), 22, 1);
		array<d2::d2CQuad@>@ colliders = manager.GetCollidersInArea(CIR);
		for (uint i = 0; i < colliders.length(); i++)
		{
			array<d2Math::LineFunc> edges = colliders[i].GetSideEdges(side);
			for (uint li = 0; li < edges.length(); li++)
			{
				if (!CIR.CheckLineIntersection(edges[li])) { continue; }
				// puts("suitable side! side: " + side);

				d2Math::Vector2 hitPos;
				if (!CIR.BoundedIntersectionPosition(edges[li], hitPos)) { continue; }
				// d2Math::Rect(hitPos, 20, 20).Draw(get_scene(), 22, 1);

				//fuck angles they never work like you want them to >:c
				int angle;
				switch (side)
				{
					case 0:
						angle = int(57.29578 * atan2(edges[li].k, 1));
						if (angle < 0) { angle += 180; }
					break;
					case 1:
						angle = int(57.29578 * atan2(edges[li].k, 1));
						if (angle > 0) { angle -= 180; }
					break;
					case 2:
						angle = int(57.29578 * atan2(-edges[li].k, -1));
					break;
					case 3:
						angle = int(57.29578 * atan2(edges[li].k, 1));
					break;
				}
				if (!IsSuitableAngle(side, angle)) { continue; }
				// puts("suitable angle! side: " + side);

				//angle fix bc df is literally racist wtf
				switch (side) 
				{
					case 0:
						if (angle > 112) { angle = 112; }
						if (angle < 67) { angle = 67; }
					break;
					case 1:
						if (angle < -112) { angle = -112; }
						if (angle > -67) { angle = -67; }
					break;
					case 2:
						if (angle > 0 && angle < 135) { angle = 135; }
						if (angle < 0 && angle > -135) { angle = -135; }
					break;
					case 3:
						if (angle > 45) { angle = 45; }
						if (angle < -45) { angle = -45; }
					break;
				}

				// puts("hit! " + side);
				tc.hit(true);
				tc.type(angle);
				int offset = 4;
				switch (side)
				{
					case 0:
						tc.hit_x(edges[li].GetRevValue((CIR.y1 + CIR.y2) / 2) - offset);
						tc.hit_y((CIR.y1 + CIR.y2) / 2);
					break;
					case 1:
						tc.hit_x(edges[li].GetRevValue((CIR.y1 + CIR.y2) / 2) + offset);
						tc.hit_y((CIR.y1 + CIR.y2) / 2);
					break;
					case 2:
						tc.hit_x((CIR.x1 + CIR.x2) / 2);
						tc.hit_y(edges[li].GetValue((CIR.x1 + CIR.x2) / 2) - offset);
					break;
					case 3:
						tc.hit_x((CIR.x1 + CIR.x2) / 2);
						tc.hit_y(edges[li].GetValue((CIR.x1 + CIR.x2) / 2) + offset);
					break;
				}
				if (@ec.as_dustman() != null)
				{
					int n1 = colliders[i].base.GetNodeFromPoint(edges[li].bound1);
					int n2 = colliders[i].base.GetNodeFromPoint(edges[li].bound2);
					int s = colliders[i].base.GetSideFromNodes(n1, n2);
					if (s != -1)
					{
						colliders[i].SideTouched(s);
					}
				}
				return;
			}
		}
		//if nothing was found use the default
		ec.check_collision(tc, side, moving, snap_offset);
	}

	bool IsSuitableAngle(int side, int angle)
	{
		// puts("side " + side + " angle " + angle);
		switch (side)
		{
			case 0:
				return 123.75 > angle && angle > 56.25;
			case 1:
				return -123.75 < angle && angle < -56.25;
			case 2:
				return angle > 123.75 || -123.75 > angle;
			case 3:
				return -56.25 < angle && angle < 56.25;
		}
		return false;
	}

	//this is for attacks clearing dust
	//I've given up on non-though-wall clearning I don't wanna raycast so ye
	void step()
	{
		controllable@ player = controller_controllable(uint(get_active_player()));
		if (@player == null) { return; }
		hitbox@ attack = player.hitbox();
		if (@attack == null || attack.state_timer() != attack.activate_time()) { return; }

		rectangle@ colRect = attack.base_rectangle();

		d2Math::IntRect CIR;
		CIR = d2Math::IntRect(
			int(colRect.left() + attack.x()),
			int(colRect.top() + attack.y()),
			int(colRect.right() + attack.x()),
			int(colRect.bottom() + attack.y())
		);

		// CIR.Draw(get_scene(), 22, 1);
		array<d2::d2CQuad@>@ colliders = manager.GetCollidersInArea(CIR);
		for (uint i = 0; i < colliders.length(); i++)
		{
			array<d2Math::LineFunc> edges = { 
				colliders[i].base.GetLineFunc(1),
				colliders[i].base.GetLineFunc(2),
				colliders[i].base.GetLineFunc(3),
				colliders[i].base.GetLineFunc(4)
			};
			for (uint li = 0; li < edges.length(); li++)
			{
				if (!CIR.CheckLineIntersection(edges[li])) { continue; }

				d2Math::Vector2 hitPos;
				if (!CIR.BoundedIntersectionPosition(edges[li], hitPos)) { continue; }

				//hit a collider!
				colliders[i].SideAttacked(li+1, player.as_dustman());
			}
		}
	}
}
