//
// 2017-05-13, jjuiddong
// Debug Arrow Mesh
//
#pragma once


namespace graphic
{

	class cDbgArrow
	{
	public:
		cDbgArrow();
		virtual ~cDbgArrow();

		bool Create(cRenderer &renderer, const Vector3 &p0, const Vector3 &p1
			, const float size = 1.f, const bool isSolid=false);
		void Render(cRenderer &renderer, const XMMATRIX &tm = XMIdentity);
		void SetDirection(const Vector3 &p0, const Vector3 &p1, const float size = 1.f);
		bool Picking(const Ray &ray, const XMMATRIX &parentTm = XMIdentity);


	public:
		bool m_isSolid; // default: false
		cPyramid m_head;
		cLine m_body;
	};

}
