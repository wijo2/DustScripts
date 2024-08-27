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
	puts(colRect.bottom() + ", " + colRect.top());
	switch (side)
	{
		case 0:
		case 1:
		CIR	= d2Math::IntRect(int(colRect.left() + ec.x()), int(colRect.top()*5/8 + ec.y()), int(colRect.right() + ec.x()), int(colRect.top()*3/8 + ec.y()));	
		break;

		case 2:
		case 3:
		CIR = d2Math::IntRect(int(colRect.left()/2 + ec.x()), int(colRect.top() + ec.y()), int(colRect.right()/2 + ec.x()), int(ec.y()));
		break;
	}
	// CIR.Draw(get_scene(), 22, 1);
	array<d2::d2CQuad@>@ colliders = manager.GetCollidersInArea(CIR);
	for (uint i = 0; i < colliders.length(); i++)
	{
		array<d2Math::LineFunc> edges = colliders[i].GetSideEdges(side);
		for (uint li = 0; li < edges.length(); li++)
		{
			if (!CIR.CheckLineIntersection(edges[li])) { continue; }

			d2Math::Vector2 hitPos = CIR.LineIntersectionPosition(edges[li]);
			if (!edges[li].IsWithinBounds(hitPos)) { continue; }

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
				puts("angle " + angle);
				break;
				case 2:
				angle = int(57.29578 * atan2(-edges[li].k, -1));
				break;
				case 3:
				angle = int(57.29578 * atan2(edges[li].k, 1));
				break;
	   		}
			if (!IsSuitableAngle(side, angle)) { continue; }
			puts("suitable!");

			// puts("hit! " + side);
			tc.hit(true);
			tc.type(angle);
			switch (side)
			{
				case 0:
				tc.hit_x(edges[li].GetRevValue((CIR.y1 + CIR.y2) / 2));
				tc.hit_y((CIR.y1 + CIR.y2) / 2);
				break;
				case 1:
				tc.hit_x(edges[li].GetRevValue((CIR.y1 + CIR.y2) / 2));
				tc.hit_y((CIR.y1 + CIR.y2) / 2);
				break;
				case 2:
				tc.hit_x((CIR.x1 + CIR.x2) / 2);
				tc.hit_y(edges[li].GetValue((CIR.x1 + CIR.x2) / 2));
				break;
				case 3:
				tc.hit_x((CIR.x1 + CIR.x2) / 2);
				tc.hit_y(edges[li].GetValue((CIR.x1 + CIR.x2) / 2));
				break;
			}
			return;
	   		}
		}
		ec.check_collision(tc, side, moving, snap_offset);
	}

	bool IsSuitableAngle(int side, int angle)
	{
		puts("side " + side + " angle " + angle);
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
}
