// ������ Ŭ����.
// �ذ� å ������ ��ƼŬ�� ������.
#pragma once


namespace graphic
{

	// ������ ���� ��.
	namespace BILLBOARD_TYPE {
		enum TYPE
		{
			NONE,
			Y_AXIS,		// Y Axis Rotation
			ALL_AXIS,	// X-Y-Z Axis Rotation
			ALL_AXIS_Z, // X-Y-Z Axis Rotation and Z Axis is Up
		};
	};


	class cBillboard : public cQuad
	{
	public:		
		cBillboard();
		virtual ~cBillboard();

		bool Create(cRenderer &renderer, const BILLBOARD_TYPE::TYPE type, const float width, const float height,
			const Vector3 &pos, const string &textureFileName = "", const bool isSizePow2=true);
		void Render(cRenderer &renderer);
		void RenderFactor(cRenderer &renderer);
		void Rotate();


	private:
		BILLBOARD_TYPE::TYPE m_type;
	};

}
