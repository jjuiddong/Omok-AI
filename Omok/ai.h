/**
 @filename ai.h
 
 Omok AI 
*/
#pragma once


namespace ai
{
	GAME_STATE SearchBestLocation( STable &table, const PIECE pieceType, OUT Pos &out );

	bool IsGameComplete(STable &table, const PIECE pieceType);
	bool SetPiece(STable &table, const Pos &pos, PIECE piece);
	PIECE GetPiece(STable &table, const Pos &pos);	
	bool CheckEmpty(STable &table, const Pos &pos);
	bool CheckRange(const Pos &pos);
}
