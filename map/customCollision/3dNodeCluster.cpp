#include "3dCC.cpp";

class d3NodeCluster : trigger_base 
{
	[text] int layer;
	[text] int sub_layer;
	[colour,alpha] uint d2colour;
	[colour,alpha] uint d3colour;

	//when nodes are deleted they're just set to 0
	//to preserve quad nodes so work around that c:
	[hidden] array<Vector3> nodes;
	array<uint> selectedNodes;

	d2Math::Vector2 oldCentre;

	//for dragging nodes
	d2Math::Vector2 oldMousePos;
	bool heldLastFrame = false;

	d3::d3Manager@ manager;
	//stores node indecies of each quad
	//each int array should be 4 indecies long
	[hidden] array<array<uint>> quadNodes;
	array<d3::d3CQuad@> quads;

	scripttrigger@ self;
	script@ script;
	input_api@ input;

	void init(script@ s, scripttrigger@ self)
	{
		@script = @s;
		@this.self = @self;
		@input = @get_input_api();
		@this.manager = @s.manager; 
		if (d2colour == 0x00000000)
		{
			d2colour = s.default2dCol;
		}
		if (d3colour == 0x00000000)
		{
			d3colour = s.default3dCol;
		}
		oldCentre = d2Math::Vector2(self.x(), self.y());
		if (layer == 0)
		{
			layer = 18;
		}
		if (sub_layer == 0)
		{
			sub_layer = 1;
		}
		InitQuads();
	}

	void checkpoint_load() { InitQuads(); }
	void on_level_start() { InitQuads(); }

	void editor_step()
	{
		if(script.editor.editor_tab() == "Triggers" 
			&& @script.editor.get_selected_trigger() != null
	 		&& script.editor.get_selected_trigger().is_same(self.as_entity()))
		{
			//add node
			if (input.key_check_pressed_gvb(18) ||
				(input.key_check_pressed_vk(0x41) && input.key_check_gvb(10) && selectedNodes.length() == 0))
			{
				AddNode(manager.cam.centre);
			}
			//select node
			if (input.key_check_pressed_gvb(2) && input.key_check_gvb(10))
			{
				d2Math::Vector2 mpos = d2Math::Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
				int cb = -1;
				float cbz = 0;
				for (uint i = 0; i < nodes.length(); i++)
				{
					Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
					Vector3 pos = manager.cam.WorldToCamPos(nodes[i]) + centre;
					float w = GetNodeRadius(i);
					if (abs(mpos.x-pos.x) < w && abs(mpos.y-pos.y) < w &&
						selectedNodes.find(i) == -1 &&
						(cb == -1 || cbz > pos.z))
					{
						cb = i;
						cbz = pos.z;
					}
				}
				if (cb != -1)
				{
					selectedNodes.insertLast(cb);
					puts("selected: " + cb);
				}
			}
			//deselect node
			if (input.key_check_pressed_gvb(3) && input.key_check_gvb(10))
			{
				d2Math::Vector2 mpos = d2Math::Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
				int cb = -1;
				float cbz = 0;
				for (uint i = 0; i < nodes.length(); i++)
				{
					Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
					Vector3 pos = manager.cam.WorldToCamPos(nodes[i]) + centre;
					float w = GetNodeRadius(i);
					if (abs(mpos.x-pos.x) < w && abs(mpos.y-pos.y) < w &&
						selectedNodes.find(i) != -1 &&
						(cb == -1 || cbz > pos.z))
					{
						cb = i;
						cbz = pos.z;
					}
				}
				if (cb != -1)
				{
					selectedNodes.removeAt(selectedNodes.find(cb));
				}
			}
			//deselect all
			if (input.key_check_pressed_gvb(5))
			{
				selectedNodes.resize(0);
			}
			//delete nodes
			if (input.key_check_pressed_gvb(22))
			{
				for (uint n = 0; n < selectedNodes.length(); n++)
				{
					//puts("node being removed " + selectedNodes[n]);
					//this is looped backwards to ensure removal doesn't
					//mess with indexes
					for (int q = quadNodes.length()-1; q >= 0; q--)
					{
						// puts("quad at test " + quadNodes[q][0] + ", " +
						// 	quadNodes[q][1] + ", " + 
						// 	quadNodes[q][2] + ", " + 
						// 	quadNodes[q][3]);
						if (quadNodes[uint(q)].find(selectedNodes[n]) != -1)
						{
							puts("removing!");
							manager.RemoveQuad(quads[uint(q)]);
							quads.removeAt(q);
							quadNodes.removeAt(q);
						}
					}
					nodes[selectedNodes[n]] = Vector3();
				}
				selectedNodes.resize(0);
				SetActiveSidesAll();
			}
			//add quad
			if (input.key_check_pressed_vk(0x41) && input.key_check_gvb(10)
				&& selectedNodes.length() == 4)
			{
				selectedNodes.sortAsc();
				if (quadNodes.find(selectedNodes) != -1) { return; }
				quadNodes.insertLast(selectedNodes);
				d3::d3CQuad nq;
				nq.layer = layer;
				nq.sub_layer = sub_layer;
				nq.base.colour = d3colour;
				nq.collisionBase.base.colour = d2colour;
				@nq.collisionBase.manager = @manager.manager;
				@nq.manager = @manager;
				quads.insertLast(@nq);
				manager.allQuads.insertLast(@nq);
				UpdatePositions();
				SetActiveSidesAll();
				manager.UpdateLooks();
			}

			//delete quad
			if (input.key_check_pressed_vk(0x44) && input.key_check_gvb(10))
			{
				Vector3 mpos = Vector3(input.mouse_x_world(21), input.mouse_y_world(21), 0);
				int bq = -1;
				for (uint q = 0; q < quadNodes.length(); q++)
				{
					if (quads[q].base.PointRelation(mpos) != 0 &&
						(bq == -1 || quads[q] < quads[bq]))
					{
						bq = q;
					}
				}
				if (bq != -1)
				{
					manager.RemoveQuad(quads[bq]);
					quads.removeAt(bq);
					quadNodes.removeAt(bq);
				}
			}

			//drag
			if (!input.key_check_gvb(2) || input.key_check_gvb(10)) { heldLastFrame = false; }
			if (input.key_check_gvb(2) && !heldLastFrame && !input.key_check_gvb(10))
			{
				heldLastFrame = true;
				oldMousePos = d2Math::Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
			}
			if (input.key_check_gvb(2) && heldLastFrame && !input.key_check_gvb(10))
			{
				d2Math::Vector2 curMouse = d2Math::Vector2(input.mouse_x_world(21), input.mouse_y_world(21));
				d2Math::Vector2 diff = curMouse - oldMousePos;
				Vector3 diff3 = manager.cam.CamToWorldDir(Vector3(diff.x, diff.y, 0));
				if (selectedNodes.length() == 0)
				{
					for (uint i = 0; i < nodes.length(); i++)
					{
						if (nodes[i] == Vector3()) { continue; }
						nodes[i] += diff3;
					}
				}
				else
				{
					for (uint i = 0; i < selectedNodes.length(); i++)
					{
						if (nodes[selectedNodes[i]] == Vector3()) { continue; }
						nodes[selectedNodes[i]] += diff3;
					}
				}
				oldMousePos = curMouse;
				UpdatePositions();
				manager.UpdateLooks();
			}
		}
	}

	void editor_draw(float ok)
	{
		scene@ s = get_scene();
		if(script.editor.editor_tab() == "Triggers" 
			&& @script.editor.get_selected_trigger() != null
	 		&& script.editor.get_selected_trigger().is_same(self.as_entity()))
		{
			for (uint i = 0; i < nodes.length(); i++)
			{
				if (nodes[i] == Vector3()) { continue; }
				Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
				Vector3 pos = manager.cam.WorldToCamPos(nodes[i]) + centre;
				float w = GetNodeRadius(i);
				uint c;
				if (selectedNodes.find(i) == -1)
				{
					c = 0xBBFF0055;
					if (pos.z < -10) { c = 0x22FF0055; }
				}
				else
				{
					c = 0xBB0055FF;
					if (pos.z < -10) { c = 0x220055FF; }
				}
				s.draw_rectangle_world(21,1, pos.x-w, pos.y-w, pos.x+w, pos.y+w, 0, c);
			}
		}
	}

	//finds first available spot and adds the node, returns index of node
	uint AddNode(Vector3 n)
	{
		for (uint i = 0; i < nodes.length(); i++)
		{
			if (nodes[i] == Vector3())
			{
				nodes[i] = n;
				return i;
			}
		}
		nodes.insertLast(n);
		return nodes.length()-1;
	}

	//it's a square but radius as in width/2 :p
	float GetNodeRadius(uint n)
	{
		Vector3 centre = Vector3(manager.cam.igCoords.x, manager.cam.igCoords.y, 0);
		Vector3 pos = manager.cam.WorldToCamPos(nodes[n]) + centre;
		float w = 10 + 0.5*pow(2.72, 2.6-pos.z/100);
		if (w > 25) { w = 25; }
		return w;
	}

	//initialises the quad list. only call in init!!!
	void InitQuads()
	{
		quads.resize(0);
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			d3::d3CQuad nq;
			nq.layer = layer;
			nq.sub_layer = sub_layer;
			nq.base.colour = d3colour;
			nq.collisionBase.base.colour = d2colour;
	   		@nq.collisionBase.manager = @manager.manager;
			@nq.manager = @manager;
	   		quads.insertLast(@nq);
			manager.allQuads.insertLast(@nq);
		}
		UpdatePositions();
		SetActiveSidesAll();
		// puts("quads " + quads.length());
	}

	void UpdatePositions()
	{
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			d3::d3CQuad@ q = quads[i];
			array<uint>@ p = @quadNodes[i];
			q.base.p1 = nodes[p[0]];
			q.base.p2 = nodes[p[1]];
			q.base.p3 = nodes[p[2]];
			q.base.p4 = nodes[p[3]];

			//shrinking the quads slightly so that layering can work
			Vector3 c = q.base.Find3dCentre();
			q.base.p1 += (c-q.base.p1)/1000;
			q.base.p2 += (c-q.base.p2)/1000;
			q.base.p3 += (c-q.base.p3)/1000;
			q.base.p4 += (c-q.base.p4)/1000;
		}
	}

	void SetActiveSidesAll()
	{
		// puts("quad 0: " + quadNodes[0][0]+","+quadNodes[0][1]+","+quadNodes[0][2]+","+quadNodes[0][3]);
		// puts("quad 1: " + quadNodes[1][0]+","+quadNodes[1][1]+","+quadNodes[1][2]+","+quadNodes[1][3]);
		ActivateAllSides();
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			// puts("");
			// puts("");
			// puts("doing quad " + i + "!!!!");
			DealWithSharedTrigs(i);
		}
	}

	//finds all other quads that share a trig with
	//this one and disable both of the sides
	void DealWithSharedTrigs(uint quad)
	{
		UpdatePositions();
		array<uint>@ q = quadNodes[quad];

		array<uint> m1 = FindSharedTrig(q[0], q[1], q[2]);
		array<uint> m2 = FindSharedTrig(q[0], q[1], q[3]);
		array<uint> m3 = FindSharedTrig(q[0], q[2], q[3]);
		array<uint> m4 = FindSharedTrig(q[1], q[2], q[3]);
		// puts("m1: " + m1.length());
		// puts("m2: " + m2.length());
		// puts("m3: " + m3.length());
		// puts("m4: " + m4.length());

		if (m1.length() > 1)
		{
			for (uint i = 0; i < m1.length(); i++)
			{
				int side = SideFromNodes(m1[i], q[0], q[1], q[2]);
				if (side < 0) { continue; }
				quads[m1[i]].activeSides[side-1] = false;
				quads[m1[i]].base.drawnSides[side-1] = false;
			}
		}
		if (m2.length() > 1)
		{
			for (uint i = 0; i < m2.length(); i++)
			{
				int side = SideFromNodes(m2[i], q[0], q[1], q[3]);
				if (side < 0) { continue; }
				quads[m2[i]].activeSides[side-1] = false;
				quads[m2[i]].base.drawnSides[side-1] = false;
			}
		}
		if (m3.length() > 1)
		{
			for (uint i = 0; i < m3.length(); i++)
			{
				int side = SideFromNodes(m3[i], q[0], q[2], q[3]);
				if (side < 0) { continue; }
				quads[m3[i]].activeSides[side-1] = false;
				quads[m3[i]].base.drawnSides[side-1] = false;
			}
		}
		if (m4.length() > 1)
		{
			for (uint i = 0; i < m4.length(); i++)
			{
				int side = SideFromNodes(m4[i], q[1], q[2], q[3]);
				if (side < 0) { continue; }
				quads[m4[i]].activeSides[side-1] = false;
				quads[m4[i]].base.drawnSides[side-1] = false;
			}
		}
	}

	void ActivateAllSides()
	{
		array<bool> a = {true, true, true, true};
		for (uint i = 0; i < quads.length(); i++)
		{
	   		d3::d3CQuad@ q = quads[i];
			q.activeSides = a;
			q.base.drawnSides = a;
		}
	}

	int SideFromNodes(uint quad, uint n1, uint n2, uint n3)
	{
		array<uint>@ q = quadNodes[quad];
		array<uint> na = {n1, n2, n3};
		array<uint> s1 = {q[0], q[1], q[2]};
		array<uint> s2 = {q[0], q[1], q[3]};
		array<uint> s3 = {q[0], q[2], q[3]};
		array<uint> s4 = {q[1], q[2], q[3]};
		if (IsSameArr(na, s1)) { return 1; }
		if (IsSameArr(na, s2)) { return 2; }
		if (IsSameArr(na, s3)) { return 3; }
		if (IsSameArr(na, s4)) { return 4; }
		return -1;
	}

	bool IsSameArr(array<uint> a1, array<uint> a2)
	{
		// puts("is same");
		// puts("a1: " + a1[0] + ", " + a1[1] + ", " + a1[2]);
		// puts("a2: " + a2[0] + ", " + a2[1] + ", " + a2[2]);
		a1.sortAsc();
		a2.sortAsc();
		// puts("same? " + (a1 == a2));
		return a1 == a2;
	}

	array<uint> FindSharedTrig(uint n1, uint n2, uint n3)
	{
		array<uint> ret;
		for (uint i = 0; i < quadNodes.length(); i++)
		{
			array<uint>@ l = quadNodes[i];
			if (l.find(n1) != -1 && l.find(n2) != -1 && l.find(n3) != -1)
			{
				// puts("found shared!");
				// puts("ns: " + n1+", "+n2+", "+n3);
				// puts("quad: " + l[0]+", "+l[1]+", "+l[2]+", "+l[3]);
				ret.insertLast(i);
			}
		}
		return ret;
	}

	void on_remove()
	{
		for (uint i = 0; i < quads.length(); i++)
		{
			//error safety is overrated
			manager.allQuads.removeAt(uint(manager.allQuads.find(quads[0])));
		}
	}
}
