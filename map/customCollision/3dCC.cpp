#include "2dCC.cpp";
#include "mathHelper.cpp";

namespace d3
{
class nothing{} //this tricks my lsp to obey

class d3Cam
{
	d3Math::Vector3 centre;
	float rotation; //0 = normal xy, z away from cam, upwards = angle in radians topdown clockwise

	d3Cam()
	{
		centre = d3Math::Vector3(0,0,0);
		rotation = 0;
	}

	d3Math::Vector3 WorldToCamPos(d3Math::Vector3 vec)
	{
		vec -= centre;
		float c = cos(rotation);
		float s = sin(rotation);
		return d3Math::Vector3(
			vec.x * c - vec.z * s,
			vec.y,
			vec.x * s + vec.z * c
		);
	}
}

class nothing2{} //this tricks my lsp to obey
class d3Quad
{
	d3Math::Vector3 p1;
	d3Math::Vector3 p2;
	d3Math::Vector3 p3;
	d3Math::Vector3 p4;
	uint colour;

	//cam space coords
	d3Math::Vector3 csp1;
	d3Math::Vector3 csp2;
	d3Math::Vector3 csp3;
	d3Math::Vector3 csp4;

	//""projected"" positions
	d2Math::Vector2 pp1;
	d2Math::Vector2 pp2;
	d2Math::Vector2 pp3;
	d2Math::Vector2 pp4;

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
		p1 = d3Math::Vector3();
		p2 = d3Math::Vector3();
		p3 = d3Math::Vector3();
		p4 = d3Math::Vector3();
	}

	d3Quad(d3Math::Vector3 p1, d3Math::Vector3 p2, d3Math::Vector3 p3, d3Math::Vector3 p4, uint colour)
	{
		this.p1 = p1;
		this.p2 = p2;
		this.p3 = p3;
		this.p4 = p4;
		this.colour = colour;
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		if (colour == 0x00000000 || behind) { return; }
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp3.x, pp3.y, pp3.x, pp3.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp2.x, pp2.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp1.x, pp1.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
		s.draw_quad_world(layer, sub_layer, false, pp2.x, pp2.y, pp3.x, pp3.y, pp4.x, pp4.y, pp4.x, pp4.y, colour, colour, colour, colour);
	}

	void ApplyProjection(d3Cam@ cam)
	{
		csp1 = cam.WorldToCamPos(p1);
		csp2 = cam.WorldToCamPos(p2);
		csp3 = cam.WorldToCamPos(p3);
		csp4 = cam.WorldToCamPos(p4);

		pp1 = d2Math::Vector2(csp1.x, csp1.y);
		pp2 = d2Math::Vector2(csp2.x, csp2.y);
		pp3 = d2Math::Vector2(csp3.x, csp3.y);
		pp4 = d2Math::Vector2(csp4.x, csp4.y);

		int behindc = 0;
		if (csp1.z <= 0) { behindc += 1; sides[0] = false; } else { sides[0] = true; }
		if (csp2.z <= 0) { behindc += 1; sides[1] = false; } else { sides[1] = true; }
		if (csp3.z <= 0) { behindc += 1; sides[2] = false; } else { sides[2] = true; }
		if (csp4.z <= 0) { behindc += 1; sides[3] = false; } else { sides[3] = true; }
		behind = behindc == 4;
		intersecting = !behind && behindc > 0;
		intersectionType = behindc == 2;
	}

	//cam coords
	d3Math::Vector3 CPointByNumber(int n)
	{
		switch (n)
		{
			case 1: return csp1;
			case 2: return csp2;
			case 3: return csp3;
			case 4: return csp4;
		}
		return d3Math::Vector3();
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

	bool IsIntersected(int side)
	{
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
		d3Math::Vector3 first = CPointByNumber(pair[1]); 
		d3Math::Vector3 second = CPointByNumber(pair[0]);
		d3Math::Vector3 dir = first - second;
		dir = dir/abs(first.z - second.z) * first.z;
		return d2Math::Vector2(dir.x + first.x, dir.y + first.y);
	}
}

class nothing3{} //this tricks my lsp to obey

class d3CQuad
{
	d3Quad@ base;
	d2::d2CQuad@ collisionBase;

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
		if (!base.intersecting) { return; }
		int side1 = 1;
		if (!base.intersectionType && !base.IsIntersected(1)) { side1 = 2; }
		//3-1 = triangle
		if (!base.intersectionType)
		{
			int side2 = GetNextSide(side1, 0);
			int side3 = GetNextSide(side2, side1);
			// puts("trig sides " + side1 + " " + side2 + " " + side3);
			collisionBase.base.p1 = base.GetIntersection(sideLookup[side1-1][side3-1]);
			collisionBase.base.p2 = base.GetIntersection(sideLookup[side1-1][side2-1]);
			collisionBase.base.p3 = base.GetIntersection(sideLookup[side2-1][side3-1]);
			collisionBase.base.p4 = collisionBase.base.p3;
			collisionBase.activeLines[0] = activeSides[side1-1];
			collisionBase.spikeLines[0] = spikeSides[side1-1];
			collisionBase.dustLines[0] = dustSides[side1-1];
			collisionBase.activeLines[1] = activeSides[side2-1];
			collisionBase.spikeLines[1] = spikeSides[side2-1];
			collisionBase.dustLines[1] = dustSides[side2-1];
			collisionBase.activeLines[2] = activeSides[side3-1];
			collisionBase.spikeLines[2] = spikeSides[side3-1];
			collisionBase.dustLines[2] = dustSides[side3-1];
			collisionBase.activeLines[3] = false;
			collisionBase.spikeLines[3] = false;
			collisionBase.dustLines[3] = false;
		}
		//2-2 = quad
		else
		{
			int side2 = GetNextSide(side1, 0);
			int side3 = GetNextSide(side2, side1);
			int side4 = GetNextSide(side3, side2);
			// puts("trig sides " + side1 + " " + side2 + " " + side3 + " " + side4);
			collisionBase.base.p1 = base.GetIntersection(sideLookup[side1][side3]);
			collisionBase.base.p2 = base.GetIntersection(sideLookup[side1][side2]);
			collisionBase.base.p3 = base.GetIntersection(sideLookup[side2][side3]);
			collisionBase.base.p4 = base.GetIntersection(sideLookup[side3][side4]);
			collisionBase.activeLines[0] = activeSides[side1-1];
			collisionBase.spikeLines[0] = spikeSides[side1-1];
			collisionBase.dustLines[0] = dustSides[side1-1];
			collisionBase.activeLines[1] = activeSides[side2-1];
			collisionBase.spikeLines[1] = spikeSides[side2-1];
			collisionBase.dustLines[1] = dustSides[side2-1];
			collisionBase.activeLines[2] = activeSides[side3-1];
			collisionBase.spikeLines[2] = spikeSides[side3-1];
			collisionBase.dustLines[2] = dustSides[side3-1];
			collisionBase.activeLines[3] = activeSides[side4-1];
			collisionBase.spikeLines[3] = spikeSides[side4-1];
			collisionBase.dustLines[3] = dustSides[side4-1];
		}
	}

	void Draw(scene@ s, uint layer, uint sub_layer)
	{
		base.Draw(s, layer, sub_layer);
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
		puts("GetNextSide returning 0 with inputs " + side + ", " + side2);
		return 0;
	}
}	

class d3Manager
{
	array<d3CQuad@> allQuads;
	d2::CollisionManager@ manager;
	d3Cam@ cam;
	
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
	}
}

}
