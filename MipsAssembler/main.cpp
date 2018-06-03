#include <iostream>
#include "Token.h"
#include "EncodingModes.h"
#include <bitset>
#include <iomanip>

using namespace std;

#include <climits>

template <typename T>
T swap_endian(T u)
{
	static_assert (CHAR_BIT == 8, "CHAR_BIT != 8");

	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

int main()
{

	Res::EncodeEnv env;
	env.loadRengisterNames();
	env.loadBaseInstructions();
	env.loadDirectives();
	//env.changeSection(".text");
	

	while (true)
	{
		env.showListing();
		//cout << "currentSectionLength = " << env.getCurrentSectionLength() << std::endl;
		//cout << "input >> ";
		string s;
		getline(cin, s);
		system("clear");
		env.encode(s);
	}

	system("pause");
}