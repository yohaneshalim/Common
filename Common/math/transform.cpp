
#include "stdafx.h"
#include "transform.h"


using namespace common;


const Transform Transform::Identity;


Transform::Transform()
: pos(0,0,0)
, scale(1,1,1)
, rot(0,0,0,1)
{

}


// Scale * Rotation * Traslation
Matrix44 Transform::GetMatrix() const
{
	Matrix44 S;
	S.SetScale(scale);

	Matrix44 tm;
	tm = S * rot.GetMatrix();

	tm._41 = pos.x;
	tm._42 = pos.y;
	tm._43 = pos.z;

	return tm;
}


#ifdef USE_D3D11_MATH

	XMMATRIX Transform::GetMatrixXM() const
	{
		XMVECTOR q = XMLoadFloat4((XMFLOAT4*)&rot);
		XMMATRIX mRot = XMMatrixRotationQuaternion(q);
		return XMMatrixScaling(scale.x, scale.y, scale.z) * mRot * XMMatrixTranslation(pos.x, pos.y, pos.z);
	}

#endif


Transform Transform::Inverse() const
{
	Transform v;
	v.pos = -pos;
	v.scale = Vector3(1/scale.x, 1/scale.y, 1/scale.z);
	v.rot = rot.Inverse();
	return v;
}


Transform Transform::operator*(const Transform &rhs) const
{
	Transform v;
	v.pos = pos + rhs.pos;
	v.scale = Vector3(scale.x * rhs.scale.x
		, scale.y * rhs.scale.y
		, scale.z * rhs.scale.z);
	v.rot = rot * rhs.rot;
	return v;
}


const Transform& Transform::operator*=(const Transform &rhs)
{
	*this = operator*(rhs);
	return *this;
}
