
#include "stdafx.h"
#include "table.h"
#include "ai.h"


#define CHECK_TIME_START __int64 freq, start, end; if (QueryPerformanceFrequency((_LARGE_INTEGER*)&freq))  {QueryPerformanceCounter((_LARGE_INTEGER*)&start);  
// a는 float type milli second이고 b가 FALSE일때는 에러입니다
#define CHECK_TIME_END(a,b) QueryPerformanceCounter((_LARGE_INTEGER*)&end); a=(float)((double)(end - start)/freq*1000); b=TRUE; } else b=FALSE;



const POINT g_tableOffset = {18,48};
const int GAPX = 25;
const int GAPY = 25;


CTable::CTable() :
	m_state(GAME)
,	m_procTime(0)
{

}

CTable::~CTable()
{
	for (int i=0; i < MAX_BRUSH; ++i)
		::DeleteObject(m_brushes[i]);
}


void CTable::Init()
{
	m_state = GAME;
	m_table.blacks.clear();
	m_table.whites.clear();
	ZeroMemory(m_table.pieces, sizeof(m_table.pieces));

	m_brushes[0] = ::CreateSolidBrush(RGB(255,255,255));
	m_brushes[1] = ::CreateSolidBrush(RGB(0,0,0));
	m_brushes[2] = ::CreateSolidBrush(RGB(235,235,235));
	m_brushes[3] = ::CreateSolidBrush(RGB(50,50,50));
	m_brushes[4] = ::CreateSolidBrush(RGB(255,255,0));

}


/**
 @brief Set Piece
 @date 2014-03-18
*/
bool CTable::SetPiece(const Pos &pos, PIECE piece)
{
	if (m_state == GAME)
		return ai::SetPiece(m_table, pos, piece);
	else
		return false;
}


/**
 @brief Render
 @date 2014-03-16
*/
void CTable::Render(HDC hdc)
{
	for (int i=0; i < MAX_TABLE_X; ++i)
	{
		int x = g_tableOffset.x + GAPX*i;
		int y = g_tableOffset.y + GAPY*(MAX_TABLE_Y-1);
		MoveToEx(hdc, x, g_tableOffset.y,NULL);
		LineTo(hdc, x, y);

		stringstream ss;
		ss << i;
		string s = ss.str();
		TextOutA(hdc, x-8, g_tableOffset.y-15, s.c_str(), s.length());
	}
	for (int i=0; i < MAX_TABLE_Y; ++i)
	{
		int x = g_tableOffset.x + GAPX*(MAX_TABLE_X-1);
		int y = g_tableOffset.y + GAPY*i;
		MoveToEx(hdc, g_tableOffset.x, y, NULL);
		LineTo(hdc, x, y);

		stringstream ss;
		ss << i;
		string s = ss.str();
		TextOutA(hdc, g_tableOffset.x-18, y-5, s.c_str(), s.length());
	}

	// render white piece
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(0,0,0));
	SelectObject(hdc, m_brushes[ 0]);
	for (unsigned int i=0; i < m_table.whites.size(); ++i)
	{
		const Pos pos = m_table.whites[ i];
		int x = g_tableOffset.x + GAPX*pos.x;
		int y = g_tableOffset.y + GAPY*pos.y;
		RECT r = {x-GAPX/2, y-GAPY/2, x+GAPX/2, y+GAPY/2};

		if (m_table.whites.size()-1 == i)
			SelectObject(hdc, m_brushes[ 2]);
		Ellipse(hdc, r.left, r.top, r.right, r.bottom);

		if (m_isDisplayNumber)
		{
			stringstream ss;
			ss << i+1;
			string s = ss.str();
			TextOutA(hdc, r.left+4, r.top+4, s.c_str(), s.length());
		}
	}

	// render black piece
	SetTextColor(hdc, RGB(255,255,255));
	SelectObject(hdc, m_brushes[ 1]);
	for (unsigned int i=0; i < m_table.blacks.size(); ++i)
	{
		const Pos pos = m_table.blacks[ i];
		int x = g_tableOffset.x + GAPX*pos.x;
		int y = g_tableOffset.y + GAPY*pos.y;
		RECT r = {x-GAPX/2, y-GAPY/2, x+GAPX/2, y+GAPY/2};

		if (m_table.blacks.size()-1 == i)
		{
			SelectObject(hdc, m_brushes[ 0]);
			Ellipse(hdc, r.left, r.top, r.right, r.bottom);
			SelectObject(hdc, m_brushes[ 1]);
			Ellipse(hdc, r.left+2, r.top+2, r.right-2, r.bottom-2);
		}
		else
		{
			Ellipse(hdc, r.left, r.top, r.right, r.bottom);
		}

		if (m_isDisplayNumber)
		{
			stringstream ss;
			ss << i+1;
			string s = ss.str();
			TextOutA(hdc, r.left+4, r.top+4, s.c_str(), s.length());
		}
	}
	
	if ((WIN_BLACK == m_state) || (WIN_WHITE == m_state))
	{
		SetTextColor(hdc, RGB(0,0,0));
		SelectObject(hdc, m_brushes[ 0]);
		Rectangle(hdc, 300, 0,400, 30);
		string msg[2] ={string("Black Win!!"), string("White Win!!")};
		if (WIN_BLACK == m_state)
			TextOutA(hdc, 320, 4, msg[0].c_str(), msg[0].length());
		else
			TextOutA(hdc, 320, 4, msg[1].c_str(), msg[1].length());
	}


	// 설명문구 출력.
	SetTextColor(hdc, RGB(0,0,0));
	{	// 오목 인공지능 계산에 걸린 시간 출력
		stringstream ss;
		ss << "ai process time(millisecond): " << m_procTime;
		const string str = ss.str();
		TextOutA(hdc, 10, 0, str.c_str(), str.length());
	}
	{ // 키설명
		stringstream ss;
		ss << "tab key : display number";
		const string str = ss.str();
		TextOutA(hdc, 10, 16, str.c_str(), str.length());
	}
}



/**
 @brief ScreenPosToTablePos
 @date 2014-03-16
*/
Pos CTable::ScreenPosToTablePos(const int x, const int y)
{
	Pos r((x - g_tableOffset.x + GAPX/2)/GAPX, (y - g_tableOffset.y+GAPY/2)/GAPY);
	return r;
}


/**
 @brief 다음 돌은 인공지능이 알아서 놓는다. 기본적으로 white 로 한다.
		     흰색돌이 이기면 0
			 흑돌이 이기면 1을 리턴한다.
			 종료상태가 아니라면 -1을 리턴한다.
 @date 2014-03-16
*/
int CTable::NextAIStep(const PIECE pieceType) //pieceType=WHITE
{
	if (ai::IsGameComplete(m_table, OppositePiece(pieceType)))
	{
		m_state = (pieceType==WHITE)? WIN_BLACK : WIN_WHITE;
		return -1;
	}

	bool err;
	CHECK_TIME_START;
	{
		Pos pos;
		m_state = ai::SearchBestLocation(m_table, pieceType, pos);
		if (GAME == m_state)
			SetPiece(pos, pieceType);
	}
	CHECK_TIME_END(m_procTime, err);

	if (ai::IsGameComplete(m_table, pieceType))
	{
		m_state = (pieceType==WHITE)? WIN_WHITE : WIN_BLACK;
	}

	return -1;
}


/**
 @brief 한수 뒤로 물리기
 @date 2014-03-19
*/
void CTable::BackPiece()
{
	if (m_state != GAME)
		m_state = GAME;

	if (!m_table.blacks.empty())
	{
		SetPiece(m_table.blacks.back(), EMPTY);
		m_table.blacks.pop_back();
	}
	if (!m_table.whites.empty())
	{
		SetPiece(m_table.whites.back(), EMPTY);
		m_table.whites.pop_back();
	}
}
