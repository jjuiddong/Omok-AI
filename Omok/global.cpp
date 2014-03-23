
#include "stdafx.h"
#include "global.h"


/**
 @brief operator
 @date 2014-03-17
*/
bool SCandidate::operator<(const SCandidate &rhs)
{
	bool r = CompareLineType(ltype, rhs.ltype);
	return r;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// STable
STable::STable()
{
}

STable::STable(const STable &t)
{
	operator=(t);
}

const STable& STable::operator=(const STable &rhs)
{
	if (this != &rhs)
	{
		memcpy_s(pieces, sizeof(pieces), rhs.pieces, sizeof(rhs.pieces));
		blacks = rhs.blacks;
		whites = rhs.whites;
	}
	return *this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////




/**
 @brief 연속된 돌의 타입으로 어떤 것이 더 높은  우선순위를 가지는지를 리턴한다.
			ltype0 < ltype1
 @date 2014-03-17
*/
bool CompareLineType(const linetype ltype0, const linetype ltype1)
{
	const bool isComb0 = IsCombinationLineType(ltype0);
	const bool isComb1 = IsCombinationLineType(ltype1);
	if (isComb0 && isComb1)
	{
		return CompareCombinationLineType(ltype0, ltype1);
	}

	if (isComb0 || isComb1)
	{
		return CompareOneSideCombinationLineType(ltype0, ltype1);
	}

	const int score0 = GetLinetypeScore(ltype0);
	const int score1 = GetLinetypeScore(ltype1);
	return score0 < score1;
}


/**
 @brief 두개 이상 조합된 라인타입이라면 true를 리턴한다.
 @date 2014-03-17
*/
bool IsCombinationLineType(const linetype ltype)
{
	return (ltype / 10000) > 0;
}


/**
 @brief 조합된 라인타입끼리 비교 operator< 
 @date 2014-03-17
*/
bool CompareCombinationLineType(const linetype ltype0, const linetype ltype1)
{
	const linetype ltype0Max = GetMaxCombinationLineType(ltype0);
	const linetype ltype1Max = GetMaxCombinationLineType(ltype1);
	const linetype ltype0Min = GetMinCombinationLineType(ltype0);
	const linetype ltype1Min = GetMinCombinationLineType(ltype1);
	
	const int score0Max = GetLinetypeScore(ltype0Max);
	const int score1Max = GetLinetypeScore(ltype1Max);

	if (score0Max == score1Max)
	{
		return CompareLineType(ltype0Min, ltype1Min);
	}
	return score0Max < score1Max;
}


/**
 @brief 한쪽만 조합된 라인타입일 때 비교
 @date 2014-03-17
*/
bool CompareOneSideCombinationLineType(const linetype ltype0, const linetype ltype1)
{
	linetype cltype0 = ltype0;
	if (IsCombinationLineType(ltype0))
	{
		cltype0 = GetMaxCombinationLineType(ltype0);
	}

	linetype cltype1 = ltype1;
	if (IsCombinationLineType(ltype1))
	{
		cltype1 = GetMaxCombinationLineType(ltype1);
	}

	const int score0Max = GetLinetypeScore(cltype0);
	const int score1Max = GetLinetypeScore(cltype1);
	if (score0Max == score1Max)
	{
		if (IsCombinationLineType(ltype0))
			return false;
		return true;
	}

	return score0Max < score1Max;
}


/**
 @brief 조합된 라인타입에서 더 높은 우선순위의 라인타입을 리턴한다.
 @date 2014-03-17
*/
linetype GetMaxCombinationLineType(const linetype ltype)
{
	linetype sltype[2];
	SeparateLineType(ltype, sltype[0], sltype[1]);
	const int maxLtype = CompareLineType(sltype[ 0], sltype[ 1]);
	if (1 == maxLtype)
		return sltype[ 1];
	return sltype[ 0];
}
linetype GetMinCombinationLineType(const linetype ltype)
{
	linetype sltype[2];
	SeparateLineType(ltype, sltype[0], sltype[1]);
	const int maxLtype = CompareLineType(sltype[ 0], sltype[ 1]);
	if (1 == maxLtype)
		return sltype[ 0];
	return sltype[ 1];
}

/**
 @brief 라인타입을 점수로 환산해서 리턴한다.
 @date 2014-03-18
*/
int GetLinetypeScore(const linetype ltype)
{
	if (IsCombinationLineType(ltype))
	{
		const linetype maxLinetype =  GetMaxCombinationLineType(ltype);
		return GetLinetypeScore(maxLinetype);
	}

	const int pieceCnt0 = GetPieceCntFromlinetype(ltype);
	const int emptyCnt0 = GetEmptyCntFromlinetype(ltype);
	const int firstCnt0 = GetFirstCntFromlinetype(ltype);
	const int lastCnt0 = GetLastCntFromlinetype(ltype);
	const int length0 = pieceCnt0 + emptyCnt0 + firstCnt0 + lastCnt0;
	
	if (pieceCnt0 >= 6)
	{
		int a = 0;
	}
	else if (pieceCnt0 == 5)
	{
		//X111101X
		//if (emptyCnt0 == 1)
		//{
		//	//// X111101X
		//	//if (length0 >= 2)
		//	//	return 0;
		//	return 100;
		//}
		//else if (emptyCnt0 >= 2)
		//{
		//	// 1010111, 1110101
		//	if (emptyCnt0 == 2) // 예외 X1101011X -> X
		//	{
		//		return 100;
		//	}
		//	else
		//	{
		//		//if (length0 == 0)
		//		//	return 30; // 11010101
		//		else if (length0 == 1)
		//			return 20;
		//		else if (length0 >= 2)
		//			return 0; // X10110101X
		//		
		//		// 0111101X -> ok (이미 처리된 상태)
		//		// X111101 -> X
		//		return 30;
		//	}
		//}

		return 100;
	}

	if (pieceCnt0 == 4)
	{
		if (length0 > 5) 
		{
			return 49;
		}
		else if (length0 == 5)
		{
		//	//if (emptyCnt0 == 0)
		//	//	return 46;
		//	//if (emptyCnt0 == 1) // 11011
		//	//	return 46;
		//	//else // 101101, 110101
		//	//	return 40;
		//	return 40;
			return 40;
		}
		else 
		{
			//if (emptyCnt0 == 0)
			//	return 0;
			//else if (emptyCnt0 == 1) // X11011X
			//	return 46;
			//else
			//	return 0;
			return 0;
		}
	}
	else if (pieceCnt0 == 3)
	{
		if (length0 <= 4)
			return 0; // X111X, X1101X, X1110X
		else
		{
			if ((emptyCnt0==1) && (firstCnt0 >= 1) && (lastCnt0 >= 1))
				return 39; // 011010, 010110
			else if ((emptyCnt0 <= 0) && (firstCnt0 >= 1) && (lastCnt0 >= 1))
				return 39; // 01110
			else if ((emptyCnt0 <= 1) && ((firstCnt0 >= 1) || (lastCnt0 >= 1)))
				return 30; // 0111X, X11010
			else if (emptyCnt0 >= 2)
				return 30; // 10101, 11001
			else
				return 0;
		}

		//if (firstCnt0 == 0)
		//{
		//	if (emptyCnt0 == 0)
		//		return 39;
		//	else if (emptyCnt0 == 1) // 1101
		//		return 39;
		//	else // 10101
		//		return 30;
		//}
		//else if (firstCnt0 == 1)
		//{
		//	return 30;
		//}
		//else
		//{
		//	if (emptyCnt0 == 2) // X10101X
		//		return 30;
		//	return 0;
		//}
	}
	else if (pieceCnt0 == 2)
	{
		if (length0 < 4) // maximum -> 001100, minimum -> 1100
			return 0;
		else if ((emptyCnt0==0) && (length0 < 5))
			return 0; // X0110X
		else if ((firstCnt0 >= 1) || (lastCnt0 >= 1))
			return 29; // 0110, 01010
		else 
			return 20; // X11000
	}
	else if (pieceCnt0 == 1)
	{
		if ((firstCnt0 >= 2) || (lastCnt0 >= 2))
			return 10; // 001, 100, 00100
		else
			return 0;
	}

	return 0;
}
