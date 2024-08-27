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
		d2Math::IntRect CIR = d2Math::IntRect(int(colRect.left()), int(colRect.top()), int(colRect.right()), int(colRect.bottom()));
		CIR.Draw(get_scene(), 22,1);
		array<d2::d2CQuad@>@ colliders = manager.GetCollidersInArea(CIR);
		for (uint i = 0; i < colliders.length(); i++)
		{
			puts("collision is managing!!");
	   		array<d2Math::LineFunc> edges = colliders[i].GetSideEdges(side);
	   		for (uint li = 0; li < edges.length(); li++)
	   		{
				if (!CIR.CheckLineIntersection(edges[li])) { continue; }
				tc.hit(true);
				tc.type(int(57.29578 * atan2(edges[li].k, 1)));
				tc.hit_x(ec.x());
				tc.hit_y(ec.y());
				return;
	   		}
		}
		// ec.check_collision(tc, side, moving, snap_offset);
	}
}
