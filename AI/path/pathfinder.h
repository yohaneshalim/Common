//
// 2017-06-19, jjuiddong
//
//
#pragma once


namespace ai
{

	struct sVertex
	{
		enum {MAX_EDGE = 5};

		Vector3 pos;
		int edge[MAX_EDGE];
		bool visit;
		bool check;
		bool reg;
		float startLen;
		float endLen;
	};

	class cPathFinder
	{
	public:
		cPathFinder();
		virtual ~cPathFinder();
		bool Create(const int vertexCount);
		bool AddVertex(const sVertex &vtx);
		bool RemoveVertex(const int index);
		bool Find(const Vector3 &start, const Vector3 &end,
			OUT vector<Vector3> &out);
		int GetNearestVertex(const Vector3 &pos) const;


	protected:
		float Distance(const Vector3 &p0, const Vector3 &p1) const;


	public:
		vector<sVertex> m_vertices;
	};

}
