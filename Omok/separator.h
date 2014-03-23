/**
 @filename separator.h
 
 
*/
#pragma once


namespace separator
{

	// 001101001100 -> 0011010 + 01100
	// 0111011100 -> 0111X + 11100
	bool SeparateAll(const string &src, vector<string> &strs);

	void SeparateSide(const string &src, string &first, string &last, string &center);
	int GetPieceInfo(const string &str, int &emptyCnt, int &firstCnt, int &lastCnt);
}
