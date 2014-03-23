
// Test Project
// to use omok line string separator

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <boost/foreach.hpp>

using namespace std;


// 000111000 -> 000 + 111 + 000
void SeparateSide(const string &src, string &first, string &last, string &center)
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


int GetCharCount(const string &str, const char c)
{
	int count = 0;
	BOOST_FOREACH (auto &s, str)
		if (s == c)
			++count;
	return count;
}

// - 001001100 -> return 5, 
// - 001111100
int GetPieceInfo(const string &str)
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
	startPiece = false;
	BOOST_REVERSE_FOREACH(auto &s, str)
	{
		if (!startPiece && ('1' == s))
			break;
		--cnt;
	}
	return cnt;
}


// 1110111 -> 111X + 111
// 1010111 -> 101X + 111
// 1010011 -> 1010 + 011
bool SeparateTwo(string &src, string &dst1, string &dst2)
{
	if (src.length() < 2) // minimum string -> 010 
		return false;

	if (GetPieceInfo(src) <= 5)
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


bool Separate(const string &src, string &dst, string &remainder)
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


bool SeparateAll(const string &src, vector<string> &strs)
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


void main()
{
	string src0 = "0001101010100";
	string src1 = "0000";
	string src2 = "1111";
	string src3 = "00111111010110";
	string src4 = "001111110010110";
	string src5 = "001000111110010110";
	string src6 = "001111110101100010001001101001";
	string src7 = "101011";

	vector<string> dest;
	string src = src6;
	SeparateAll(src, dest);
	

	cout << "source: " << src << endl;
	BOOST_FOREACH (auto &str, dest)
		cout << str << endl;
}
