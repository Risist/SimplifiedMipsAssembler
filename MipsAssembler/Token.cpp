#include "Token.h"

namespace Res
{

	void tokenize(std::vector<Token_t>& tokens, const char* text, 
		bool(*isDelimiter)(char), 
		bool(*isDelimiterOmmit)(char), 
		bool(*isDelimiterFinish)(char)
	)
	{
		const char* itLast = text;
		size_t n = 1;
		for (const char* it = text; *it; ++it, ++n)
		{
			char c = *it;
			if (isDelimiter(c))
			{
				if (n > 1)
					tokens.push_back(std::string(itLast, n - 1));
				if (!isDelimiterOmmit(c))
					tokens.push_back(std::string(it, 1));
				
				n = 0;
				itLast = it + 1;
				if (isDelimiterFinish(c))
					break;
			}
		}
		if (n > 1)
			tokens.push_back(std::string(itLast, n - 1));
	}

	void tokenizeWithString(std::vector<Token_t>& tokens, const char* text,
		bool(*isDelimiter)(char),
		bool(*isDelimiterOmmit)(char),
		bool(*isDelimiterFinish)(char)
	)
	{
		const char* itLast = text;
		size_t n = 1;
		bool atString = false;
		for (const char* it = text; *it; ++it, ++n)
		{
			char c = *it;

			if (atString)
			{
				if (c == '\"' && *(it-1) != '\\')
				{
					tokens.push_back(std::string(itLast, n - 1));
					n = 0;
					itLast = it + 1;
					atString = false;
				}
			}else if (isDelimiter(c))
			{	
				if (n > 1)
					tokens.push_back(std::string(itLast, n - 1));
				if (!isDelimiterOmmit(c))
					tokens.push_back(std::string(it, 1));
				

				n = 0;
				itLast = it + 1;
				if (c == '\"')
					atString = true;
				if (isDelimiterFinish(c))
					break;
			}

			
		}
		if (n > 1)
			tokens.push_back(std::string(itLast, n - 1));
	}
}