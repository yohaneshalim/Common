
#include "stdafx.h"
#include "grid.h"

using namespace graphic;


cGrid::cGrid() 
	: cNode2(common::GenerateId(), "grid", eNodeType::MODEL)
{
}

cGrid::~cGrid()
{
}


void cGrid::Create(cRenderer &renderer, const int rowCellCount, const int colCellCount, const float cellSize)
{
	// init member
	m_rowCellCount = rowCellCount;
	m_colCellCount = colCellCount;
	m_cellSize = cellSize;

	// Init Grid
	const int rowVtxCnt  = rowCellCount+1;
	const int colVtxCnt  = colCellCount+1;
	const int cellCnt = rowCellCount * colCellCount;
	const int vtxCount = rowVtxCnt * colVtxCnt;

	sVertexDiffuse *vertices = new sVertexDiffuse[vtxCount];
	{
		const float startx = -cellSize*(colCellCount / 2);
		const float starty = cellSize*(rowCellCount / 2);
		const float endx = startx + cellSize*colCellCount;
		const float endy = starty - cellSize*rowCellCount;

		const float uCoordIncrementSize = 1.0f / (float)colCellCount;
		const float vCoordIncrementSize = 1.0f / (float)rowCellCount;

		int i = 0;
		for (float y = starty; y >= endy; y -= cellSize, ++i)
		{
			int k = 0;
			for (float x = startx; x <= endx; x += cellSize, ++k)
			{
				int index = (i * colVtxCnt) + k;
				vertices[ index].p = XMFLOAT3(x, 0, y);
				vertices[ index].c = XMFLOAT4(1,1,1,1);
			}
		}
	}

	m_vtxBuff.Create(renderer, vtxCount, sizeof(sVertexDiffuse), vertices);
	delete[] vertices;

	WORD *indices = new WORD[cellCnt * 2 * 3];
	{
		int baseIndex = 0;
		for (int i = 0; i < rowCellCount; ++i)
		{
			for (int k = 0; k < colCellCount; ++k)
			{
				indices[baseIndex] = (i * colVtxCnt) + k;
				indices[baseIndex + 1] = (i   * colVtxCnt) + k + 1;
				indices[baseIndex + 2] = ((i + 1) * colVtxCnt) + k;

				indices[baseIndex + 3] = ((i + 1) * colVtxCnt) + k;
				indices[baseIndex + 4] = (i   * colVtxCnt) + k + 1;
				indices[baseIndex + 5] = ((i + 1) * colVtxCnt) + k + 1;

				// next quad
				baseIndex += 6;
			}
		}
	}

	m_idxBuff.Create(renderer, cellCnt * 2, indices);
	delete[] indices;
}


void cGrid::Render(cRenderer &renderer)
{
	m_vtxBuff.Bind(renderer);
	m_idxBuff.Bind(renderer);

	renderer.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	renderer.GetDeviceContext()->DrawIndexed(m_idxBuff.GetFaceCount()*3, 0, 0);

	//renderer.GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);

	//m_vtxBuff.Bind(renderer);
	//m_idxBuff.Bind(renderer);
	//renderer.GetDevice()->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_vtxBuff.GetVertexCount(),
	//	0, m_idxBuff.GetFaceCount());
}


void cGrid::RenderLinelist(cRenderer &renderer)
{
	m_vtxBuff.Bind(renderer);
	m_idxBuff.Bind(renderer);

	renderer.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	renderer.GetDeviceContext()->DrawIndexed(m_idxBuff.GetFaceCount() * 3, 0, 0);

	//renderer.GetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	//renderer.GetDevice()->SetTexture(0, NULL);

	//m_vtxBuff.Bind(renderer);
	//m_idxBuff.Bind(renderer);
	//renderer.GetDevice()->DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, m_vtxBuff.GetVertexCount(),
	//	0, m_idxBuff.GetFaceCount()*3/2);
}