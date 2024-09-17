class Replay 
	{
		ReplayData data;
		string character = "dustman";

		dustman@ object;
		private bool active = false;

		Replay(){}
		Replay(uint accuracy, string character) 
		{ 
			data = ReplayData(accuracy); 
			this.character = character;
		}

		void SetActive(bool active) 
		{
			this.active = active;
			if (active) 
			{
				this.object = create_entity(NameToEnt(character));
				Update(0);
			}
			else 
			{
				get_scene().remove_entity(object);
			}
		}

		void Update(int subframe) 
		{
			if (!active) { return; }

			ReplaySnapshot@ snap = data.GetData(subframe);
			if (@snap == null) { return; }

			object.x(snap.x);
			object.y(snap.y);
			object.state(snap.state);
		}
	}

class ReplayData
{
	uint accuracy = 1; //snapshots every 2^accuracy subframes
	array<ReplaySnapshot@> snapshots;

	ReplayData() {}
	ReplayData(uint accuracy) { this.accuracy = accuracy; }

	void RecieveData(ReplaySnapshot@ snap, int subframes) //subframe 0 = first subframe of replay
	{
		int index = floor(((float)subframes) / accuracy);
		if (snapshots.length() <= index) 
		{
			snapshots.insertLast(snap);
		}
	}

	ReplaySnapshot@ GetData(int subframe) 
	{
		int index = floor(((float)subframes) / accuracy);
		if (index * accuracy != subframe) 
		{
			return null;
		}
		return snapshots[uint(index)];
	}
}

class ReplaySnapshot 
	{
		float x;
		float y;
		int state;

		ReplaySnapshot() {}
		ReplaySnapshot(float x, float y, int state) 
		{
			this.x = x;
			this.y = y;
			this.state = state;
		}
	}

string NameToEnt(string name) 
{
	switch (name) 
	{
		case "dustman":
			return "dust_man";
		case "dustgirl":
			return "dust_girl";
		case "dustkid":
			return "dust_girl";
		case "dustworth":
			return "dust_worth";
		default:
			return "incorrect char name you fucking moron piece of shit just code better bruhhh";
	}
}
