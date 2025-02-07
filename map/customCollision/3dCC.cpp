#include "2dCC.cpp";
#include "3dExtras.cpp";
#include "mathHelper.cpp";

namespace d3
{
class nothing{} //this tricks my lsp to obey

class d3Cam
{
	Vector3 centre;
	float rotation; //0 = normal xy, z away from cam, upwards = angle in radians topdown clockwise
	//coords of centre of cam in in-game coordinate system
	d2Math::Vector2 igCoords;

	d3Cam()
	{
		centre = Vector3(0,0,0);
		d2Math::Vector2 igCoords;
		rotation = 0;
	}

	Vector3 WorldToCamPos(Vector3 vec)
	{
		vec -= centre;
		float c = cos(rotation);
		float s = sin(rotation);
		return Vector3(
			vec.x * c - vec.z * s,
			vec.y,
			vec.x * s + vec.z * c
		);
	}

	Vector3 CamToWorldPos(Vector3 vec)
	{
		float c = cos(-rotation);
		float s = sin(-rotation);
		vec = Vector3(
			vec.x * c - vec.z * s,
			vec.y,
			vec.x * s + vec.z * c
		);
		return vec + centre;
	}

	Vector3 CamToWorldDir(Vector3 vec)
	{
		float c = cos(-rotation);
		float s = sin(-rotation);
		vec = Vector3(
			vec.x * c - vec.z * s,
			vec.y,
			vec.x * s + vec.z * c
		);
		return vec;
	}

	Vector3 WorldToCamDir(Vector3 vec)
	{
		float c = cos(rotation);
		float s = sin(rotation);
		vec = Vector3(
			vec.x * c - vec.z * s,
			vec.y,
			vec.x * s + vec.z * c
		);
		return vec;
	}
}

class nothing2{} //this tricks my lsp to obey
class d3Quad
{
	Vector3 p1;
	Vector3 p2;
	Vector3 p3;
	Vector3 p4;
	uint colour;
	bool shaded = true;

	//cached answer for shading
	float fac1;
	float fac2;
	float fac3;
	float fac4;

	//cam space coords
	Vector3 csp1;
	Vector3 csp2;
	Vector3 csp3;
	Vector3 csp4;

	//""projected"" positions
	d2Math::Vector2 pp1;
	d2Math::Vector2 pp2;
	d2Math::Vector2 pp3;
	d2Math::Vector2 pp4;

	array<bool> drawnSides(4);

	//behind cam, don't draw
	bool behind;
	//intersecting with cam plane
	bool intersecting;
	//false for 1-3, true for 2-2
	bool intersectionType;

	//for every point, which side of cam is it on?
	//1 = front
	array<bool> sides(4);

	d3Quad()
	{
		p1 = Vector3();
		p2 = Vector3();
		p3 = Vector3();
		p4 = Vector3();
	}

	d3Quad(Vector3 p1, Vector3 p2, Vector3 p3, Vector3 p4, uint colour)
	{
		this.p1 = p1;
		this.p2 = p2;
		this.p3 = p3;
		this.p4 = p4;
		this.colour = colour;
	}

	void Draw(scene@ s, uint layer, uint sub_layer, script@ script)
	{
		if (colour == 0x00000000 || behind) { return; }
		if (!shaded)
		{
			if (drawnSides[0])
			{
				s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp3.x, pp3.y, pp3.x, pp3.y, colour, colour, colour, colour);
			}
			if (drawnSides[1])
			{
				s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
			}
			if (drawnSides[2])
			{
				s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
			}
			if (drawnSides[3])
			{
				s.draw_quad_world(layer, sub_layer, false, pp2.x, pp2.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
			}
		}
		else
		{
			uint col = 0;
			if (fac1 > 0 && drawnSides[0])
			{
				col = GetFactoredColour(fac1);
				s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp3.x, pp3.y, pp3.x, pp3.y, 
					  col, col, col, col);
			}
			if (fac2 > 0 && drawnSides[1])
			{
				col = GetFactoredColour(fac2);
				s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp4.x, pp4.y, pp4.x, pp4.y,
					  col, col, col, col);
			}
			if (fac3 > 0 && drawnSides[2])
			{
				col = GetFactoredColour(fac3);
				s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y,
					  col, col, col, col);
			}
			if (fac4 > 0 && drawnSides[3])
			{
				col = GetFactoredColour(fac4);
				s.draw_quad_world(layer, sub_layer, false, pp2.x, pp2.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y,
					  col, col, col, col);
			}
		}
	}

	uint GetFactoredColour(float factor)
	{
		factor *= 1.2;
		if (factor > 1) { factor = 1; }
		if (factor < 0.2) { factor = 0.2; }
		return (uint(float(colour & 0x000000FF)*factor) & 0x000000FF) +
	   (uint(float(colour & 0x00FF0000)*factor) & 0x00FF0000) +
	   (uint(float(colour & 0x0000FF00)*factor) & 0x0000FF00) +
	   (colour & 0xFF000000);
	}

	void ApplyProjection(d3Cam@ cam)
	{
		Vector3 centre = Vector3(cam.igCoords.x, cam.igCoords.y, 0);
		csp1 = cam.WorldToCamPos(p1) + centre;
		csp2 = cam.WorldToCamPos(p2) + centre;
		csp3 = cam.WorldToCamPos(p3) + centre;
		csp4 = cam.WorldToCamPos(p4) + centre;

		pp1 = d2Math::Vector2(csp1.x, csp1.y);
		pp2 = d2Math::Vector2(csp2.x, csp2.y);
		pp3 = d2Math::Vector2(csp3.x, csp3.y);
		pp4 = d2Math::Vector2(csp4.x, csp4.y);

		int behindc = 0;
		if (csp1.z <= 0) { behindc += 1; sides[0] = false; } else { sides[0] = true; }
		if (csp2.z <= 0) { behindc += 1; sides[1] = false; } else { sides[1] = true; }
		if (csp3.z <= 0) { behindc += 1; sides[2] = false; } else { sides[2] = true; }
		if (csp4.z <= 0) { behindc += 1; sides[3] = false; } else { sides[3] = true; }
		// puts("behindc " + behindc);
		behind = (behindc == 4);
		intersecting = !behind && behindc > 0;
		// puts("intersecting " + intersecting);
		intersectionType = (behindc == 2);

		fac1 = GetSideFacing(1);
		fac2 = GetSideFacing(2);
		fac3 = GetSideFacing(3);
		fac4 = GetSideFacing(4);
	}

	//cam coords
	Vector3 CPointByNumber(int n)
	{
		switch (n)
		{
			case 1: return csp1;
			case 2: return csp2;
			case 3: return csp3;
			case 4: return csp4;
		}
		return Vector3();
	}

	//sides:
	//1: 1, 2, 3
	//2: 1, 2, 4
	//3: 1, 3, 4
	//4: 2, 3, 4
	bool DoesSideIntersect(int s)
	{
		return (s == 1 && (sides[0] != sides[1] || sides[1] != sides[2])) ||
		(s == 2 && (sides[0] != sides[1] || sides[1] != sides[3])) ||
		(s == 3 && (sides[0] != sides[2] || sides[2] != sides[3])) ||
		(s == 4 && (sides[1] != sides[2] || sides[2] != sides[3]));
	}

	bool GetSide(int point)
	{
		switch (point)
		{
			case 1: return csp1.z > 0;
			case 2: return csp2.z > 0;
			case 3: return csp3.z > 0;
			case 4: return csp4.z > 0;
		}
		return false;
	}

	//I realise now this is the same functionality as the method literally 2 methods up
	//but oh well can't be bothered to fix :p
	bool IsIntersected(int side)
	{
		// if (side == 1) { puts("1 is intersected " + 
		// 				!(GetSide(1) == GetSide(2) && GetSide(2) == GetSide(3))
		// ); }
		//if all points of side are on same side of cam it's not intersected
		switch (side)
		{
			case 1: return !(GetSide(1) == GetSide(2) && GetSide(2) == GetSide(3));
			case 2: return !(GetSide(1) == GetSide(2) && GetSide(2) == GetSide(4));
			case 3: return !(GetSide(1) == GetSide(3) && GetSide(3) == GetSide(4));
			case 4: return !(GetSide(2) == GetSide(3) && GetSide(3) == GetSide(4));
		}
		return false;
	}

	//gets where line between points 0 and 1 in pair crosses cam plane
	//input is pair for convenience
	d2Math::Vector2 GetIntersection(array<int> pair)
	{
		Vector3 first = CPointByNumber(pair[1]); 
		Vector3 second = CPointByNumber(pair[0]);
		Vector3 dir = second - first;
		if (first.z == second.z) { return d2Math::Vector2(first.x + second.x, first.y + second.y)/2; }
		dir = dir/abs(first.z - second.z) * abs(first.z);
		return d2Math::Vector2(dir.x + first.x, dir.y + first.y);
	}

	Vector3 Find3dCentre()
	{
		return (p1 + p2 + p3 + p4)/4;
	}

	//gets facing of side's normal in relarion to camera from 1 to -1, positive = towards cam 
	float GetSideFacing(int side)
	{
		return -GetNormalVector(side).z;
	}

	Vector3 GetNormalVector(int side)
	{
		array<Vector3> points(3);
		switch (side)
		{
			case 1:
				points[0] = csp1;
				points[1] = csp2;
				points[2] = csp3;
			break;
			case 2:
				points[0] = csp1;
				points[1] = csp2;
				points[2] = csp4;
			break;
			case 3:
				points[0] = csp1;
				points[1] = csp3;
				points[2] = csp4;
			break;
			case 4:
				points[0] = csp2;
				points[1] = csp3;
				points[2] = csp4;
			break;
		}
		Vector3 dir = (points[0]+points[1]+points[2])/3 - (csp1+csp2+csp3+csp4)/4;
		Vector3 norm = (points[0] - points[1]).Cross(points[2] - points[1]);
		return (norm*norm.Dot(dir)).Normalised();
	}

	//1 = point is under (more z), -1 = point is over (less z), 0 = point not inside
	int PointRelation(Vector3 pos)
	{
		if (d2Math::PointInTriangle(
				d2Math::Vector2(pos.x, pos.y),
				d2Math::Vector2(csp1.x, csp1.y),
				d2Math::Vector2(csp2.x, csp2.y),
				d2Math::Vector2(csp3.x, csp3.y)))
		{
			// puts("upstream nonzero");
			Vector3 norm = GetNormalVector(1);
			if (norm.z * (norm.Dot(pos - (csp1+csp2+csp3)/3)) > 0)
			{
				return 1;
			}
			return -1;
		}
		if (d2Math::PointInTriangle(
				d2Math::Vector2(pos.x, pos.y),
				d2Math::Vector2(csp1.x, csp1.y),
				d2Math::Vector2(csp2.x, csp2.y),
				d2Math::Vector2(csp4.x, csp4.y)))
		{
			// puts("upstream nonzero");
			Vector3 norm = GetNormalVector(2);
			if (norm.z * (norm.Dot(pos - (csp1+csp2+csp4)/3)) > 0)
			{
				return 1;
			}
			return -1;
		}
			if (d2Math::PointInTriangle(
				d2Math::Vector2(pos.x, pos.y),
				d2Math::Vector2(csp1.x, csp1.y),
				d2Math::Vector2(csp3.x, csp3.y),
				d2Math::Vector2(csp4.x, csp4.y)))

		{
			// puts("upstream nonzero");
			Vector3 norm = GetNormalVector(3);
			if (norm.z * (norm.Dot(pos - (csp1+csp3+csp4)/3)) > 0)
			{
				return 1;
			}
			return -1;

		}
		if (d2Math::PointInTriangle(
				d2Math::Vector2(pos.x, pos.y),
				d2Math::Vector2(csp2.x, csp2.y),
				d2Math::Vector2(csp3.x, csp3.y),
				d2Math::Vector2(csp4.x, csp4.y)))

		{
			// puts("upstream nonzero");
			Vector3 norm = GetNormalVector(4);
			if (norm.z * (norm.Dot(pos - (csp2+csp3+csp4)/3)) > 0)
			{
				return 1;
			}
			return -1;
		}
		// puts("upstream 0");
		return 0;
	}
}

class nothing3{} //this tricks my lsp to obey

class d3CQuad
{
	d3Quad@ base;
	d2::d2CQuad@ collisionBase;

	d3Manager@ manager;

	//I would rather not have this here but what u gonna do, I need to centralise drawing 
	//if I want proper layering :/
	int layer;
	int sub_layer;

	//sides:
	//1: 1, 2, 3
	//2: 1, 2, 4
	//3: 1, 3, 4
	//4: 2, 3, 4
	array<bool> activeSides(4);

	array<bool> spikeSides(4);
	array<bool> dustSides(4);

	//given 2 sides, what are the 2 points that they share (points 1 indexed)
	//for x,x pairs it's empty array
	array<array<array<int>>> sideLookup = {
		{{}, {1,2}, {1,3}, {2,3}},
		{{1,2}, {}, {1,4}, {2,4}},
		{{1,3}, {1,4}, {}, {3,4}},
		{{2,3}, {2,4}, {3,4}, {}}
	};

	d3CQuad()
	{
		@base = @d3Quad();
		@collisionBase = @d2::d2CQuad();
	}

	//doesn't update collision
	void UpdateIntersectQuad(d3Cam@ cam)
	{
		//it's a mess yeah tell me something I don't already know,
		//to my credit it's actually quite a tough problem
		if (!base.intersecting) 
		{
			collisionBase.activeLines[0] = false;
			collisionBase.activeLines[1] = false;
			collisionBase.activeLines[2] = false;
			collisionBase.activeLines[3] = false;
			collisionBase.deactive = true;
			return;
		}
		collisionBase.deactive = false;

		//sides:
		//1: 1, 2, 3
		//2: 1, 2, 4
		//3: 1, 3, 4
		//4: 2, 3, 4

		// puts("trying to do intersecting");
		int side1 = 1;
		if (!base.intersectionType && !base.IsIntersected(1)) { side1 = 2; }
		// puts("starting with " + side1);
		//3-1 = triangle
		if (!base.intersectionType)
		{
			int side2 = GetNextSide(side1, 0);
			int side3 = GetNextSide(side2, side1);
			// puts("trig sides " + side1 + " " + side2 + " " + side3);
			// puts("spike sides " + spikeSides[0] + " " + spikeSides[1] + " " + spikeSides[2] + " " + spikeSides[3]);
			collisionBase.base.p1 = base.GetIntersection(sideLookup[side1-1][side2-1]);
			collisionBase.base.p2 = base.GetIntersection(sideLookup[side2-1][side3-1]);
			collisionBase.base.p3 = base.GetIntersection(sideLookup[side3-1][side1-1]);
			//need to offset for nodefrompoint to work correctly
			collisionBase.base.p4 = collisionBase.base.p3 + d2Math::Vector2(0.1,0.1);
			collisionBase.activeLines[0] = activeSides[side2-1];
			collisionBase.spikeLines[0] = spikeSides[side2-1];
			collisionBase.dustLines[0] = dustSides[side2-1];
			collisionBase.activeLines[1] = activeSides[side3-1];
			collisionBase.spikeLines[1] = spikeSides[side3-1];
			collisionBase.dustLines[1] = dustSides[side3-1];
			collisionBase.activeLines[2] = false;
			collisionBase.spikeLines[2] = false;
			collisionBase.dustLines[2] = false;
			collisionBase.activeLines[3] = activeSides[side1-1];
			collisionBase.spikeLines[3] = spikeSides[side1-1];
			collisionBase.dustLines[3] = dustSides[side1-1];
		}
		//2-2 = quad
		else
		{
			int side2 = GetNextSide(side1, 0);
			int side3 = GetNextSide(side2, side1);
			int side4 = GetNextSide(side3, side2);
			// puts("quad sides " + side1 + " " + side2 + " " + side3 + " " + side4);
			// puts("spike sides " + spikeSides[0] + " " + spikeSides[1] + " " + spikeSides[2] + " " + spikeSides[3]);
			// puts("trying 1 is intersected " + base.IsIntersected(1));
			collisionBase.base.p1 = base.GetIntersection(sideLookup[side1-1][side2-1]);
			collisionBase.base.p2 = base.GetIntersection(sideLookup[side2-1][side3-1]);
			collisionBase.base.p3 = base.GetIntersection(sideLookup[side3-1][side4-1]);
			collisionBase.base.p4 = base.GetIntersection(sideLookup[side4-1][side1-1]);
			// puts("quad points " + collisionBase.base.p1 + " " + collisionBase.base.p2 + " " + collisionBase.base.p3 + " " + collisionBase.base.p4);
			collisionBase.activeLines[0] = activeSides[side2-1];
			collisionBase.spikeLines[0] = spikeSides[side2-1];
			collisionBase.dustLines[0] = dustSides[side2-1];
			collisionBase.activeLines[1] = activeSides[side3-1];
			collisionBase.spikeLines[1] = spikeSides[side3-1];
			collisionBase.dustLines[1] = dustSides[side3-1];
			collisionBase.activeLines[2] = activeSides[side4-1];
			collisionBase.spikeLines[2] = spikeSides[side4-1];
			collisionBase.dustLines[2] = dustSides[side4-1];
			collisionBase.activeLines[3] = activeSides[side1-1];
			collisionBase.spikeLines[3] = spikeSides[side1-1];
			collisionBase.dustLines[3] = dustSides[side1-1];
		}
	}

	void DrawBase(scene@ s)
	{
		//I could make a loop and stuff but like this is literally easier so whatever
		base.Draw(s, layer, sub_layer, manager.script);
		uint sc = manager.script.spikeColour;
		uint dc = manager.script.dustColour;
		if (spikeSides[0] && base.fac1 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp1.x, base.pp1.y, base.pp2.x, base.pp2.y, base.pp3.x, base.pp3.y, base.pp3.x, base.pp3.y,
					 sc,sc,sc,sc);
		}
		if (spikeSides[1] && base.fac2 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp1.x, base.pp1.y, base.pp2.x, base.pp2.y, base.pp4.x, base.pp4.y, base.pp4.x, base.pp4.y,
					 sc,sc,sc,sc);
		}
		if (spikeSides[2] && base.fac3 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp1.x, base.pp1.y, base.pp3.x, base.pp3.y, base.pp4.x, base.pp4.y, base.pp4.x, base.pp4.y,
					 sc,sc,sc,sc);
		}
		if (spikeSides[3] && base.fac4 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp2.x, base.pp2.y, base.pp3.x, base.pp3.y, base.pp4.x, base.pp4.y, base.pp4.x, base.pp4.y,
					 sc,sc,sc,sc);
		}
		if (dustSides[0] && base.fac1 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp1.x, base.pp1.y, base.pp2.x, base.pp2.y, base.pp3.x, base.pp3.y, base.pp3.x, base.pp3.y,
					 dc,dc,dc,dc);
		}
		if (dustSides[1] && base.fac2 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp1.x, base.pp1.y, base.pp2.x, base.pp2.y, base.pp4.x, base.pp4.y, base.pp4.x, base.pp4.y,
					 dc,dc,dc,dc);
		}
		if (dustSides[2] && base.fac3 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp1.x, base.pp1.y, base.pp3.x, base.pp3.y, base.pp4.x, base.pp4.y, base.pp4.x, base.pp4.y,
					 dc,dc,dc,dc);
		}
		if (dustSides[3] && base.fac4 > 0)
		{
			s.draw_quad_world(layer, sub_layer, false, 
					 base.pp2.x, base.pp2.y, base.pp3.x, base.pp3.y, base.pp4.x, base.pp4.y, base.pp4.x, base.pp4.y,
					 dc,dc,dc,dc);
		}
	}

	void DrawIntersect(scene@ s)
	{
		collisionBase.Draw(s, layer, sub_layer);
	}

	//next side for loop around intersection that is not side2
	int GetNextSide(int side, int side2)
	{
		auto shared1 = sideLookup[side-1][0];
		auto shared2 = sideLookup[side-1][1];
		auto shared3 = sideLookup[side-1][2];
		auto shared4 = sideLookup[side-1][3];
		if (side != 1 && side2 != 1 &&
			((base.intersectionType && base.GetSide(shared1[0]) != base.GetSide(shared1[1])) || 
			(!base.intersectionType && base.IsIntersected(1)))) { return 1; }
		if (side != 2 && side2 != 2 &&
			((base.intersectionType && base.GetSide(shared2[0]) != base.GetSide(shared2[1])) || 
			(!base.intersectionType && base.IsIntersected(2)))) { return 2; }
		if (side != 3 && side2 != 3 &&
			((base.intersectionType && base.GetSide(shared3[0]) != base.GetSide(shared3[1])) || 
			(!base.intersectionType && base.IsIntersected(3)))) { return 3; }
		if (side != 4 && side2 != 4 &&
			((base.intersectionType && base.GetSide(shared4[0]) != base.GetSide(shared4[1])) || 
			(!base.intersectionType && base.IsIntersected(4)))) { return 4; }
		puts("GetNextSide returning 0 with inputs " + side + ", " + side2 + " and intersection type " + base.intersectionType);
		return 0;
	}

	//is any point of this quad is under o return 1, if any of this is over other quad return -1, otherwise 0
	int AnyPointUnder(d3CQuad@ o)
	{
		int i;
		i = o.base.PointRelation(base.csp1);
		if (i != 0) { return i; }
		i = o.base.PointRelation(base.csp2);
		if (i != 0) { return i; }
		i = o.base.PointRelation(base.csp3);
		if (i != 0) { return i; }
		i = o.base.PointRelation(base.csp4);
		return i;
	}

	int opCmp(d3CQuad@ o)
	{
		int r = AnyPointUnder(o);
		if (r != 0) { return -r; }
		return o.AnyPointUnder(this);
	}
}	

class d3Manager
{
	array<d3CQuad@> allQuads;
	d2::CollisionManager@ manager;
	d3Cam@ cam;

	script@ script;
	
	d3Manager()
	{
		@manager = @d2::CollisionManager();
		@cam = @d3Cam();
	}

	void UpdateLooks()
	{
		for (uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].base.ApplyProjection(cam);
			allQuads[i].UpdateIntersectQuad(cam);
		}
		SortQuadList();
	}

	void UpdateCollision()
	{
		for (uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].collisionBase.UpdateCollision(false);
		}

	}

	void SortQuadList()
	{
		allQuads.sortAsc();
	}

	void Draw()
	{
		for (uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].DrawBase(get_scene());
		}
		for (uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].DrawIntersect(get_scene());
		}
	}

	void RemoveQuad(d3CQuad@ q)
	{
		for (uint i = 0; i < allQuads.length(); i++)
		{
			if (allQuads[i] is q)
			{
				allQuads.removeAt(i);
			}
		}
	}
}

}
