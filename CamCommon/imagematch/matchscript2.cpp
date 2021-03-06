#include "stdafx.h"
#include "matchscript2.h"
#include "matchprocessor.h"

using namespace cv;
using namespace cvproc;
using namespace cvproc::imagematch;
using namespace std;


cMatchScript2::cMatchScript2()
 	: m_matchType(0)
 	, m_treeId(0)
	, m_cloneLinkId(0)
{
}

cMatchScript2::~cMatchScript2()
{
	Clear();
}


void cMatchScript2::build(sParseTree *parent, sParseTree *prev, sParseTree *current)
{
	RET(!current);

// 	vector<string> attribs;
// 	const bool isSymbolCode = buildAttributes(current, current->line, attribs) > 0;

	// check tree label node link
	// only link node,  paren==null is head node
	if ((current->type == 0) && parent)
	{
		if (const sParseTree *linkNode = FindTreeLabel(current->attrs["id"])) // 링크노드 일경우.
		{
			// 링크노드의 클론을 만들어 추가한다.
			//sParseTree *node = new sParseTree;
			//memcpy(node, linkNode, sizeof(sParseTree));
			sParseTree *node = linkNode->clone(false);
			node->next = current->next;
			SAFE_DELETE(current);
			if (prev)
				prev->next = node;
			else
				parent->child = node;

			build(parent, node, node->next);
			return;
		}
	}

	//setTreeAttribute(current, attribs);
	current->id = m_treeId++;

	build(current, NULL, current->child);
	build(parent, current, current->next);
}


// tok1, tok2, tok3, tok4, tok5 ...  분리 
// node->str = tok1
// tok2~5 : roi x,y,w,h
// { tok = tok }
// return value : 0 = none
//						   1,2,3... = add symbol table, symbolCode
int cMatchScript2::buildAttributes(const sParseTree *node, const string &str, vector<string> &attributes)
{
	int reval = 0;

 	m_parser.m_lineStr = (char*)str.c_str();
	while (1)
	{
		const char *pid = m_parser.id();
		const char *pnum = (!*pid) ? m_parser.number() : NULL;

		if (*pid)
		{
			const string symb = m_parser.GetSymbol(pid);
			const int symbType = m_parser.GetSymbolType(pid);
			if (symbType == 0) // string type
			{
				if (symb.empty())
				{
					attributes.push_back(pid);
				}
				else
				{
					char *old = m_parser.m_lineStr;
					reval += buildAttributes(node, symb.c_str(), attributes);
					m_parser.m_lineStr = old;
				}
			}
			else if (symbType == 1)
			{
				// string table type,
				// setTreeAttribute() 에서 처리한다.
				attributes.push_back(pid);
			}
		}
		else if (*pnum)
		{
			attributes.push_back(pnum);
		}
		else if (m_parser.m_lineStr[0] == '{')
		{
			// parsing, { id = id, id = id } 
			if (m_parser.assigned_list(node))
			{
				// 심볼테이블 트리 추가
				reval += 1;
			}
		}
		else
		{
			dbg::ErrLog("line {%d} error!! tree attribute fail [%s]\n", node->lineNum, str.c_str());
			m_parser.m_isError = true;
			break;
		}

		// comma check
		m_parser.m_lineStr = m_parser.passBlank(m_parser.m_lineStr);
		if (!m_parser.m_lineStr)
			break;
		if (*m_parser.m_lineStr == ',')
			++m_parser.m_lineStr;
	}

	return reval;
}


// threshold_0.9 
// templatematch, featurematch
void cMatchScript2::setTreeAttribute(sParseTree *node, vector<string> &attribs)
{
	node->id = m_treeId++;

	if (attribs.empty())
		return;

	vector<int> rmIdices; // remove attribute index array
	int hsvCnt = 0;
	int hlsCnt = 0;

	// check match type
//	node->matchType = -1; // defautl value is -1

	// check threshold
// 	for (int i = 0; i < (int)attribs.size(); ++i)
// 	{
// 		const int pos1 = attribs[i].find("featurematch");
// 		const int pos2 = attribs[i].find("templatematch");
// 		const int pos3 = attribs[i].find("ocrmatch");
// 		if ((string::npos != pos1) || (string::npos != pos2) || (string::npos != pos3))
// 		{
// 			if (string::npos != pos1)
// 				node->matchType = 1;
// 			if (string::npos != pos2)
// 				node->matchType = 0;
// 			if (string::npos != pos3)
// 				node->matchType = 2;
// 			rmIdices.push_back(i);
// 		}
// 		else if (string::npos != attribs[i].find("threshold_"))
// 		{
// 			// threshold 값 설정
// 			const int valPos = attribs[i].find("_");
// 			const float threshold = (float)atof(attribs[i].substr(valPos + 1).c_str());
// 			node->threshold = threshold;
// 			rmIdices.push_back(i);
// 		}
// 		else if (string::npos != attribs[i].find("scalar_"))
// 		{
// 			// scalar_255_0_255_1.5
// 			const int valPos = attribs[i].find("_");
// 			sscanf(attribs[i].c_str(), "scalar_%d_%d_%d_%f", &node->scalar[0], &node->scalar[1], &node->scalar[2], &node->scale);
// 			rmIdices.push_back(i);
// 		}
// 		else if (string::npos != attribs[i].find("hsv_"))
// 		{
// 			// hsv_40_40_255_120_255_255
// 			if (hsvCnt < 3)
// 			{
// 				sscanf(attribs[i].c_str(), "hsv_%d_%d_%d_%d_%d_%d", &node->hsv[hsvCnt][0], &node->hsv[hsvCnt][1], &node->hsv[hsvCnt][2],
// 					&node->hsv[hsvCnt][3], &node->hsv[hsvCnt][4], &node->hsv[hsvCnt][5]);
// 			}
// 			++hsvCnt;
// 			rmIdices.push_back(i);
// 		}
// 		else if (string::npos != attribs[i].find("hls_"))
// 		{
// 			// hls_40_40_255_120_255_255
// 			if (hlsCnt < 3)
// 			{
// 				sscanf(attribs[i].c_str(), "hls_%d_%d_%d_%d_%d_%d", &node->hls[hlsCnt][0], &node->hls[hlsCnt][1], &node->hls[hlsCnt][2],
// 					&node->hls[hlsCnt][3], &node->hls[hlsCnt][4], &node->hls[hlsCnt][5]);
// 			}
// 			++hlsCnt;
// 			rmIdices.push_back(i);
// 		}
// 		else if (vector<string> *table = m_parser.GetSymbol2(attribs[i]))
// 		{
// 			// string table 설정
// 			node->table = table;
// 			rmIdices.push_back(i);
// 		}
// 	}
// 
// 	strcpy_s(node->name, attribs[0].c_str());
// 
// 	for (int i = rmIdices.size() - 1; i >= 0; --i)
// 		rotatepopvector(attribs, rmIdices[i]);
// 
// 	if (node->type == 0) // tree
// 	{
// 		if (attribs.size() >= 5)
// 		{
// 			for (int i = 1; i < 5; ++i)
// 				node->roi[i - 1] = atoi(attribs[i].c_str());
// 		}
// 		if (attribs.size() >= 6)
// 		{
// 			if (attribs[5] == "+")
// 				node->isRelation = true;
// 		}
// 	}
// 	else if (node->type == 1) // exec
// 	{
// 		if (attribs.size() >= 2)
// 		{
// 			strcat_s(node->name, " ");
// 			strcat_s(node->name, attribs[1].c_str());
// 		}
// 	}

}


// node 가 매칭에 성공했다면, 성공한 위치(matchLoc)의 이미지를 복사해 리턴한다.
void cMatchScript2::GetCloneMatchingArea(const Mat &input, const string &inputName, const int inputImageId, 
	sParseTree *node, OUT cv::Mat *out)
{
	const Mat &matScene = cMatchProcessor::Get()->loadImage(inputName);
	if (matScene.empty())
		return;

	const Mat &matObj = cMatchProcessor::Get()->loadImage(node->attrs["id"]);
	if (matObj.empty())
		return;

	const cv::Size csize(input.cols - matObj.cols + 1, input.rows - matObj.rows + 1);
	if ((csize.height < 0) || csize.width < 0)
		return;

	const Mat *src = &matScene;

	// channel match
	if (!node->IsEmptyBgr())
	{
		cv::Scalar scalar;
		sscanf(node->attrs["scalar"].c_str(), "%lf,%lf,%lf", &scalar[0], &scalar[1], &scalar[2]);
		const float scale = (float)atof(node->attrs["scale"].c_str());
		src = &cMatchProcessor::Get()->loadScalarImage(inputName, inputImageId, scalar, scale); // BGR
	}

	// hsv match
	if (!node->IsEmptyCvt())
	{
		src = &cMatchProcessor::Get()->loadCvtImageAcc(inputName, inputImageId, node);
	}

	if (!src->data)
		return;// fail

	const bool isFeatureMatch = ((m_matchType == 1) && (node->attrs["type"].empty())) || (node->attrs["type"] == "featurematch");
	if (!isFeatureMatch) // --> templatematch
	{
		Point left_top = node->matchLoc;
		*out = (*src)(Rect(left_top.x, left_top.y, matObj.cols, matObj.rows));
	}
}


// 스크립트의 exec 명령어를 실행한다.
void cMatchScript2::Exec()
{
	sParseTree *node = m_parser.m_execRoot;
	while (node)
	{
		stringstream ss(node->attrs["id"]);
		string label, file;
		ss >> label >> file;
		if (label.empty() || file.empty())
			break;

		Mat img = imread(file.c_str());
		if (!img.data)
			break;

		const int t1 = timeGetTime();
		//ExecuteEx(label, file, img, true);
		const int t2 = timeGetTime();
		//cout << "exec(" << label << ") << " << file << " = " << m_threadArg.resultStr << ", " << t2-t1 << endl;

		node = node->next;
	}
}


const sParseTree* cMatchScript2::FindTreeLabel(const string &label) const
{
	auto it = m_treeLabelTable.find(label);
	return (m_treeLabelTable.end() != it)? it->second : NULL;
}


// 스크립트를 읽고, 파스트리를 생성한다.
bool cMatchScript2::Read(const string &fileName)
{
	Clear();

	if (!m_parser.Read(fileName))
		return false;

	// head node들의 next 링크를 모두 제거한다. 독립적으로 작동하는 트리를 만들기 위해서다.
	vector<sParseTree*> headNodes; // 트리 헤드노드들을 순서대로 처리하기 위해서 만듬
	sParseTree *node = (m_parser.m_treeRoot)? m_parser.m_treeRoot->clone() : NULL; // tree copy
	while (node)
	{
		if ('@' != node->attrs["id"][0])
		{
			MessageBoxA(NULL, "Error!! ImageMatchScript LabelNode Name must start \'@\' ", "Error", MB_OK);
			return false;
		}

		m_treeLabelTable[node->attrs["id"]] = node;
		headNodes.push_back(node);
		sParseTree *next = node->next;
		node->next = NULL; // tree label node에는 자식만 연결된 상태로 등록된다. next는 없다.
		node = next;
	}

	m_treeId = 0;
	m_cloneLinkId = 0;
	build(NULL, NULL, m_parser.m_execRoot);
	for each (auto it in headNodes)
		build(NULL, NULL, it);

  	m_isDebug = atoi(m_parser.GetSymbol("debug").c_str()) ? true : false;
  	m_matchType = atoi(m_parser.GetSymbol("matchtype").c_str());

	// update node table
	m_nodes.clear();
	for each (auto it in m_treeLabelTable)
		m_parser.collectTree(it.second, m_nodes);

	ZeroMemory(m_nodeTable, sizeof(m_nodeTable));
	for each (auto it in m_nodes)
		m_nodeTable[it->id] = it;

	return true;
}


void cMatchScript2::Clear()
{
	m_parser.Clear();

	for each (auto kv in m_nodes)
		delete kv;
	m_nodes.clear();
	m_treeLabelTable.clear();
}


// sParsetree 의 result 값을 초기화 한다.
// 함수가 재귀적으로 호출될 때, 중복 연산을 막기위해 쓰인다.
void cMatchScript2::clearResultTree() 
{
	for each (auto it in m_nodes)
	{
		it->result = -1;
		it->processCnt = 0;
	}
}
