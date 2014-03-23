
#include "stdafx.h"
#include "separator.h"

namespace separator
{
	int GetCharCount(const string &str, const char c);
	bool SeparateTwo(string &src, string &dst1, string &dst2);
	bool Separate(const string &src, string &dst, string &remainder);
	int GetPieceRangeCount(const string &str);
}

using namespace separator;


// 000111000 -> 000 + 111 + 000
void separator::SeparateSide(const string &src, string &first, string &last, string &center)
{
	const int n1 = src.find('1');
	const int n2 = src.find_last_of('1')+1;
	if (n1 != string::npos)
		first = src.substr(0, n1);
	if (n2 != string::npos)
		last = src.substr(n2);

	const int fst1 = (n1 == string::npos)? 0 : n1;
	const int lst1 = (n2 == string::npos)? src.length() : n2;
	center = src.substr(fst1, lst1-fst1);
}


int separator::GetCharCount(const string &str, const char c)
{
	int count = 0;
	BOOST_FOREACH (auto &s, str)
		if (s == c)
			++count;
	return count;
}


// - 001001100 -> emptyCnt=2, firstCnt=2, lastCnt=2, return 3,  (return piece Count)
// - 001111100 -> emptyCnt=0, firstCnt=2, lastCnt=2, return 5
// - 01011100 -> emptyCnt=1, firstCnt=1, lastCnt=2, return 5
int separator::GetPieceInfo(const string &str, OUT int &emptyCnt, int &firstCnt, int &lastCnt)
{
	bool startPiece = false;
	int cnt = 0;
	emptyCnt = 0;
	firstCnt = 0;
	lastCnt = 0;
	BOOST_FOREACH (auto &s, str)
	{
		if (!startPiece && ('1' == s))
			startPiece = true;
		if ('1' ==s)
			++cnt;
		if (startPiece && (('0' ==s) || ('S' ==s)))
			++emptyCnt;
		if (!startPiece && (('0' ==s) || ('S' ==s)))
			++firstCnt;
	}
	BOOST_REVERSE_FOREACH(auto &s, str)
	{
		if (('0' ==s) || ('S' ==s))
			++lastCnt;
		if ('1' == s)
			break;
		--emptyCnt;
	}
	return cnt;
}


// - 001001100 -> return 5
// - 01110 -> return 3
// - 0110100 -> return 4
int separator::GetPieceRangeCount(const string &str)
{
	bool startPiece = false;
	int cnt = 0;
	BOOST_FOREACH (auto &s, str)
	{
		if (!startPiece && ('1' == s))
			startPiece = true;
		if (startPiece)
			++cnt;
	}
	BOOST_REVERSE_FOREACH(auto &s, str)
	{
		if ('1' == s)
			break;
		--cnt;
	}
	return cnt;
}


// 1110111 -> 111X + 111
// 1010111 -> 101X + 111
// 1010011 -> 1010 + 011
bool separator::SeparateTwo(string &src, string &dst1, string &dst2)
{
	if (src.length() < 2) // minimum string -> 010 
		return false;

	if (GetPieceRangeCount(src) <= 5)
		return false;

	const int zeroCnt = GetCharCount(src, '0');
	if (zeroCnt <= 0)
		return false;
	if ((zeroCnt == 1) && ((src.front() == '0') || (src.back() == '0')))
		return false;

	const int last1 = src.find_last_of('1');
	const int idx = src.find_last_of('0', last1);
	if (((idx-1 >= 0) && (src[ idx-1] == '0')) ||
		((idx+1 < (int)src.length()) && (src[ idx+1] == '0')))
	{
		// nothing~
	}
	else
	{
		src[ idx] = 'X';
	}

	dst1 = src.substr(0, idx);
	dst2 = src.substr(idx);
	return true;
}


/**
 @brief 
 @date 2014-03-21
*/
bool separator::Separate(const string &src, string &dst, string &remainder)
{
	string first, last, center;
	SeparateSide(src, first, last, center);

	const int zeroCnt = GetCharCount(center, '0');
	const int oneCnt = GetCharCount(center, '1');
	if ((zeroCnt+oneCnt) <= 5) {
		dst = src;
		return false;
	}

	int cnt = 0;
	bool perfectSeparate = false;
	for (unsigned int i=0; i < center.size(); ++i)
	{
		dst += center[ i];

		if ((cnt >= 4) && (center[ i] == '0'))
		{
			if ((center[ i-1] == '0') ||
				((i+1 < center.size()) && (center[ i+1] == '0')))
			{
				// nothing
			}
			else
			{ // separate only one '0' zero piece
				dst.back() = 'X'; // can't locate position
			}
			break;
		}

		++cnt;	
	}

	string simpleDst1, simpleDst2;
	if (SeparateTwo(dst, simpleDst1, simpleDst2))
	{
		dst = first + simpleDst1;
		if ((int)center.size() > cnt)
			remainder = simpleDst2 + center.substr(cnt+1) + last;
		else
			remainder = simpleDst2 + last;
	}
	else
	{
		dst = first + dst;
		if ((int)center.size() > cnt)
			remainder = center.substr(cnt+1) + last;
		else
			remainder = last;
	}

	return true;
}


/**
 @brief 
 @date 2014-03-21
*/
bool separator::SeparateAll(const string &src, vector<string> &strs)
{
	string tmpSrc = src;
	bool loop = true;
	while (loop)
	{
		string dst, remainder;
		loop = Separate(tmpSrc, dst, remainder);
		strs.push_back(dst);
		tmpSrc = remainder;
	}

	return true;
}
