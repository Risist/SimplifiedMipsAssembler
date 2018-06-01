#pragma once
#include <cctype>

#include <vector>
#include <stack>
#include <string> 
#include <cstring>
#include <algorithm>
#include <functional>

namespace Res
{
	typedef std::string Token_t;

	inline bool isComma(char c)
	{
		return (c == ',') | (c == ';');
	}
	inline bool isBracket(char c)
	{
		return (c == '(') | (c == ')') | (c == '[') | (c == ']') | (c == '<') | (c == '>');
	}
	inline bool isOperator(char c)
	{
		return (c == '+') | (c == '-') | (c == '*') | (c == '/') | (c == '%') | (c == '^');
	}
	inline bool isOperator(const std::string& c)
	{
		return (c == "+") | (c == "-") | (c == "*") | (c == "/") | (c == "%") | (c == "^");
	}
	inline bool isDelimiter(char c)
	{
		return isblank(c) || isOperator(c) || isBracket(c) || isspace(c) || isComma(c)
			|| (c == '\"') || (c == '\'');//|| (c == '=' )
		;
	}

	inline bool isConstant(const char* c)
	{
		if (*c == 0 || (*c != '-' && !isdigit(*c)))
			return false;
		int dot = 0;
		for (const char* it = c + 1; *it; ++it)
		{
			if (*it == '.')
				++dot;

			if (!isdigit(*it) && dot > 1)
				return false;
		}
		return true;
	}


	/// inserts tokens from @text into @tokens array
	/// @tokens [out]	array of output tokens
	/// @text	[in]	input text to tokenize
	void tokenize(std::vector<Token_t>& tokens, 
		const char* text, bool (*isDelimiter)(char),
		bool (*isDelimiterOmmit)(char),
		bool(*isDelimiterFinish)(char)
	);

	/// inserts tokens from @text into @tokens array
	/// @tokens [out]	array of output tokens
	/// @text	[in]	input text to tokenize
	void tokenizeWithString (std::vector<Token_t>& tokens,
		const char* text, bool(*isDelimiter)(char),
		bool(*isDelimiterOmmit)(char),
		bool(*isDelimiterFinish)(char)
	);
}