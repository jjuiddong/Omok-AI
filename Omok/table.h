/**
@filename table.h

*/
#pragma once


class CTable
{
public:
	CTable();
	virtual ~CTable();

	void Init();
	void Render(HDC hdc);
	int NextAIStep(const PIECE pieceType=WHITE);
	GAME_STATE GetGameState();
	bool SetPiece(const Pos &pos, PIECE piece);
	void BackPiece();//한수 뒤로
	Pos ScreenPosToTablePos(const int x, const int y);


private:
	GAME_STATE m_state;
	STable m_table;
	HBRUSH m_brushes[ MAX_BRUSH];
};


inline GAME_STATE CTable::GetGameState() { return m_state; }
