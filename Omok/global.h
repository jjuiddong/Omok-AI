/**
 @filename global.h
 
*/
#pragma once


#include <vector>
#include <set>
#include <map>
#include <string>
#include <list>
#include <algorithm>
#include <sstream>
#include <assert.h>
#include <boost/foreach.hpp>
using namespace  std;

#define INOUT // IN + OUT


enum {
	MAX_TABLE_X = 30,
	MAX_TABLE_Y = 30,
};


enum GAME_STATE 
{ 
	GAME, 
	WIN_WHITE, 
	WIN_BLACK 
};

enum PIECE {
	EMPTY=0,
	WHITE,
	BLACK,
	WALL,
};

typedef int linetype;


/**
 @brief 
 @date 2014-03-15
*/
inline PIECE OppositePiece(const PIECE piece) {
	switch (piece)
	{
	case WHITE: return BLACK;
	case  BLACK: return WHITE;
	}
	return WALL;
}



struct Pos 
{
	Pos(): x(0),y(0) {}
	Pos(int x0, int y0): x(x0),y(y0) {}

	bool operator==(const Pos &rhs) const {
		return (x==rhs.x) && (y == rhs.y);
	}
	bool operator<(const Pos &rhs) const {
		//return (x<rhs.x) && (y<rhs.y);
		return (x*100+y) < (rhs.x*100+rhs.y);
	}
	Pos operator+(const Pos &rhs) const {
		return Pos(x+rhs.x, y+rhs.y);
	}
	Pos operator-(const Pos &rhs) const {
		return Pos(x-rhs.x, y-rhs.y);
	}
	const Pos& operator+=(const Pos &rhs) {
		x+=rhs.x;
		y+=rhs.y;
		return *this;
	}
	const Pos& operator-=(const Pos &rhs) {
		x-=rhs.x;
		y-=rhs.y;
		return *this;
	}
	Pos operator-() const {
		return Pos(-x,-y);
	}
	const Pos& operator=(const Pos &rhs) {
		if (this != &rhs)
		{
			x = rhs.x;
			y = rhs.y;
		}
		return *this;
	}

	int x,y;
};



struct SCandidate
{
	SCandidate() {}
	SCandidate(const Pos &pos0, const linetype ltype0) : pos(pos0), ltype(ltype0) {}
	bool operator<(const SCandidate &rhs);

	Pos pos;
	linetype ltype;
};



enum CHECK_TYPE {
	CHECK_ROW					= 1,
	CHECK_COL						= 1<<1,
	CHECK_SLASH_LEFT		= 1<<2,
	CHECK_SLASH_RIGHT	= 1<<3,
};


struct SSearchInfo
{
	SSearchInfo() : check(0) {}

	int check; // CHECK_TYPE combination
};


struct STable
{
	PIECE pieces[ MAX_TABLE_X][ MAX_TABLE_Y];
	vector<Pos> whites;
	vector<Pos> blacks;

	STable();
	STable(const STable &t);
	const STable& operator=(const STable &rhs);
};



inline linetype GetLineType( const int pieceCnt, const int emptyCnt, const int wallCnt )
{
	if (pieceCnt <= 0)
		return 0;
	return pieceCnt*100 + emptyCnt*10 + wallCnt;
}
inline int GetPieceCntFromlinetype( const linetype &type )
{
	return (type / 100) % 10;
}
inline int GetEmptyCntFromlinetype( const linetype &type )
{
	return (type / 10) % 10;
}
inline int GetWallCntFromlinetype( const linetype &type )
{
	return (type % 10);
}
inline linetype MergeLineType(const linetype ltype0, const linetype ltype1)
{
	return ltype0 * 1000 + ltype1;
}
inline void SeparateLineType(const linetype ltype, OUT linetype &out0, OUT linetype &out1)
{
	out0 = ltype % 1000;
	out1 = ltype / 1000;
}


bool CompareLineType(const linetype ltype0, const linetype ltype1);
bool CompareCombinationLineType(const linetype ltype0, const linetype ltype1);
bool CompareOneSideCombinationLineType(const linetype ltype0, const linetype ltype1);
bool IsCombinationLineType(const linetype ltype);
linetype GetMaxCombinationLineType(const linetype ltype);
linetype GetMinCombinationLineType(const linetype ltype);
int GetLinetypeScore(const linetype ltype);
