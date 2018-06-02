#include "EncodingModes.h"
#include <iostream>
#include <sstream>
#include <iomanip>

bool Res::EncodeEnv::insertTag(std::vector<Token_t>& tokens, size_t elemId)
{
	if (tokens[1] == ":")
	{
		tags[tokens[0]] = elemId;
		tokens.erase(tokens.begin());
		tokens.erase(tokens.begin());

		return true;
	}
	return false;
}

void Res::EncodeEnv::showListing()
{
	Res::Address_t currAddres = 0x0;
	currentSectionName = ".text";

	for (auto itSection : sections)
	{
		Res::Address_t currAddres = 0x0;
		for (auto &itElem : itSection.second.elementsIds)
		{
			sectionElements[itElem].address = currAddres;
			sectionElements[itElem].nextAddress(currAddres, &sectionElements[itElem]);
		}
	}

	showSymtab();


	currentSectionName = ".text";
	Address_t sectionLength = getCurrentSectionLength();
	if (sections[".data"].sectionAddressApplied)
		sectionLength = sections[".data"].currentSectionStartAddress;
	for (auto &itElem : sections[".data"].elementsIds)
	{
		sectionElements[itElem].address += sectionLength;
	}

	if (sections[".text"].sectionAddressApplied)
	{
		sectionLength = sections[".text"].currentSectionStartAddress;
		for (auto &itElem : sections[".text"].elementsIds)
		{
			sectionElements[itElem].address += sectionLength;
		}
	}

	for (auto it : relocations)
	{
		Encode_t init
			= ((sectionElements[it.objId].bytes[0] << 24) & 0xFF)
			| ((sectionElements[it.objId].bytes[1] << 16) & 0xFF)
			| ((sectionElements[it.objId].bytes[2] << 8) & 0xFF)
			| ((sectionElements[it.objId].bytes[3]) & 0xFF)
			;
		sectionElements[it.objId].bytes.clear();
		sectionElements[it.objId].insert32(it.relocate(this, init));
	}

	for (auto itElem : sectionElements)
	{
		if (itElem.displayMode == SectionElem::DisplayMode::EStandard)
		{
			std::cout << std::setfill('0') << std::setw(8) << std::hex << itElem.address << "\t";
			for (auto it : itElem.bytes)
				std::cout << std::setfill('0') << std::setw(2) << std::hex << (int)it;
			std::cout << std::setfill(' ') << "\t" << std::setw(15) << itElem.verbose << "\n";
		}
		else if (itElem.displayMode == SectionElem::DisplayMode::EMinimalVerbose)
		{
			std::cout << std::setfill(' ') << itElem.verbose << "\n";
		}

		itElem.nextAddress(currAddres, &itElem);
	}

}

void Res::EncodeEnv::showSymtab()
{
	std::cout << "symtab:\n\\\\\\\\\\\\\n";
	for (auto itTag : tags)
	{
		std::cout << itTag.first << ", " << sectionElements[itTag.second].sectionName << ", " << sectionElements[itTag.second].address << "\n";
	}
	std::cout << "\n\\\\\\\\\\\\\\\\\\\n";
}

void Res::EncodeEnv::encode(const std::string & line)
{
	std::vector<Res::Token_t> tokens;
	Res::tokenizeWithString(tokens, line.c_str(),
		[](char c) {
		/// delimiters
		return isblank(c) || isspace(c)
			|| (c == '\"') || (c == '\'')
			|| (c == '#') || (c == ',') || (c == '\"') || (c == '\\')
			|| (c == '(') || (c == ')') || (c == ':')
			;
		},

		/// ignored delimiters
		[](char c) {
			return isspace(c) || (c == ',') || (c == '#') || (c == '\"') || (c == '\"') || (c == '\\');
		},
		/// ending delimiters
		[](char c) {
			return (c == '#');
		}
	);
	
	Token_t tag = tokens[0];
	bool b = insertTag(tokens, sectionElements.size());
	encode(tokens);
	if (b)
	{
		sectionElements.back().verbose = tag + ":\t" + sectionElements.back().verbose;
	}
}

void Res::EncodeEnv::encode(const std::vector<Token_t>& tokens)
{
	if (tokens.empty())
		return;

	auto directiveIt = directives.find(tokens[0]);
	if (directiveIt != directives.end())
	{
		auto directive = (*directiveIt).second;
		directive( tokens, this );
		return;
	}
	
	auto instructionIt = instructions.find(tokens[0]);
	if(instructionIt != instructions.end() )
	{
		auto instruction = (*instructionIt).second;

		SectionElem elem;
		elem.insert32( 
			instruction.encode(
				instruction.opcode, tokens, this,			
				instruction.flag1, instruction.flag2, instruction.flag3, instruction.flag4
			));
		
		/// text representation
		elem.verbose = tokens[0] + "\t";

		for (int i = 1; i < tokens.size() - 1; ++i)
			elem.verbose +=  tokens[i] + ", ";
		elem.verbose += tokens[tokens.size() - 1];

		addSectionElem(elem, allignment);
		return;
	}


	std::cerr << "unknown insctuction or directive \"" << tokens[0] << "\"\n";
}

std::vector<uint8_t> Res::EncodeEnv::encodeString(const Token_t & token)
{
	std::vector<uint8_t> r;
	for (auto it = token.begin(); it != token.end(); ++it)
	{
		if (*it == '\\')
		{
			++it;
			if (*it == '\\')
				r.push_back('\\');
			else if (*it == 'n')
				r.push_back('\n');
			else if (*it == 't')
				r.push_back('\t');
			else if (*it == '\"')
				r.push_back('\"');
		}
		else
		{
			r.push_back(*it);
		}
	}

	return r;
}

Res::Encode_t Res::EncodeEnv::encodeConstant(Token_t str)
{
	if (str[0] == '-' && str[1] == '0' && str[2] == 'x')
	{
		str.erase(str.begin() + 1);
		str.erase(str.begin() + 1);

		return stol(str, nullptr, 16);
	}
	else
	if (str[0] == '0' && str[1] == 'x')
	{
		str.erase(str.begin());
		str.erase(str.begin());

		return stol(str, nullptr, 16);
	}else
		return stol(str, nullptr, 10);
}

void Res::EncodeEnv::loadRengisterNames()
{
	addRengisterName("zero", 0);
	addRengisterName("at", 1);
	addRengisterName("v0", 2);
	addRengisterName("v1", 3);


	addRengisterName("a0", 4);
	addRengisterName("a1", 5);
	addRengisterName("a2", 6);
	addRengisterName("a3", 7);

	addRengisterName("t0", 8);
	addRengisterName("t1", 9);
	addRengisterName("t2", 10);
	addRengisterName("t3", 11);
	addRengisterName("t4", 12);
	addRengisterName("t5", 13);
	addRengisterName("t6", 14);
	addRengisterName("t7", 15);

	addRengisterName("s0", 16);
	addRengisterName("s1", 17);
	addRengisterName("s2", 18);
	addRengisterName("s3", 19);
	addRengisterName("s4", 20);
	addRengisterName("s5", 21);
	addRengisterName("s6", 22);
	addRengisterName("s7", 23);


	addRengisterName("t8", 24);
	addRengisterName("t9", 25);


	addRengisterName("k0", 26);
	addRengisterName("k1", 27);


	addRengisterName("gp", 28);
	addRengisterName("sp", 29);
	addRengisterName("fp", 30);
	addRengisterName("ra", 31);
	///////////////////////////
	for (Encode_t i = 0; i < 32; ++i)
	{
		std::stringstream stream;
		stream << i;
		addRengisterName(stream.str(), i);
	}
}
void Res::EncodeEnv::loadBaseInstructions()
{
	addInstruction_I("addi", 0x8);
	addInstruction_I("addiu", 0x9);

	addInstruction_I("slti", 0xa);
	addInstruction_I("sltiu", 0xb);


	addInstruction_I("andi", 0xc);
	addInstruction_I("ori", 0xc);
	addInstruction_I("xori", 0xe); 

	addInstruction_I("lui", 0xf, 0, 1);
		
	///												sizes:					|6 |5  |5 | 5 | 5	 | 6|
	addInstruction_R("sll", 0x0, 0, 2, 1, 3 );	 	/// sll rd, rt, shamt	|0 |rs |rt| rd| shamt| 0| : /// ask about shift codding
	addInstruction_R("srl", 0x2, 0, 2, 1, 3 ); 		/// srl rd, rt, shamt	|0 |rs |rt| rd| shamt| 2| : /// ask about shift codding
	addInstruction_R("sra", 0x3, 0, 2, 1, 3 ); 		/// sra rd, rt, shamt	|0 |rs |rt| rd| shamt| 3| : /// ask about shift codding
	
	addInstruction_R("sllv", 0x4);
	addInstruction_R("srlv", 0x6);
	addInstruction_R("srav", 0x7);


	/////////////////////////////
	/////////////////////
	//////////

	addInstruction_R("mfhi", 0x10, 0, 0, 1, 0);
	addInstruction_R("mflo", 0x12, 0, 0, 1, 0);

	////////////////////////////
	//////////////
	////////

	addInstruction_R("mthi", 0x11, 0, 1, 0, 0);
	addInstruction_R("mtlo", 0x13, 0, 1, 0, 0);
	
	////////////////////////////////////////////
	/////////////////
	///////

	/// sizes:			|6 |5  |5  |10 |6
	/// mult rs, rt		|0 |rs |rt |0  |0x18
	addInstruction_R("mult", 0x18	, 1, 2, 0, 0);
	addInstruction_R("multu", 0x19	, 1, 2, 0, 0);
	addInstruction_R("div", 0x1a	, 1, 2, 0, 0);
	addInstruction_R("divu", 0x1b	, 1, 2, 0, 0);


	addInstruction_R("add", 0x20);
	addInstruction_R("addu", 0x21);
	addInstruction_R("sub", 0x22);
	addInstruction_R("subu", 0x23);
	addInstruction_R("and", 0x24);
	addInstruction_R("or", 0x25);
	addInstruction_R("xor", 0x26);
	addInstruction_R("nor", 0x27);


	addInstruction_R("slt", 0x2a);
	addInstruction_R("sltu", 0x2b);


	/////////////////////// jump & condition

	addInstruction_I("beq", 0x4, 1, 2);
	addInstruction_I("bne", 0x5, 1, 2);
	addInstruction_J("j", 0x2);
	addInstruction_J("j", 0x2);
	addInstruction_J("jal", 0x2);
	addInstruction_J("jr", 0x2);

}

void Res::EncodeEnv::loadDirectives()
{
	addDirective("text", [](const std::vector<Token_t> &tokens, EncodeEnv*env) 
	{
		SectionElem elem;
		elem.verbose = ".text";
		elem.displayMode = SectionElem::EMinimalVerbose;
		elem.nextAddress = [=](Address_t& addr, const SectionElem* elem)
		{
			env->changeSection(".text");
		};
		env->addSectionElem(elem, 0);
	});
	addDirective("data", [](const std::vector<Token_t> &tokens, EncodeEnv*env) 
	{
		SectionElem elem;
		elem.verbose = ".data";
		elem.displayMode = SectionElem::EMinimalVerbose;
		elem.nextAddress = [=](Address_t& addr, const SectionElem* elem) 
		{
			env->changeSection(".data");
		};
		
		env->addSectionElem(elem, 0);
	});
	addDirective("ascii", [](const std::vector<Token_t> &tokens, EncodeEnv*env) 
	{ 
		SectionElem elem;
		elem.bytes = env->encodeString(tokens[1]);
		elem.verbose = ".ascii\t\"" + tokens[1] + "\"";
		env->addSectionElem(elem);
	});
	addDirective("asciiz", [](const std::vector<Token_t> &tokens, EncodeEnv*env)
	{
		SectionElem elem;
		elem.bytes = env->encodeString(tokens[1]);
		elem.bytes.push_back(0);
		elem.verbose = ".asciiz\t\"" + tokens[1] + "\"";
		env->addSectionElem(elem);
	});
	addDirective("word", [](const std::vector<Token_t> &tokens, EncodeEnv*env)
	{
		SectionElem elem;
		elem.verbose = ".word\t";

		//for (int i = 1; i < tokens.size() - 1; ++i)
		//	elem.verbose += tokens[i] + ", ";
		//elem.verbose += tokens[tokens.size() - 1];

		for (int i = 1; i < tokens.size(); ++i)
		{
			int16_t t;

			if (tokens[i] == "%lo")
			{
				elem.verbose += tokens[i] + " ( " + tokens[i + 1] + " ), ";
				RelocationElem rel;
				rel.objId = env->sectionElements.size();
				rel.tagId = tokens[i + 1]; ++i;
				rel.type = RelocationElem::ELo;
				env->addRelocation(rel);
				elem.insert32(0);
			}
			else if (tokens[i] == "%hi")
			{
				elem.verbose += tokens[i] + " (" + tokens[i + 1] + "), ";
				RelocationElem rel;
				rel.objId = env->sectionElements.size();
				rel.tagId = tokens[i + 1]; ++i;
				rel.type = RelocationElem::EHi;
				env->addRelocation(rel);
				elem.insert32(0);
			}
			else if (isdigit(tokens[i][0]) || (tokens[i][0] == '-'))
			{
				t = env->encodeConstant(tokens[i]);
				elem.insert32(t);
				elem.verbose += tokens[i] + ", ";
			}
			else
			{
				elem.verbose += tokens[i] + ", ";
				RelocationElem rel;
				rel.objId = env->sectionElements.size();
				rel.tagId = tokens[i];
				rel.type = RelocationElem::ERel;
				env->addRelocation(rel);
				elem.insert32(0);
			}


		}
		elem.verbose.resize(elem.verbose.size() - 2);

		env->addSectionElem(elem);
	});
	addDirective("half", [](const std::vector<Token_t> &tokens, EncodeEnv*env)
	{
		SectionElem elem;
		elem.verbose = ".half\t";

		for (int i = 1; i < tokens.size() - 1; ++i)
			elem.verbose += tokens[i] + ", ";
		elem.verbose += tokens[tokens.size() - 1];

		for (int i = 1; i < tokens.size(); ++i)
		{
			int16_t t = env->encodeConstant(tokens[i]);
			elem.insert16(t);
		}

		env->addSectionElem(elem);
	});
	addDirective("byte", [](const std::vector<Token_t> &tokens, EncodeEnv*env)
	{
		SectionElem elem;
		elem.verbose = ".byte\t";

		for (int i = 1; i < tokens.size() - 1; ++i)
			elem.verbose += tokens[i] + ", ";
		elem.verbose += tokens[tokens.size() - 1];

		for(int i = 1; i < tokens.size(); ++i)
		{
			int16_t t = env->encodeConstant(tokens[i]);
			elem.insert8(t);
		}

		env->addSectionElem(elem);
	});
	addDirective("space", [](const std::vector<Token_t> &tokens, EncodeEnv*env)
	{
		SectionElem elem;
		elem.verbose = ".space\t" + tokens[1];

		Token_t str = tokens[1];
		int16_t n = env->encodeConstant(tokens[1]);

		for(int i = 0; i < n; ++i)
			elem.insert8(0);
		
		env->addSectionElem(elem);
	});
	addDirective("org", [](const std::vector<Token_t> &tokens, EncodeEnv*env)
	{
		SectionElem elem;
		elem.verbose = ".org\t" + tokens[1];
		Address_t orgAddr = env->encodeConstant(tokens[1]);
		elem.nextAddress = [=](Address_t& addr, const SectionElem* elem)
		{
			//addr = orgAddr;
			env->setNextSectionAddress(orgAddr);
		};

		elem.displayMode = SectionElem::EMinimalVerbose;

		env->addSectionElem(elem, 0);
	});
	addDirective("allign", [](const std::vector<Token_t> &tokens, EncodeEnv*env)
	{
		SectionElem elem;
		elem.verbose = ".allign\t" + tokens[1];

		env->setAllignment(env->encodeConstant(tokens[1]));
		elem.displayMode = SectionElem::EMinimalVerbose;

		env->addSectionElem(elem,0);
	});
}

Res::Encode_t Res::EncodeEnv::encodeI(Encode_t opCode, const std::vector<Token_t>& tokens, EncodeEnv* env,
	uint8_t flag1, uint8_t flag2, uint8_t flag3, uint8_t flag4)
{
	/// opcode
	Encode_t r = (opCode << (5 + 5 + 16));
	Encode_t t;
	
	if(flag1)
	{
		t = env->findRengister(tokens[flag1]);
		r |= (t << (16));
	}

	if (flag2)
	{
		Encode_t t = env->findRengister(tokens[flag2]);
		r |= (t << (5 + 16));
	}
	

	/// imm
	{
		const Token_t &tok = tokens[std::max(flag1, flag2) + 1];

		if (tok == "%lo")
		{
			RelocationElem rel;
			rel.objId = env->sectionElements.size();
			rel.tagId = tokens[std::max(flag1, flag2) + 2];
			rel.type = RelocationElem::ELo;
			env->addRelocation(rel);
		}
		else if (tok == "%hi")
		{
			RelocationElem rel;
			rel.objId = env->sectionElements.size();
			rel.tagId = tokens[std::max(flag1, flag2) + 2];
			rel.type = RelocationElem::EHi;
			env->addRelocation(rel);
		}
		else if ( isdigit(tok[0]) || (tok[0] == '-') )
		{
			t = env->encodeConstant(tok);
		}
		else
		{
			RelocationElem rel;
			rel.objId = env->sectionElements.size();
			rel.tagId = tokens[std::max(flag1, flag2) + 2];
			rel.type = RelocationElem::ERel;
			env->addRelocation(rel);
		}

		r |= (t & 0xFFFF);
		
	}

	return r;
}

Res::Encode_t Res::EncodeEnv::encodeR(Encode_t opCode, const std::vector<Token_t>& tokens, EncodeEnv* env,
	uint8_t flag1, uint8_t flag2, uint8_t flag3, uint8_t flag4)
{
	/// opcode 
	Encode_t r = opCode;
	
	Encode_t t;

	if (flag1)
	{
		t = env->findRengister(tokens[flag1]);
		r |= (t << (6));
	}

	if (flag2)
	{
		t = env->findRengister(tokens[flag2]);
		r |= (t << (6 + 5));
	}

	if (flag3)
	{
		t = env->findRengister(tokens[flag3]);
		r |= (t << (6 + 5 + 5));
	}

	if (flag4)
	{
		t = env->findRengister(tokens[flag4]);
		r |= (t << (6 + 5 + 5 + 5));
	}
	return r;
}

Res::Encode_t Res::EncodeEnv::encodeJ(Encode_t opCode, const std::vector<Token_t>& tokens, EncodeEnv* env,
	uint8_t flag1, uint8_t flag2, uint8_t flag3, uint8_t flag4)
{
	Encode_t r = (opCode << (5 + 5 + 16));
	Encode_t t;

	{
		if (tokens[1] == "%lo")
		{
			RelocationElem rel;
			rel.objId = env->sectionElements.size();
			rel.tagId = tokens[2];
			rel.type = RelocationElem::ELo;
			env->addRelocation(rel);
		}
		else if (tokens[1] == "%hi")
		{
			RelocationElem rel;
			rel.objId = env->sectionElements.size();
			rel.tagId = tokens[2];
			rel.type = RelocationElem::EHi;
			env->addRelocation(rel);
		}
		else if (isdigit(tokens[1][0]) || (tokens[1][0] == '-'))
		{
			t = env->encodeConstant(tokens[1]);
		}
		else
		{
			RelocationElem rel;
			rel.objId = env->sectionElements.size();
			rel.tagId = tokens[2];
			rel.type = RelocationElem::EAbs;
			env->addRelocation(rel);
		}

		r |= t && 0xFFFFFF;
	}
	return r;
}

Res::Encode_t Res::RelocationElem::relocate(EncodeEnv* env, Encode_t init)
{
	Address_t currentAddress = env->sectionElements[objId].address;

	std::cout << init << std::endl;
	Encode_t t;
	switch (type)
	{
	case ELo:
		t = env->sectionElements[env->tags[tagId]].address;
		return init | (t & 0xFFFF);
	case EHi:
		t = env->sectionElements[env->tags[tagId]].address >> 4;
		return init | (t & 0xFFFF);
	case RelocationType::ERel:
		t = env->sectionElements[env->tags[tagId]].address + currentAddress;
		return init | (t & 0xFFFF);
	case RelocationType::EAbs:
		t = env->sectionElements[env->tags[tagId]].address;
		return init | (t & 0xFFFFFF);
	}
	
}
