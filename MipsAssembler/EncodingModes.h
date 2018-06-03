#pragma once

#include "Token.h"
#include <map>

namespace Res
{

	typedef uint32_t Address_t;
	typedef uint32_t Encode_t;

	class SectionElem
	{
	public:
		std::string verbose;
		std::vector<uint8_t> bytes;

		enum DisplayMode
		{
			EStandard,
			EMinimalVerbose,
			ESize
		} displayMode = EStandard;

		Address_t getByteLength() const { return bytes.size(); }
		void insert8(uint8_t s) { bytes.push_back(s); }
		void insert32(uint32_t s)
		{ 
			uint8_t b1 = s & 0xFF; s = (s >> 8);
			uint8_t b2 = s & 0xFF; s = (s >> 8);
			uint8_t b3 = s & 0xFF; s = (s >> 8);
			uint8_t b4 = s & 0xFF; 

			bytes.push_back(b4);
			bytes.push_back(b3);
			bytes.push_back(b2);
			bytes.push_back(b1);
		}
		void insert16(uint16_t s)
		{
			uint8_t b1 = s & 0xFF; s = (s >> 8);
			uint8_t b2 = s & 0xFF; s = (s >> 8);

			bytes.push_back(b2);
			bytes.push_back(b1);
		}

		std::function<void(Address_t&, const SectionElem*)> nextAddress = [](Address_t& addr, const SectionElem* elem) { addr += elem->bytes.size(); };
		Address_t address;
		Token_t sectionName;
	};

	class Section_t
	{
	public:
		std::vector<size_t> elementsIds;
		Address_t currentSectionStartAddress = 0x0;
		bool sectionAddressApplied = false;
	};

	class EncodeEnv;

	class RelocationElem
	{
	public:
		size_t objId;
		Token_t tagId;

		enum RelocationType
		{
			ELo,
			EHi,
			ERel,
			EAbs,
			ESize
		}type;

		Encode_t relocate( EncodeEnv* env, Encode_t init);
	};

	class EncodeEnv
	{
	public:
		typedef std::function<void(const std::vector<Token_t> &, EncodeEnv*)>  Directive_t;
	private:
		struct EncodeStruct
		{
			Encode_t(*encode)(Encode_t opCode, const std::vector<Token_t> &, EncodeEnv*,
				uint8_t, uint8_t, uint8_t, uint8_t) = nullptr;

			/// activates encoding specified field
			uint8_t flag1 : 3;
			uint8_t flag2 : 3;
			uint8_t flag3 : 3;
			uint8_t flag4 : 3;

			Encode_t opcode;
		};

	public:

		void showListing();
		void showSymtab();
		void showRelatab(const Token_t& sectionName);

		/////////////// Encode functions

		/// reads tokens from vector and returns decoded instruction
		void					encode				(const std::string& line);
		void					encode				(const std::vector<Token_t> &tokens);
		std::vector<uint8_t>	encodeString		(const Token_t &tokens);
		Encode_t				encodeConstant		(Token_t tokens);


		bool insertTag				(std::vector<Token_t> &tokens, size_t elemId);


		/// clears
		void clearSections			()	{ sectionElements.clear();					}
		void clearTags				()	{ tags.clear();						}
		void clear					()	{ tags.clear(); sectionElements.clear();	}

		/////////////////////////////////// init functions
		void loadRengisterNames		();
		void loadBaseInstructions	();
		void loadDirectives			();

		////////
		void addRengisterName		(const Token_t& name, Encode_t id)
		{
			rengisterNames["$" + name] = id;
		}
		Encode_t findRengister		(const Token_t& name)
		{
			return rengisterNames[name];
		}

		////// Add Instruction

		void addInstruction_I		(const Token_t& name, Encode_t opCode,
			uint8_t flag2 = 2, uint8_t flag1 = 1)
		{
			instructions[name] = { encodeI,  flag1, flag2, 0, 0, opCode };
		}
		void addInstruction_R		(const Token_t& name, Encode_t funct,
			uint8_t flag4 = 2, uint8_t flag3 = 3, uint8_t flag2 = 1, uint8_t flag1 = 0)
		{
			instructions[name] = { encodeR, flag1, flag2, flag3, flag4, funct };
		}
		void addInstruction_J(const Token_t& name, Encode_t opcode)
		{
			instructions[name] = { encodeJ, 0, 0, 0, 0, opcode};
		}

		void addDirective(const Token_t& name, Directive_t func)
		{
			directives["." + name] = { func };
		}

		void addSectionElem(SectionElem s)
		{
			addSectionElem(s, allignment);
		}
		void addSectionElem(SectionElem s, size_t allignment)
		{
			size_t n = 1 << allignment;
			if (allignment != 0 && s.bytes.size() % n != 0)
			{
				SectionElem ss = s;

				while (ss.bytes.size() % n != 0)
					ss.insert8(0);
				ss.sectionName = currentSectionName;
				sectionElements.push_back(ss);
			}
			else
			{
				s.sectionName = currentSectionName;
				sectionElements.push_back(s);
			}

			sections[currentSectionName].elementsIds.push_back(sectionElements.size()-1);
		}
		void setAllignment(size_t s) 
		{ 
			allignment = s;
		}
		void changeSection(const Token_t& sectionName)
		{
			currentSectionName = sectionName;
			if (nextSectionAddressApplied)
			{
				setCurrentSectionAddress(nextSectionAddress);
				nextSectionAddressApplied = false;
			}
		}

		Address_t getCurrentSectionLength()
		{
			Address_t max = 0x0;
			for (auto i : sections[currentSectionName].elementsIds)
			{
				max = std::max(sectionElements[i].address + sectionElements[i].getByteLength(), max);
			}
			return max;
		}
		void setCurrentSectionAddress(Address_t s)
		{
			sections[currentSectionName].currentSectionStartAddress = s;
			sections[currentSectionName].sectionAddressApplied = true;
		}
		void setNextSectionAddress(Address_t s)
		{
			nextSectionAddress = s;
			nextSectionAddressApplied = true;
		}

		void addRelocation(const RelocationElem& elem)
		{
			relocations.push_back(elem);
		}

	private:
		std::map<Token_t, Section_t>			sections;
		std::vector<SectionElem>				sectionElements;
		std::string currentSectionName;
		
		std::map<Token_t, Encode_t>				rengisterNames;
		std::map<Token_t, EncodeStruct>			instructions;
		std::map<Token_t, Directive_t>			directives;

		std::vector<RelocationElem>				relocations;

		/// list of available tags;
		/// tags are in format of id in @sectionElements
		/// address of tag the same as address of element it is pointing to
		std::map<Token_t, size_t>			tags;

		size_t allignment = 0;
		Address_t nextSectionAddress = 0x0;
		bool nextSectionAddressApplied = false;
		
		/// Encode fucntions for internal use;
		
		static Encode_t encodeI		(Encode_t opCode, const std::vector<Token_t> &tokens, EncodeEnv* env,
			uint8_t flag1, uint8_t flag2, uint8_t flag3, uint8_t flag4);
		static Encode_t encodeR		(Encode_t opCode, const std::vector<Token_t> &tokens, EncodeEnv* env, 
			uint8_t flag1, uint8_t flag2, uint8_t flag3, uint8_t flag4);

		static Encode_t encodeJ		(Encode_t opCode, const std::vector<Token_t> &tokens, EncodeEnv* env,
			uint8_t flag1, uint8_t flag2, uint8_t flag3, uint8_t flag4);


		friend class RelocationElem;
	};
}
	
