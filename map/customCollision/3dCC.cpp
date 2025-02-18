#include "2dCC.cpp";
#include "mathHelper.cpp";

//this file contains a pure basis (other than drawing) for 3d custom collision.

namespace d3
{
class nothing{} //this tricks my lsp to obey

class d3Cam
{
	Vector3 centre;
	float rotation; //0 = normal xy, z away from cam, upwards = angle in radians topdown clockwise
	//coords of centre of cam in in-game coordinate system
	Vector2 igCoords;

	d3Cam()
	{
		centre = Vector3(0,0,0);
		Vector2 igCoords;
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
	//distance used for fog by spikes/dust
	float closest;

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
	Vector2 pp1;
	Vector2 pp2;
	Vector2 pp3;
	Vector2 pp4;

	//centre so that it doesn't have to be recalculated every single time
	//since it's already calced everywhere I'll just do a reference to this since that's a lot easier
	Vector3 CamSpaceCentre;

	array<bool> drawnSides(4);

	//behind cam, don't draw
	bool behind;
	//is any side drawn
	bool drawn = true;
	//intersecting with cam plane
	bool intersecting;
	//false for 1-3, true for 2-2
	bool intersectionType;

	//for every point, which side of cam is it on?
	//1 = front
	array<bool> sides(4);

	//optimisations
	//2 far away things can be compared simpler
	float maxDistanceHorisontal = 0;
	float maxDistanceUp = 0;
	float maxDistanceDown = 0;
	//idk for sure if this is a valid optimisation or if something breaks but it sure does save time
	bool simplerPointRelation;
	//things in same convex cluster don't need to be sorted
	int clusterId = -1;
	//un-optimisation: do line checks as well for better sort results
	bool lineComp = false;

	Renderable@ renderable;

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
		if (!drawn) { return; }
		if (colour == 0x00000000 || behind) { return; }

		//debug
		// Vector2 cen = (pp1+pp2+pp3+pp4)/4;
		// script.debugDraw.insertLast(d2Math::Rect(cen.x-20, cen.y-20, cen.x+20, cen.y+20));
		// GetNormalVector(1);
		// GetNormalVector(2);
		// GetNormalVector(3);
		// GetNormalVector(4);

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

		pp1 = Vector2(csp1.x, csp1.y);
		pp2 = Vector2(csp2.x, csp2.y);
		pp3 = Vector2(csp3.x, csp3.y);
		pp4 = Vector2(csp4.x, csp4.y);

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

		CamSpaceCentre = (csp1+csp2+csp3+csp4)/4;

		fac1 = GetSideFacing(1);
		fac2 = GetSideFacing(2);
		fac3 = GetSideFacing(3);
		fac4 = GetSideFacing(4);
	}

	void UpdateDrawn()
	{
		drawn = !behind && ((drawnSides[0] && fac1 > 0) || (drawnSides[1] && fac2 > 0) 
			|| (drawnSides[2] && fac3 > 0) || (drawnSides[3] && fac4 > 0));
	}

	//long dist checks
	void UpdateMaxDist()
	{
		Vector3@ c = (p1+p2+p3+p4)/4;

		Vector2 cf = Vector2(c.x, c.z);
		Vector2 fp1 = Vector2(p1.x, p1.z);
		Vector2 fp2 = Vector2(p2.x, p2.z);
		Vector2 fp3 = Vector2(p3.x, p3.z);
		Vector2 fp4 = Vector2(p4.x, p4.z);

		maxDistanceHorisontal = (cf-fp1).Magnitude();
		float m = (cf-fp2).Magnitude();
		if (m > maxDistanceHorisontal) { maxDistanceHorisontal = m; }
		m = (cf-fp3).Magnitude();
		if (m > maxDistanceHorisontal) { maxDistanceHorisontal = m; }
		m = (cf-fp4).Magnitude();
		if (m > maxDistanceHorisontal) { maxDistanceHorisontal = m; }

		maxDistanceUp = 0;
		maxDistanceDown = 0;
		m = c.y - p1.y;
		if (m > maxDistanceUp) { maxDistanceUp = m; }
		if (m < maxDistanceDown) { maxDistanceDown = m; }
		m = c.y - p2.y;
		if (m > maxDistanceUp) { maxDistanceUp = m; }
		if (m < maxDistanceDown) { maxDistanceDown = m; }
		m = c.y - p3.y;
		if (m > maxDistanceUp) { maxDistanceUp = m; }
		if (m < maxDistanceDown) { maxDistanceDown = m; }
		m = c.y - p4.y;
		if (m > maxDistanceUp) { maxDistanceUp = m; }
		if (m < maxDistanceDown) { maxDistanceDown = m; }
		maxDistanceDown *= -1;
	}

	//med dist checks
	float MaxDistCamLeft()
	{
		Vector3@ c = @CamSpaceCentre;
		float ret = 0;
		float m = c.x-csp1.x;
		if (m > ret) { ret = m; }
		 m = c.x-csp2.x;
		if (m > ret) { ret = m; }
		 m = c.x-csp3.x;
		if (m > ret) { ret = m; }
		 m = c.x-csp4.x;
		if (m > ret) { ret = m; }
		return ret;
	}
	float MaxDistCamRight()
	{
		Vector3@ c = @CamSpaceCentre;
		float ret = 0;
		float m = csp1.x-c.x;
		if (m > ret) { ret = m; }
		 m = csp2.x-c.x;
		if (m > ret) { ret = m; }
		 m = csp3.x-c.x;
		if (m > ret) { ret = m; }
		 m = csp4.x-c.x;
		if (m > ret) { ret = m; }
		return ret;
	}
	//up = +z, down = -z
	float MaxDistCamUp()
	{
		Vector3@ c = @CamSpaceCentre;
		float ret = 0;
		float m = csp1.z-c.z;
		if (m > ret) { ret = m; }
		 m = csp2.z-c.z;
		if (m > ret) { ret = m; }
		 m = csp3.z-c.z;
		if (m > ret) { ret = m; }
		 m = csp4.z-c.z;
		if (m > ret) { ret = m; }
		return ret;
	}
	float MaxDistCamDown()
	{
		Vector3@ c = @CamSpaceCentre;
		float ret = 0;
		float m = c.z-csp1.z;
		if (m > ret) { ret = m; }
		 m = c.z-csp2.z;
		if (m > ret) { ret = m; }
		 m = c.z-csp3.z;
		if (m > ret) { ret = m; }
		 m = c.z-csp4.z;
		if (m > ret) { ret = m; }
		return ret;
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

	Vector3 CoordsFromNode(int point)
	{
		switch (point)
		{
			case 1: return csp1;
			case 2: return csp2;
			case 3: return csp3;
			case 4: return csp4;
		}
		return Vector3();
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
	Vector2 GetIntersection(array<int> pair)
	{
		Vector3 first = CPointByNumber(pair[1]); 
		Vector3 second = CPointByNumber(pair[0]);
		Vector3 dir = second - first;
		if (first.z == second.z) { return Vector2(first.x + second.x, first.y + second.y)/2; }
		dir = dir/abs(first.z - second.z) * abs(first.z);
		return Vector2(dir.x + first.x, dir.y + first.y);
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
		//added some extra normalisation cause the numbers were getting massive -> lots of float impresicion
		Vector3 dir = ((points[0]+points[1]+points[2])/3 - CamSpaceCentre).Normalised();
		Vector3 norm = (points[0] - points[1]).Cross(points[2] - points[1]).Normalised();
		norm = (norm*norm.Dot(dir)).Normalised();

		// debug
		// Vector3 c1 = (points[0]+points[1]+points[2])/3;
		// get_scene().draw_line_world(21,2,c1.x,c1.y,c1.x+norm.x*100,c1.y+norm.y*100,5,0xAA0000FF);
		// get_scene().draw_rectangle_world(21,1,c1.x-10,c1.y-10,c1.x+10,c1.y+10,0,0xFF00FF00);

		return norm;
	}

	//1 = point is under (more z), -1 = point is over (less z), 0 = point not inside
	int PointRelation(Vector3 pos)
	{
		if ((!simplerPointRelation || (drawnSides[0] && fac1 > 0))
			&& d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp1.x, csp1.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp3.x, csp3.y)))
		{
			// puts("upstream nonzero");
			Vector3 norm = GetNormalVector(1);
			if (norm.z * (norm.Dot(pos - (csp1+csp2+csp3)/3)) > 0)
			{
				return 1;
			}
			return -1;
		}
		if ((!simplerPointRelation || (drawnSides[1] && fac2 > 0))
			&& d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp1.x, csp1.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp4.x, csp4.y)))
		{
			// puts("upstream nonzero");
			Vector3 norm = GetNormalVector(2);
			if (norm.z * (norm.Dot(pos - (csp1+csp2+csp4)/3)) > 0)
			{
				return 1;
			}
			return -1;
		}
			if ((!simplerPointRelation || (drawnSides[2] && fac3 > 0))
			&& d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp1.x, csp1.y),
				Vector2(csp3.x, csp3.y),
				Vector2(csp4.x, csp4.y)))

		{
			// puts("upstream nonzero");
			Vector3 norm = GetNormalVector(3);
			if (norm.z * (norm.Dot(pos - (csp1+csp3+csp4)/3)) > 0)
			{
				return 1;
			}
			return -1;

		}
		if ((!simplerPointRelation || (drawnSides[3] && fac4 > 0))
			&& d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp3.x, csp3.y),
				Vector2(csp4.x, csp4.y)))

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

	uint PointOverlaps(Vector3 pos)
	{
		if (d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp1.x, csp1.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp3.x, csp3.y)))
		{
			return 1;
		}
		if (d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp1.x, csp1.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp4.x, csp4.y)))
		{
			return 1;
		}
		if (d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp1.x, csp1.y),
				Vector2(csp3.x, csp3.y),
				Vector2(csp4.x, csp4.y)))

		{
			return 1;
		}
		if (d2Math::PointInTriangle(
				Vector2(pos.x, pos.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp3.x, csp3.y),
				Vector2(csp4.x, csp4.y)))

		{
			return 1;
		}
		return 0;
	}


	//which active side is this screen point on?
	int SideFromPoint(Vector2 pos, bool allowDeactive = false)
	{
		if (d2Math::PointInTriangle(
				pos,
				Vector2(csp1.x, csp1.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp3.x, csp3.y)))
		{
			puts("overlaps 1 " + fac1);
			if ((drawnSides[0] || allowDeactive) && fac1 > 0)
			{
				return 1;
			}
		}
		if (d2Math::PointInTriangle(
				pos,
				Vector2(csp1.x, csp1.y),
				Vector2(csp2.x, csp2.y),
				Vector2(csp4.x, csp4.y)))
		{
			puts("overlaps 2 " + fac2);
			if ((drawnSides[1] || allowDeactive) && fac2 > 0)
			{
				return 2;
			}
		}
			if (d2Math::PointInTriangle(
				pos,
				Vector2(csp1.x, csp1.y),
				Vector2(csp3.x, csp3.y),
				Vector2(csp4.x, csp4.y)))
		{
			puts("overlaps 3 " + fac3);
			if ((drawnSides[2] || allowDeactive) && fac3 > 0)
			{
				return 3;
			}
		}
		if (d2Math::PointInTriangle(
				pos,
				Vector2(csp2.x, csp2.y),
				Vector2(csp3.x, csp3.y),
				Vector2(csp4.x, csp4.y)))
		{
			puts("overlaps 4 " + fac4);
			if ((drawnSides[3] || allowDeactive) && fac4 > 0)
			{
				return 4;
			}
		}
		return -1;
	}

	//line comparisons (more expensive but catch more cases)
	
	//1 = this is behind, 0 = id, -1 = this is front
	int CompareLine(d3Quad@ o, uint i1, uint i2, uint oi1, uint oi2)
	{
		Vector3 point1 = CoordsFromNode(i1);
		Vector3 point2 = CoordsFromNode(i2);
		Vector3 opoint1 = o.CoordsFromNode(oi1);
		Vector3 opoint2 = o.CoordsFromNode(oi2);
		Vector2 point12 = Vector2(point1.x, point1.y);
		Vector2 point22 = Vector2(point2.x, point2.y);
		Vector2 opoint12 = Vector2(opoint1.x, opoint1.y);
		Vector2 opoint22 = Vector2(opoint2.x, opoint2.y);
		d2Math::LineFunc l1 = d2Math::LineFunc(point12, point22);
		d2Math::LineFunc l2 = d2Math::LineFunc(opoint12, opoint22);
		l1.SetBounds(point12, point22);
		l2.SetBounds(opoint12, opoint22);

		//check if they're so far they can't possibly intersect
		if (!l1.CanIntersect(l2)) { return 0; }

		Vector2 intersect = l1.BoundedIntersectionPosition(l2);
		if (intersect == Vector2()) { return 0; }
		float z1 = point1.z + (point2.z - point1.z)*l1.HowFarAlong(intersect);
		float z2 = opoint1.z + (opoint2.z - opoint1.z)*l2.HowFarAlong(intersect);
		if (z1 > z2) { return 1; }
		return -1;
	}

	//sides -> node pairs
	array<array<uint>> GetPairCollection(array<uint> sides)
	{
		array<array<uint>> ret;
		for(uint i = 0; i < sides.length(); i++)
		{
			array<uint> nodes = NodesFromSide(sides[i]);
			array<array<uint>> newPairs = {
			{nodes[0], nodes[1]},
			{nodes[0], nodes[2]},
			{nodes[1], nodes[2]}
			};
			for(uint newPair = 0; newPair < newPairs.length(); newPair++)
			{
				array<uint>@ thisPair = @newPairs[newPair];
				array<uint> altPair = { thisPair[1], thisPair[0] };
				if (ret.find(thisPair) == -1 && ret.find(altPair) == -1)
				{
					ret.insertLast(thisPair);
				}
			}
		}
		return ret;
	}

	array<uint> NodesFromSide(uint side)
	{
		array<uint> points(3);
		switch (side)
		{
			case 1:
				points[0] = 1;
				points[1] = 2;
				points[2] = 3;
				return points;
			case 2:
				points[0] = 1;
				points[1] = 2;
				points[2] = 4;
				return points;
			case 3:
				points[0] = 1;
				points[1] = 3;
				points[2] = 4;
				return points;
			case 4:
				points[0] = 2;
				points[1] = 3;
				points[2] = 4;
				return points;
		}
		return points;
	}

	array<uint> GetActiveSides()
	{
		array<uint> ret;
		if (drawnSides[0] && fac1 > 0) { ret.insertLast(1); }
		if (drawnSides[1] && fac2 > 0) { ret.insertLast(2); }
		if (drawnSides[2] && fac3 > 0) { ret.insertLast(3); }
		if (drawnSides[3] && fac4 > 0) { ret.insertLast(4); }
		return ret;
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

	//stores dust sides of collisionbase, [cbase side, 3dside], 1 indexed
	array<array<uint>> cbaseDust;

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
			collisionBase.base.p4 = collisionBase.base.p3 + Vector2(0.1,0.1);
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

			//cbaseDust
			cbaseDust.resize(0);
			if (dustSides[side1-1])
			{
				array<uint> narr = { 4, side1 };
				cbaseDust.insertLast(narr);
			}
			if (dustSides[side2-1])
			{
				array<uint> narr = { 1, side2 };
				cbaseDust.insertLast(narr);
			}
			if (dustSides[side3-1])
			{
				array<uint> narr = { 2, side3 };
				cbaseDust.insertLast(narr);
			}
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

			//cbaseDust
			cbaseDust.resize(0);
			if (dustSides[side1-1])
			{
				array<uint> narr = { 4, side1 };
				cbaseDust.insertLast(narr);
			}
			if (dustSides[side2-1])
			{
				array<uint> narr = { 1, side2 };
				cbaseDust.insertLast(narr);
			}
			if (dustSides[side3-1])
			{
				array<uint> narr = { 2, side3 };
				cbaseDust.insertLast(narr);
			}
			if (dustSides[side4-1])
			{
				array<uint> narr = { 3, side4 };
				cbaseDust.insertLast(narr);
			}
		}
	}

	void CheckCbaseDust()
	{
		for(uint pair = 0; pair < cbaseDust.length(); pair++)
		{
			if (!collisionBase.dustLines[cbaseDust[pair][0]-1])
			{
				dustSides[cbaseDust[pair][1]-1] = false;
			}
		}
		UpdateIntersectQuad(manager.cam);
	}

	uint GetDustCount()
	{
		uint s = 0;
		if (dustSides[0]) { s += 1; }
		if (dustSides[1]) { s += 1; }
		if (dustSides[2]) { s += 1; }
		if (dustSides[3]) { s += 1; }
		return s;
	}

	void DrawBase(scene@ s)
	{
		//I could make a loop and stuff but like this is literally easier so whatever
		base.Draw(s, layer, sub_layer, manager.script);
		if (base.behind) { return; }
		uint sc = manager.script.ApplyFog(manager.script.spikeColour3d, base.closest);
		uint dc = manager.script.ApplyFog(manager.script.dustColour3d, base.closest);
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

	//any line of this under other return 1 and vice verca same as all the others
	int AnyLineUnder(d3CQuad@ o)
	{
		array<array<uint>> pairs = base.GetPairCollection(base.GetActiveSides());
		array<array<uint>> opairs = o.base.GetPairCollection(o.base.GetActiveSides());

		for(uint i = 0; i < pairs.length(); i++)
		{
			for(uint j = 0; j < opairs.length(); j++)
			{
				int k = base.CompareLine(o.base, pairs[i][0],pairs[i][1],opairs[j][0],opairs[j][1]);
				if (k != 0) { return k; }
			}
		}
		return 0;
	}

	//most of the time the majority of the lines are used anyways so maybe it's better to just do all of them?
	//result: pretty much the same if not slightly worse :/
	int AnyLineUnder2(d3CQuad@ o)
	{
		array<array<uint>> pairs = {
		{1,2},{1,3},{1,4},
		{2,3},{2,4},
		{3,4}
		};
		for(uint i = 0; i < pairs.length(); i++)
		{
			for(uint j = 0; j < pairs.length(); j++)
			{
				int k = base.CompareLine(o.base, pairs[i][0],pairs[i][1],pairs[j][0],pairs[j][1]);
				if (k != 0) { return k; }
			}
		}
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

	//return -1 if behind o
	int opCmp(d3CQuad@ o)
	{
		if (base.behind) { return 1; }
		if (!base.drawn) { return 0; }
		if (!o.base.drawn) { return 0; }
		if (o.base.behind) { return -1; }

		//I thought I could put 1 there, nope, don't do that c:
		if (base.clusterId != -1 && base.clusterId == o.base.clusterId) { return 0; }

		Vector3@ c1 = @base.CamSpaceCentre;
		Vector3@ c2 = @o.base.CamSpaceCentre;

		//long dist checks
		if (abs(c1.z-c2.z) > base.maxDistanceHorisontal + o.base.maxDistanceHorisontal)
		{
			if (c1.z > c2.z)
			{
				return -1;
			}
			return 1;
		}

		if (abs(c1.x-c2.x) > base.maxDistanceHorisontal + o.base.maxDistanceHorisontal
			|| c1.y-c2.y > base.maxDistanceUp + o.base.maxDistanceDown
			|| c2.y-c1.y > base.maxDistanceDown + o.base.maxDistanceUp)
		{
			return 0;
		}

		//medium dist checks
		if (c1.z > c2.z)
		{
			if (c1.z-c2.z > base.MaxDistCamDown() + o.base.MaxDistCamUp())
			{
				return -1;
			}
		}
		else
		{
			if (c2.z-c1.z > base.MaxDistCamUp() + o.base.MaxDistCamDown())
			{
				return 1;
			}
		}
		if (c1.x > c2.x)
		{
			if (c1.x-c2.x > base.MaxDistCamLeft() + o.base.MaxDistCamRight())
			{
				return 0;
			}
		}
		else
		{
			if (c2.x-c1.x > base.MaxDistCamRight() + o.base.MaxDistCamLeft())
			{
				return 0;
			}
		}

		//long checks :c
		Vector2 cf1 = Vector2(c1.x, c1.y);
		Vector2 cf2 = Vector2(c2.x, c2.y);

		int r = AnyPointUnder(o);
		if (r != 0) { return -r; }
		r = o.AnyPointUnder(this);
		if (r != 0) { return r; }

		r = AnyLineUnder(o);
		if (r != 0) { return -r; }
		return o.AnyLineUnder(this);
	}

	//since this is THE lag centre I'm making an extra copy for this to
	//not fuck over fps even more with 5 million useless ifs
	int opCmpDraw(d3CQuad@ o)
	{
		if (base.behind) { return 1; }
		if (!base.drawn) { return 0; }
		if (!o.base.drawn) { return 0; }
		if (o.base.behind) { return -1; }

		//I thought I could put 1 there, nope, don't do that c:
		if (base.clusterId != -1 && base.clusterId == o.base.clusterId) { return 0; }

		Vector3@ c1 = @base.CamSpaceCentre;
		Vector3@ c2 = @o.base.CamSpaceCentre;
		//long dist checks
		if (abs(c1.z-c2.z) > base.maxDistanceHorisontal + o.base.maxDistanceHorisontal)
		{
			if (c1.z > c2.z)
			{
				return -1;
			}
			return 1;
		}

		if (abs(c1.x-c2.x) > base.maxDistanceHorisontal + o.base.maxDistanceHorisontal
			|| c1.y-c2.y > base.maxDistanceUp + o.base.maxDistanceDown
			|| c2.y-c1.y > base.maxDistanceDown + o.base.maxDistanceUp)
		{
			return 0;
		}

		get_scene().draw_line_world(21,1,c1.x,c1.y,c2.x,c2.y,3,0xAAFFFF00);
		// get_scene().draw_line_world(21,1,c1.x,c1.y,c1.x,c1.y+base.maxDistanceDown,3,0xFF0000FF);
		// get_scene().draw_line_world(21,1,c1.x,c1.y,c1.x,c1.y-base.maxDistanceUp,3,0xFF0000FF);
		//medium dist checks
		if (c1.z > c2.z)
		{
			if (c1.z-c2.z > base.MaxDistCamDown() + o.base.MaxDistCamUp())
			{
				return -1;
			}
		}
		else
		{
			if (c2.z-c1.z > base.MaxDistCamUp() + o.base.MaxDistCamDown())
			{
				return 1;
			}
		}
		if (c1.x > c2.x)
		{
			if (c1.x-c2.x > base.MaxDistCamLeft() + o.base.MaxDistCamRight())
			{
				return 0;
			}
		}
		else
		{
			if (c2.x-c1.x > base.MaxDistCamRight() + o.base.MaxDistCamLeft())
			{
				return 0;
			}
		}

		//short checks :c
		Vector2 cf1 = Vector2(c1.x, c1.y);
		Vector2 cf2 = Vector2(c2.x, c2.y);

		get_scene().draw_line_world(21,2,c1.x,c1.y,c2.x,c2.y,5,0xAA0000FF);
		// get_scene().draw_rectangle_world(21,3,c1.x-10, c1.y-10, c1.x+10,c1.y+10, 0,0xFF0000FF); 
		// get_scene().draw_rectangle_world(21,3,c2.x-10, c2.y-10, c2.x+10,c2.y+10, 0,0xFF0000FF); 
		// if (c1.x-c2.x > 0)
		// {
		// 	get_scene().draw_line_world(21,2,c1.x,c1.y,c1.x-base.maxDistanceHorisontal,c1.y,5,0xFFFFFF00);
		// }
		// else
		// {
		// 	get_scene().draw_line_world(21,2,c1.x,c1.y,c1.x+base.maxDistanceHorisontal,c1.y,5,0xFFFFFF00);
		// }

		int r = AnyPointUnder(o);
		if (r != 0) { return -r; }
		r = o.AnyPointUnder(this);
		// return r;
		if ((!base.lineComp && !o.base.lineComp) || r != 0) { return r; }

		get_scene().draw_line_world(21,1,c1.x,c1.y,c2.x,c2.y,5,0xFFFFFFFF);

		r = AnyLineUnder(o);
		if (r != 0) { return -r; }
		return o.AnyLineUnder(this);
	}

	//I hate doing this but I need to for re-enable side in node cluster
	int opCmpForce(d3CQuad@ o)
	{
		Vector3@ c1 = @base.CamSpaceCentre;
		Vector3@ c2 = @o.base.CamSpaceCentre;

		Vector2 cf1 = Vector2(c1.x, c1.y);
		Vector2 cf2 = Vector2(c2.x, c2.y);

		int r = AnyPointUnder(o);
		if (r != 0) { return -r; }
		r = o.AnyPointUnder(this);
		// return r;
		if ((!base.lineComp && !o.base.lineComp) || r != 0) { return r; }

		r = AnyLineUnder(o);
		if (r != 0) { return -r; }
		return o.AnyLineUnder(this);
	}
}	

class gluggligjdh{}


//interface for 2d renderables
abstract class d3FlatDrawable
{
	d2Math::Rect drawRect;
	float depth;
	
	void Draw(scene@ s){}
}

//this was originally written for quad/enemy/prop before
//I realised how that was kinda pointless so that's why it
//reads a bit weird :p
class Renderable
{
	//0 = quad, 1 = flat
	uint type;
	d3CQuad@ quad;
	d3FlatDrawable@ flat;

	Renderable(){}
	Renderable(uint type) { this.type = type; }

	int opCmp(Renderable@ o)
	{
		//both quads
		if (type == 0 && o.type == 0)
		{
			return quad.opCmp(o.quad);
		}
		if (type == 0)
		{
			return -o.opCmp(this);
		}
		//both flat
		if (o.type != 0)
		{
			float d = flat.depth;
			float od = o.flat.depth;

			if (d < od) { return 1; }
			if (d > od) { return -1; }
			return 0;
		}
		//other quad, this one not

		Vector2 rcen2 = (flat.drawRect.p1 + flat.drawRect.p2)/2;
		Vector3 c1 = Vector3(rcen2.x, rcen2.y, flat.depth);
		float rmagx = abs(rcen2.x - flat.drawRect.p1.x);
		float rmagy = abs(rcen2.y - flat.drawRect.p1.y);
		Vector3@ c2 = @o.quad.base.CamSpaceCentre;

		//long dist checks
		if (abs(c1.z-c2.z) > o.quad.base.maxDistanceHorisontal)
		{
			if (c1.z > c2.z)
			{
				return -1;
			}
			return 1;
		}
		if (abs(c1.x-c2.x) > rmagx + o.quad.base.maxDistanceHorisontal
			|| c1.y-c2.y > rmagy + o.quad.base.maxDistanceDown
			|| c2.y-c1.y > rmagy + o.quad.base.maxDistanceUp) 
		{
			return 0;
		}

		//med distance checks
		if (c1.z > c2.z)
		{
			if (c1.z-c2.z > o.quad.base.MaxDistCamUp())
			{
				return -1;
			}
		}
		else
		{
			if (c2.z-c1.z > o.quad.base.MaxDistCamDown())
			{
				return 1;
			}
		}
		if (c1.x > c2.x)
		{
			if (c1.x-c2.x > rmagx + o.quad.base.MaxDistCamRight())
			{
				return 0;
			}
		}
		else
		{
			if (c2.x-c1.x > rmagx + o.quad.base.MaxDistCamLeft())
			{
				return 0;
			}
		}

		d2Math::Rect drect = flat.drawRect; 
		float depth = flat.depth; 
		// puts("depth" + depth);
		
		int i = AnyPointRUnderQ(o.quad, drect, depth);
		// puts("i1: " + (-i));
		if (i != 0) { return -i; }
		i = AnyPointQUnderR(o.quad, drect, depth);
		// puts("i2: " + i);
		return i;
	}

	//since this is THE lag centre I'm making an extra copy for this to
	//not fuck over fps even more with 5 million useless ifs
	int opCmpDraw(Renderable@ o)
	{
		//both quads
		if (type == 0 && o.type == 0)
		{
			return quad.opCmpDraw(o.quad);
		}
		if (type == 0)
		{
			return -o.opCmp(this);
		}
		//both flat
		if (o.type != 0)
		{
			float d = flat.depth;
			float od = o.flat.depth;

			if (d < od) { return 1; }
			if (d > od) { return -1; }
			return 0;
		}
		//other quad, this one not

		Vector2 rcen2 = (flat.drawRect.p1 + flat.drawRect.p2)/2;
		Vector3 c1 = Vector3(rcen2.x, rcen2.y, flat.depth);
		float rmagx = abs(rcen2.x - flat.drawRect.p1.x);
		float rmagy = abs(rcen2.y - flat.drawRect.p1.y);
		Vector3@ c2 = @o.quad.base.CamSpaceCentre;

		//long dist checks
		if (abs(c1.z-c2.z) > o.quad.base.maxDistanceHorisontal)
		{
			if (c1.z > c2.z)
			{
				return -1;
			}
			return 1;
		}
		if (abs(c1.x-c2.x) > rmagx + o.quad.base.maxDistanceHorisontal
			|| c1.y-c2.y > rmagy + o.quad.base.maxDistanceDown
			|| c2.y-c1.y > rmagy + o.quad.base.maxDistanceUp) 
		{
			return 0;
		}

		get_scene().draw_line_world(21,1,c1.x,c1.y,c2.x,c2.y,3,0xAAFFFF00);

		//med distance checks
		if (c1.z > c2.z)
		{
			if (c1.z-c2.z > o.quad.base.MaxDistCamUp())
			{
				return -1;
			}
		}
		else
		{
			if (c2.z-c1.z > o.quad.base.MaxDistCamDown())
			{
				return 1;
			}
		}
		if (c1.x > c2.x)
		{
			if (c1.x-c2.x > rmagx + o.quad.base.MaxDistCamRight())
			{
				return 0;
			}
		}
		else
		{
			if (c2.x-c1.x > rmagx + o.quad.base.MaxDistCamLeft())
			{
				return 0;
			}
		}

		get_scene().draw_line_world(21,2,c1.x,c1.y,c2.x,c2.y,5,0xAA0000FF);

		d2Math::Rect drect = flat.drawRect; 
		float depth = flat.depth; 
		// puts("depth" + depth);
		
		int i = AnyPointRUnderQ(o.quad, drect, depth);
		// puts("i1: " + (-i));
		if (i != 0) { return -i; }
		i = AnyPointQUnderR(o.quad, drect, depth);
		// puts("i2: " + i);
		return i;
	}

	//is any point of rect under quad
	//-1 = r in front, 0 = idk, 1 = r behind
	int AnyPointRUnderQ(d3CQuad@ q, d2Math::Rect r, float depth)
	{
		//debug
		// if (!(cast<d3FlatObjectBase>(flat) is null) && @cast<d3FlatObjectBase>(flat).script != null)
		// {
		// 	Vector3@ p1 = q.base.csp1;
		// 	Vector3@ p2 = q.base.csp2;
		// 	Vector3@ p3 = q.base.csp3;
		// 	Vector3@ p4 = q.base.csp4;
		// 	cast<d3FlatObjectBase>(flat).script.debugDraw.insertLast(r);
		// 	cast<d3FlatObjectBase>(flat).script.debugDraw.insertLast(
		// 		d2Math::Rect(p1.x-5, p1.y-5, p1.x+5, p1.y+5));
		// 	cast<d3FlatObjectBase>(flat).script.debugDraw.insertLast(
		// 		d2Math::Rect(p2.x-5, p2.y-5, p2.x+5, p2.y+5));
		// 	cast<d3FlatObjectBase>(flat).script.debugDraw.insertLast(
		// 		d2Math::Rect(p3.x-5, p3.y-5, p3.x+5, p3.y+5));
		// 	cast<d3FlatObjectBase>(flat).script.debugDraw.insertLast(
		// 		d2Math::Rect(p4.x-5, p4.y-5, p4.x+5, p4.y+5));
		// }

		int i;
		i = q.base.PointRelation(Vector3(r.x1, r.y1, depth));
		if (i != 0) { return i; }
		i = q.base.PointRelation(Vector3(r.x2, r.y2, depth));
		if (i != 0) { return i; }
		i = q.base.PointRelation(Vector3(r.x1, r.y2, depth));
		if (i != 0) { return i; }
		i = q.base.PointRelation(Vector3(r.x2, r.y1, depth));
		return i;
	}
	//is any point of quad under rect
	//-1 = q in front, 0 = idk, 1 = q behind
	int AnyPointQUnderR(d3CQuad@ q, d2Math::Rect r, float depth)
	{
		Vector3@ p1 = q.base.csp1;
		Vector3@ p2 = q.base.csp2;
		Vector3@ p3 = q.base.csp3;
		Vector3@ p4 = q.base.csp4;

		if (r.PointInside(Vector2(p1.x,p1.y)))
		{
			if (p1.z < depth) { return -1; }
			return 1;
		}
		if (r.PointInside(Vector2(p2.x,p2.y)))
		{
			if (p2.z < depth) { return -1; }
			return 1;
		}
		if (r.PointInside(Vector2(p3.x,p3.y)))
		{
			if (p3.z < depth) { return -1; }
			return 1;
		}
		if (r.PointInside(Vector2(p4.x,p4.y)))
		{
			if (p4.z < depth) { return -1; }
			return 1;
		}
		return 0;
	}

	bool IsDrawn()
	{
		switch (type)
		{
			case 0:
				return quad.base.drawn;
			case 1:
				//this might sadly cut off really big enemies before their thickness
				//runs out but oh well I need that performance :p
				return flat.depth > -1000;
		}
		return false;
	}

	void Draw(scene@ s)
	{
		switch (type)
		{
			case 0:
				// puts("draw quad!");
				quad.DrawBase(s);
				break;
			case 1:
				// puts("draw flat!");
				flat.Draw(s);
				break;
		}
	}
}

class d3Manager
{
	array<d3CQuad@> allQuads;
	array<Renderable@> renderables;
	//holds the drawn subset of renderables
	array<Renderable@> nextRender;
	d2::CollisionManager@ manager;
	d3Cam@ cam;

	script@ script;
	
	d3Manager()
	{
		@manager = @d2::CollisionManager();
		@cam = @d3Cam();
	}

	void UpdateLooks(bool force)
	{
		for (uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].base.ApplyProjection(cam);
			allQuads[i].UpdateIntersectQuad(cam);
		}
		if (!force && script.layerFrameCounter < script.layerFrames) { script.layerFrameCounter++; return; }
		script.layerFrameCounter = 0;
		SortRenderList();
	}

	void UpdateCollision()
	{
		for (uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].collisionBase.UpdateCollision(false);
		}

	}

	//I'll make some scuffed merge sort for this I don't care anymore
	//(the built in array.sortAsc() appears to be full of shit)
	void SortRenderList()
	{
		//only render what you absolutely need to c:
		nextRender.resize(0);
		for(uint i = 0; i < renderables.length(); i++)
		{
			if (renderables[i].IsDrawn())
			{
				nextRender.insertLast(renderables[i]);
			}
		}
		if (!script.layerDebug)
		{
			BetterInsSort(nextRender);
		}
		else
		{
			BetterInsSortDraw(nextRender);
		}
	}

	//plz be better plz be better plz be better (spoiler alert it's 1000x better c:)
	void BetterInsSort(array<Renderable@>@ arr)
	{
		// puts("");
		// puts("sort start!!!");
		uint len = arr.length();
		if (len < 2) { return; }
		for (uint i = 0; i < len; i++)
		{
	   		Renderable@ e = arr[i];
	   		bool success = false;
			for (uint j = i; j >= 1; j--)
			{
				//I'll just use the raw int it's clearer honestly :p
				int c = e.opCmp(arr[j-1]); 
				// puts("comparing old " + i + " to " + (j-1));
				if (c <= 0)
				{
					// puts("moving " + (j-1) + " -> " + j);
					@arr[j] = @arr[j-1];
				}
				else
				{
					// puts("setting old " + i + " -> " + j);
					@arr[j] = @e;
					success = true;
					break;
				}
			}
			if (!success)
			{
				// puts("new behindest old " + i + " ->  0");
				@arr[0] = @e;
			}
		}
	}

	//since this is THE lag centre I'm making an extra copy for this to
	//not fuck over fps even more with 5 million useless ifs
	void BetterInsSortDraw(array<Renderable@>@ arr)
	{
		// puts("");
		// puts("sort start!!!");
		uint len = arr.length();
		if (len < 2) { return; }
		for (uint i = 0; i < len; i++)
		{
	   		Renderable@ e = arr[i];
	   		bool success = false;
			for (uint j = i; j >= 1; j--)
			{
				//I'll just use the raw int it's clearer honestly :p
				int c = e.opCmpDraw(arr[j-1]); 
				// puts("comparing old " + i + " to " + (j-1));
				if (c <= 0)
				{
					// puts("moving " + (j-1) + " -> " + j);
					@arr[j] = @arr[j-1];
				}
				else
				{
					// puts("setting old " + i + " -> " + j);
					@arr[j] = @e;
					success = true;
					break;
				}
			}
			if (!success)
			{
				// puts("new behindest old " + i + " ->  0");
				@arr[0] = @e;
			}
		}
	}

	void Draw()
	{
		scene@ s = get_scene();
		// puts("renderables: " + renderables.length());
		for (uint i = 0; i < nextRender.length(); i++)
		{
			nextRender[i].Draw(s);
		}
		//intersects unlayered
		for (uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].DrawIntersect(s);
		}
	}

	void Step()
	{
		manager.step();
		for(uint i = 0; i < allQuads.length(); i++)
		{
			allQuads[i].CheckCbaseDust();
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
	void RemoveRenderable(Renderable@ r)
	{
		for (uint i = 0; i < renderables.length(); i++)
		{
			if (renderables[i] is r)
			{
				renderables.removeAt(i);
			}
		}
	}
}

}
