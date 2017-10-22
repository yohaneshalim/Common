
#include "stdafx.h"
#include "line.h"

using namespace graphic;


cLine::cLine() 
	: cNode(common::GenerateId(), "line", eNodeType::MODEL)
{
}

cLine::cLine(cRenderer &renderer
	, const Vector3 &p0 //= Vector3(0, 0, 0)
	, const Vector3 &p1 //= Vector3(1, 1, 1)
	, const float width //= 1.f
	, const int vtxType //= (eVertexType::POSITION | eVertexType::NORMAL)
	, const cColor color //= cColor::BLACK
)
	: cNode(common::GenerateId(), "line", eNodeType::MODEL)
{
	m_shape.Create2(renderer, vtxType, color);
	SetLine(p0, p1, width);
	m_color = color;
}


bool cLine::Create(cRenderer &renderer
	, const Vector3 &p0 //= Vector3(0, 0, 0)
	, const Vector3 &p1 //= Vector3(1, 1, 1)
	, const float width //= 1.f
	, const int vtxType //=(eVertexType::POSITION | eVertexType::NORMAL)
	, const cColor color //= cColor::BLACK
)
{
	m_shape.Create2(renderer, vtxType, color);
	SetLine(p0, p1, width);
	m_color = color;
	return true;
}


bool cLine::Render(cRenderer &renderer
	, const XMMATRIX &parentTm //= XMIdentity
	, const int flags //= 1
)
{
	cShader11 *shader = (m_shader) ? m_shader : renderer.m_shaderMgr.FindShader(m_shape.m_vtxType);
	assert(shader);
	shader->SetTechnique("Unlit");
	shader->Begin();
	shader->BeginPass(renderer, 0);

	renderer.m_cbPerFrame.m_v->mWorld = XMMatrixTranspose(m_boundingBox.GetTransformXM() * parentTm);
	renderer.m_cbPerFrame.Update(renderer);
	renderer.m_cbLight.Update(renderer, 1);

	const Vector4 color = m_color.GetColor();
	renderer.m_cbMaterial.m_v->diffuse = XMVectorSet(color.x, color.y, color.z, color.w);
	renderer.m_cbMaterial.Update(renderer, 2);

	m_shape.Render(renderer);
	return true;
}


void cLine::SetLine(const Vector3 &p0, const Vector3 &p1, const float width)
{
	m_boundingBox.SetLineBoundingBox(p0, p1, width);
}


void cLine::SetColor(const cColor &color)
{
	assert(0);
}

