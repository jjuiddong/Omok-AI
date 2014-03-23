
#include "stdafx.h"
#include "ai.h"
#include "separator.h"


namespace ai
{
	struct SearchResult {
		SearchResult(Pos &pos0, int score0, int depth0) : pos(pos0), score(score0), depth(depth0) {}
		Pos pos;
		int score;
		int depth;
	};
	SearchResult RecursiveSearch( STable &table, const PIECE pieceType, const PIECE originalPieceType, const int count );

	enum GETLINETYPE_OPTION { SEARCH_CANDIDATE, SEARCH_DEFENSE };
	bool GetCandidateLocation( STable &table, const PIECE piece, const GETLINETYPE_OPTION option,
		OUT vector<SCandidate> &candidates, OUT map<Pos,SSearchInfo> &info );

	linetype GetLineType( STable &table, const CHECK_TYPE chkType, const PIECE piece, const Pos &pos, const GETLINETYPE_OPTION option,
		INOUT map<Pos,SSearchInfo> &info, OUT vector<SCandidate> &candidates );

	bool LineScanning(STable &table, const CHECK_TYPE chkType, const PIECE piece, const Pos &pos, 
		INOUT map<Pos,SSearchInfo> &info, OUT Pos &startPos, OUT string &lineStr);

	void FilteringLineType( const GETLINETYPE_OPTION option, const int &pieceCnt, const int &emptyCnt, INOUT int &removeFirstCount, INOUT string &lineStr);

	void SearchCombination( INOUT vector<SCandidate> &candidates );
	void MergeCandidate(const vector<SCandidate> &curCandidates, const vector<SCandidate> &oppositeCandidates, OUT vector<SCandidate> &out);
	int CalculateCandidateScore(const vector<SCandidate> &curCandidates, const vector<SCandidate> &oppositeCandidates);
	Pos RandLocation(STable &table, const PIECE piece);
	//bool CompareLines(const vector<PosInfo> &line, const string &pieces);
	Pos GetOffset(const CHECK_TYPE chkType);

	struct SResultInfo 
	{
		SResultInfo(	int result0, int depth0, Pos pos0, linetype ltype0,PIECE pieceType0) : 
			result(result0), depth(depth0), pos(pos0), ltype(ltype0), pieceType(pieceType0) {}
		bool operator<(const SResultInfo &rhs) {
			if (result == rhs.result) {
				if (depth == rhs.depth)
					return CompareLineType(ltype, rhs.ltype);
				else
					return depth < rhs.depth;
			}
			return result < rhs.result;
		}
		int result;
		int depth;
		Pos pos;
		linetype ltype;
		PIECE pieceType;
	};

	struct SMergeInfo 
	{
		SMergeInfo(Pos pos0, linetype ltype0, int score0) : pos(pos0), ltype(ltype0), score(score0) {}
		bool operator<(const SMergeInfo &rhs) {
			return score < rhs.score;
		}
		Pos pos;
		linetype ltype;
		int score;
	};

	int g_totalCnt=0;
}

using namespace ai;


/**
 @brief 오목 인공지능 시작.
			pieceType이 놓을 가장 최적의 위치를 리턴한다.
 @date 2014-03-18
*/
GAME_STATE ai::SearchBestLocation( STable &table, const PIECE pieceType, OUT Pos &out )
{
	g_totalCnt = 0;
	SearchResult result = RecursiveSearch(table, pieceType, pieceType, 3);

	out = result.pos;
	return GAME;
}


/**
 @brief 재귀적으로 탐색하면 가장 좋은 수를 찾는다.
			 count 가 n 번이 될 때까지 트리를 만들고, 그 상태에서 peiceType의 상황을
			 점수로 표시해 리턴한다.

			 점수가 0 점인경우 랜덤으로 위치를 정할 때
			 양수인 경우 pieceType이 유리 한 상황
			 음수인 경우 pieceType의 상대가 유리한 상황 

			 리턴값 pair<위치, 점수>
 @date 2014-03-18
*/
SearchResult ai::RecursiveSearch( STable &table, const PIECE pieceType, const PIECE originalPieceType, const int count )
{
	stringstream ss;
	ss << "RecursiveSearch " << ++g_totalCnt << " curCnt " << count << std::endl;
	OutputDebugStringA( ss.str().c_str() );

	map<Pos,SSearchInfo> searchInfo;
	vector<SCandidate> curCandidate, oppositeCandidate;
	const bool isOverOpposite = GetCandidateLocation(table, OppositePiece(pieceType), SEARCH_DEFENSE, oppositeCandidate, searchInfo);
	const bool isOverCurrent = GetCandidateLocation(table, pieceType, SEARCH_CANDIDATE, curCandidate, searchInfo);

	if (isOverOpposite || isOverCurrent)
	{
		return (isOverCurrent)? SearchResult(Pos(-1,-1),-100,100-count) : SearchResult(Pos(-1,-1),100,100-count);
	}

	if ((count <= 0) && (pieceType == originalPieceType))// max search depth
	{
		const int score = CalculateCandidateScore(curCandidate, oppositeCandidate);
		return SearchResult(Pos(-1,-1), -score, 100-count);
	}

	vector<SCandidate> candidates;
	MergeCandidate(curCandidate, oppositeCandidate, candidates);

	if (candidates.empty())
	{
		Pos pos = RandLocation(table, pieceType);
		candidates.push_back(SCandidate(pos, 0));
	}
	
	int procCnt = 0;
	int maxCnt = 4;
	vector<SResultInfo> results;
	results.reserve(10);
	BOOST_REVERSE_FOREACH (auto &cand, candidates)
	{
		if (++procCnt > maxCnt)
			break;

		STable newTable = table;
		SetPiece(newTable, cand.pos, pieceType);
		const SearchResult r = RecursiveSearch(newTable, OppositePiece(pieceType), originalPieceType, count-1);
		results.push_back( SResultInfo(r.score, r.depth, cand.pos, cand.ltype, pieceType) );
	}

	if (results.empty())
		return SearchResult(Pos(-1,-1), -100, count);

	// 가장 높은 점수를 찾아 리턴한다.
	std::sort(results.begin(), results.end());

	return SearchResult(results.back().pos, -results.back().result, results.back().depth); // 상대편 루틴으로 넘어가면서 result값은 부호가 바뀐다.
}


/**
 @brief check range
 @date 2014-03-15
*/
bool ai::CheckEmpty(STable &table, const Pos &pos)
{
	if (!CheckRange(pos))
		return false;
	if (table.pieces[ pos.x][ pos.y] != EMPTY)
		return false;
	return true;
}


/**
 @brief 테이블 범위를 벗어나면 false를 리턴한다.
 @date 2014-03-17
*/
bool ai::CheckRange(const Pos &pos)
{
	if (MAX_TABLE_X <= pos.x)
		return false;
	if (0 > pos.x)
		return false;
	if (MAX_TABLE_Y <= pos.y)
		return false;
	if (0 > pos.y)
		return false;
	return true;
}


/**
 @brief 
 @date 2014-03-15
*/
bool ai::SetPiece(STable &table, const Pos &pos, PIECE piece)
{
	//if (GAME != m_state)
	//	return false;
	if (piece == EMPTY)
	{
		if (!CheckRange(pos))
			return false;
	}
	else
	{
		if (!CheckEmpty(table, pos))
			return false;
	}

	table.pieces[ pos.x][ pos.y] = piece;

	switch (piece)
	{
	case BLACK: table.blacks.push_back(pos); break;
	case WHITE: table.whites.push_back(pos); break;
	}
	return true;
}


/**
 @brief 
 @date 2014-03-15
*/
PIECE ai::GetPiece(STable &table, const Pos &pos)
{
	if (CheckRange(pos))
		return table.pieces[ pos.x][ pos.y];
	return WALL;
}


/**
 @brief Get Candidate Location
			가장 좋은 수는 candidates 의 back() 에 있다.
			높은점수 순서대로 오름차순으로 정렬된다.
 @date 2014-03-15
*/
bool ai::GetCandidateLocation( STable &table, const PIECE piece,  const GETLINETYPE_OPTION option, 
	OUT vector<SCandidate> &candidates, OUT map<Pos,SSearchInfo> &info )
{
	if ((piece != WHITE) && (piece != BLACK)) return false;

	CHECK_TYPE chktypes[4] = {CHECK_ROW, CHECK_COL, CHECK_SLASH_LEFT, CHECK_SLASH_RIGHT};

	const vector<Pos> &pieces = (piece == WHITE)? table.whites : table.blacks;

	vector<SCandidate> cand;
	cand.reserve(pieces.size());

	BOOST_FOREACH (auto &pos, pieces)
 	{
		for (int c=0; c < 4; ++c)
		{
			vector<SCandidate> out;
			GetLineType(table, chktypes[ c], piece, pos, option, info, out);

			if (!out.empty())
			{
				BOOST_FOREACH (auto &s, out)
				{
					if ((GetPieceCntFromlinetype(s.ltype) == 5) && (GetEmptyCntFromlinetype(s.ltype) == 0))
						return true; // 오목, 게임오버.

					if (s.ltype != 0)
						cand.push_back(s);//SCandidate(p0, ltype));
				}
			}
		}
	}

	SearchCombination(cand);

	//----------------------------------------------------------------------------------
	// candidate 정렬.

	const int MAX_CANDIDATE_COUNT = 5;
	// 가장 높은 우선순위의 Location 을 MAX_CANDIDATE_COUNT 개만 찾아 리턴한다.
	int count = 0;
	while ((count < MAX_CANDIDATE_COUNT) && (count < (int)cand.size()))
	{
		int maxIdx = 0;
		for (unsigned int i=0; i < cand.size(); ++i)
		{
			if (i == maxIdx)
				continue;
			if (cand[ i].ltype == 0)
				continue;

			if (CompareLineType(cand[ maxIdx].ltype, cand[ i].ltype))  // cand[ maxIdx].ltype < cand[ i].ltype
				maxIdx = (int)i;
		}
		// find!!
		if (maxIdx >= 0)
		{
			++count;
			candidates.push_back( SCandidate(cand[ maxIdx].pos, cand[ maxIdx].ltype) );
			cand[ maxIdx].ltype = 0;// initialize
		}
	}

	//std::sort(candidates.begin(), candidates.end());
	std::reverse(candidates.begin(), candidates.end());
	//----------------------------------------------------------------------------------


	return false;
}


/**
 @brief 연속된 알의 갯수를 결정한다. 2, 3, 4 
             빈칸을 포함한 조합도 탐색한다.
 @date 2014-03-15
*/
linetype ai::GetLineType( STable &table, const CHECK_TYPE chkType, const PIECE pieceType, const Pos &pos, 
	const GETLINETYPE_OPTION option, OUT map<Pos,SSearchInfo> &info, OUT vector<SCandidate> &candidates )
{
	string lineStr;
	Pos startPos;
	if (!LineScanning(table, chkType, pieceType, pos, info, startPos, lineStr))
		return 0;

	vector<string> strs;
	separator::SeparateAll(lineStr, strs);

	Pos linePos = startPos;
	const Pos offset = GetOffset(chkType);
	BOOST_FOREACH(const auto &str, strs)
	{
		string line = str;
		int emptyCnt=0, firstCnt=0, lastCnt=0;
		int pieceCnt = separator::GetPieceInfo(line, emptyCnt, firstCnt, lastCnt);
		Pos p0 = linePos;
		linePos += offset * line.length(); // next line pos
		int rmFirstCnt = 0;
		FilteringLineType(option, pieceCnt, emptyCnt, rmFirstCnt, line);
		p0 += offset * rmFirstCnt;

		emptyCnt = 0; firstCnt = 0; lastCnt = 0;
		pieceCnt = separator::GetPieceInfo(line, emptyCnt, firstCnt, lastCnt);
		const linetype ltype = ::GetLineType(pieceCnt, emptyCnt, firstCnt, lastCnt);
		if ((pieceCnt == 5) && (emptyCnt==0))
		{
			candidates.push_back(SCandidate(p0, ltype));
			return 0;
		}

		// 최종 후보지를 저장한다.
		for (unsigned int i=0; i < line.length(); ++i)
		{
			if ('0' == line[i]) // empty location 이 후보지가 된다.
				candidates.push_back(SCandidate(p0+(offset*i), ltype));
		}
	}

	return 0;
}


/**
 @brief 
 @date 2014-03-21
*/
Pos ai::GetOffset(const CHECK_TYPE chkType)
{
	Pos offset(0,0);
	switch (chkType)
	{
	case CHECK_ROW: offset = Pos(1,0); break;
	case CHECK_COL: offset = Pos(0,1); break;
	case CHECK_SLASH_LEFT: offset = Pos(1,1); break;
	case CHECK_SLASH_RIGHT: offset = Pos(1,-1); break;
	}
	return offset;
}


/**
 @brief 한 라인을 스캔해서 라인 정보를 리턴한다.
 @date 2014-03-21
*/
bool ai::LineScanning(STable &table, const CHECK_TYPE chkType, const PIECE pieceType, const Pos &pos, 
	INOUT map<Pos,SSearchInfo> &info, OUT Pos &startPos, OUT string &lineStr)
{
	const int ALLOW_MAX_EMPTYCNT = 3;

	Pos offset = GetOffset(chkType);

	//Pos startPos(-1,-1);
	startPos = Pos(-1,-1);
	int pieceCnt = 0;
	//wallCnt = 0;
	int loopCnt = 0; // loopCnt=0 : 시작점 찾기,  loopCnt=1 : 스캐닝 시작.
	while (loopCnt < 2)
	{
		int emptyContinue = 0;
		Pos p0 = (startPos==Pos(-1,-1))? pos : startPos;

		while (1)
		{
			const PIECE piece = GetPiece(table, p0);
			if ((piece == WALL) || (piece == OppositePiece(pieceType)))
			{
				//++wallCnt;
				//if (emptyContinue > 0)
				//	--wallCnt;

				if (0 == loopCnt) // 시작점 발견, loopCnt=1 에서 본격적으로 스캐닝한다.
				{
					startPos = p0 + offset;
					if (emptyContinue > 0)
					{
						int cnt = 0;
						Pos tempP = p0+offset;
						while (cnt < emptyContinue) {
							//line.push_back(PosInfo(tempP,0));
							lineStr += '0';
							tempP += offset;
							startPos += offset;
							++cnt;
						}
					}
				}
				break;
			}

			auto it = info.find(p0);
			if (info.end() != it)
			{
				if (chkType & it->second.check)
					return false; // already check
			}

			if (piece == EMPTY)
			{
				++emptyContinue;
				if (emptyContinue >= ALLOW_MAX_EMPTYCNT)
				{
					if (0 == loopCnt)
					{
						startPos = p0;// - offset;// - offset;

						int cnt = 0;
						Pos tempP = p0;
						while (cnt < emptyContinue) {
							//line.push_back(PosInfo(tempP,0));
							lineStr += '0';
							tempP += offset;
							startPos += offset;
							++cnt;
						}

						//line.push_back(PosInfo(p0,0));
						//line.push_back(PosInfo(p0-offset,0));
					}
					else
					{
						//line.push_back(PosInfo(p0,0));
						lineStr += '0';
					}

					break;
				}
				else
				{
					if (1 == loopCnt) 
					{
						//line.push_back(PosInfo(p0,0));
						lineStr += '0';
					}
				}
			}
			else
			{
				emptyContinue = 0;

				if (1 == loopCnt)
				{
					//line.push_back(PosInfo(p0,1));
					lineStr += '1';
					++pieceCnt;
					info[ p0].check |= chkType;
				}
			}

			p0 += ((loopCnt==0)? -offset : offset);
		} // while

		++loopCnt;
	} // while


	// 0001000 -> 00100
	// 00011000 -> 001100
	// 0001101000 -> 00110100
	if (lineStr.length() > 3)
	{
		if ((lineStr[ 0] == '0') && (lineStr[ 1] == '0') && (lineStr[ 2] == '0'))
			lineStr.erase(0,1); // pop_front
		const int len = lineStr.length();
		if ((lineStr[ len-1] == '0') && (lineStr[ len-2] == '0') && (lineStr[ len-3] == '0'))
			lineStr.pop_back();
	}

	// adjust start poisition
	BOOST_FOREACH (auto &s, lineStr)
	{
		if (s == '1')
			break;
		startPos -= offset;
	}

	return true;
}


/**
 @brief pieces 배열을 필요에 따라 나누거나 제거한다.
 @param pieces : 나열된 오목 배열 
							0:빈자리 
							1:돌이 있는자리.
 @date 2014-03-21
*/
void ai::FilteringLineType( const GETLINETYPE_OPTION option, const int &pieceCnt, const int &emptyCnt, 
	INOUT int &removeFirstCount, INOUT string &lineStr )
{
	if (lineStr.length() < 3)
		return;

	bool removeSide = false;
	// 001101100 -> 11011
	// 0101110 -> 10111 ... etc
	if (((4 == pieceCnt) && (1 == emptyCnt)) ||
		((3 == pieceCnt) && (1 == emptyCnt)) || // 010110 -> S1011S ... 
		((3 == pieceCnt) && (2 == emptyCnt)) ) // 0101010 -> S10101S
	{
		removeSide = true;
	}

	bool removeSide2Empty = false;
	// 3개이상 연속될 경우 양쪽 옆 empty는 1개로만 제한한다.
	if (3 <= pieceCnt)
	{
		removeSide2Empty = true;
	}

	// 수비옵션일 때는 양쪽의 empty구간을 1로 제한한다.
	// 연속된 peice가 4개일때도 마찬가지.
	// 00111100 -> S011110S
	if (removeSide2Empty ||
		(SEARCH_DEFENSE == option))
	{
		if ((lineStr[ 0] == '0') && (lineStr[ 1] == '0'))
			lineStr.front() = 'S';
		const int len = lineStr.length();
		if ((lineStr[ len-1] == '0') && (lineStr[ len-2] == '0'))
			lineStr.back() = 'S';
	}

	// 00~~00 -> SS~~SS
	if (removeSide)
	{
		if (lineStr[ 0] == '0')
			lineStr[ 0] = 'S';
		if (lineStr[ 1] == '0')
			lineStr[ 1] = 'S';
		const int len = lineStr.length();
		if (lineStr[ len-1] == '0')
			lineStr[ len-1] = 'S';
		if (lineStr[ len-2] == '0')
			lineStr[ len-2] = 'S';
	}
	
}


/**
 @brief 3-3, 3-4, 4-4 조합을 찾아서 합친다.
 @date 2014-03-16
*/
void ai::SearchCombination( INOUT vector<SCandidate> &candidates )
{
	map<Pos, vector<SCandidate> > combs;

	BOOST_FOREACH (auto cand, candidates)
	{
		combs[ cand.pos].push_back(cand);
	}

	// sorting
	BOOST_FOREACH (auto &kv, combs)
		std::sort(kv.second.begin(), kv.second.end());

	BOOST_FOREACH (auto &kv, combs)
	{
		if (2 > GetPieceCntFromlinetype(kv.second.back().ltype))
		{
			// 연속갯수가 2개이상만 검사에 포함시킨다.
			kv.second.clear();
			continue;
		}

		if (kv.second.size() >= 2)
		{
			const linetype ltype = MergeLineType(kv.second[ kv.second.size()-1].ltype, kv.second[ kv.second.size()-2].ltype);
			kv.second.clear();
			kv.second.push_back(SCandidate(kv.first, ltype));
		}
	}

	candidates.clear();
	candidates.reserve(combs.size());

	BOOST_FOREACH(auto kv, combs)
	{
		if (!kv.second.empty())
			candidates.push_back( kv.second.back() );
	}
}


/**
 @brief candidate0-1 조합을 합쳐서 우선순위가 높은 점수로 오름차순으로 정렬한다.
 @date 2014-03-18
*/
void ai::MergeCandidate(const vector<SCandidate> &cur, const vector<SCandidate> &oppoisite, 
	OUT vector<SCandidate> &out)
{
	if (cur.empty() && oppoisite.empty())
		return;

	const int MAX_PROC_COUNT = 5;
	vector<SMergeInfo> m;
	m.reserve(cur.size() + oppoisite.size());
	
	for (int i=cur.size()-1; ((cur.size()-i) < MAX_PROC_COUNT) && (i >= 0); --i)
		m.push_back( SMergeInfo(cur[ i].pos, cur[ i].ltype, GetLinetypeScore(cur[ i].ltype)+5) );

	for (int i=oppoisite.size()-1; ((oppoisite.size()-i) < MAX_PROC_COUNT) && (i >= 0); --i)
		m.push_back( SMergeInfo(oppoisite[ i].pos, oppoisite[ i].ltype, GetLinetypeScore(oppoisite[ i].ltype)) );
	
	sort(m.begin(), m.end());

	BOOST_FOREACH (auto &info, m)
		out.push_back( SCandidate(info.pos, info.ltype) );
}


/**
 @brief 현재 상태를 기준으로 점수를 구한다. 연속된 돌 갯수와 wall 갯수로 결정짓는다.
            자신과 상대의 점수의 차를 리턴한다.
			자신이 유리하면 양수, 불리하면 음수를 리턴하게 된다.
 @date 2014-03-18
*/
int ai::CalculateCandidateScore(const vector<SCandidate> &cur, const vector<SCandidate> &opposite)
{
	int curScore = (cur.empty())? 0 : GetLinetypeScore(cur.back().ltype);
	int oppositeScore = (opposite.empty())? 0 : GetLinetypeScore(opposite.back().ltype);

	// 현재 수를 둘 사람에게 +5 점을 추가한다.
	return curScore+5 - oppositeScore;
}


/**
 @brief 오목이 완성되면 true를 리턴한다.
 @date 2014-03-17
*/
bool ai::IsGameComplete(STable &table, const PIECE pieceType)
{
	map<Pos,SSearchInfo> info;
	vector<SCandidate> candidates;
	if (GetCandidateLocation(table, pieceType, SEARCH_CANDIDATE, candidates, info))
		return true;

	return false;
}


/**
 @brief search random  location
 @date 2014-03-17
*/
Pos ai::RandLocation(STable &table, const PIECE piece)
{
	vector<Pos> &pieces = (OppositePiece(piece)==BLACK)? table.blacks : table.whites;

	int x = MAX_TABLE_X/2;
	int y = MAX_TABLE_Y/2;
	if (!pieces.empty())
	{
		x = pieces[0].x;
		y = pieces[0].y;
	}

	while (table.pieces[ x][ y] != EMPTY)
	{
		++x;
	}

	return Pos(x,y);
}

//
///**
// @brief 나열된 돌의 순서가 같다면 true를 리턴한다.
//		    pieces => 010101 , 11011, 1?1 -> 101, 111 모두 해당
// @date 2014-03-19
//*/
//bool ai::CompareLines(const vector<PosInfo> &line, const string &pieces)
//{
//	if (line.size() != pieces.length())
//		return false;
//
//	for (unsigned int i=0; i < line.size(); ++i)
//	{
//		if (pieces[ i] == '?')
//			continue;
//		if ((pieces[ i] == '0') && (line[ i].piece == 0))
//			continue;
//		if ((pieces[ i] == '1') && (line[ i].piece == 1))
//			continue;
//		return false;
//	}
//	return true;
//}
// 