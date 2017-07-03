#ifndef CODE6CDISASSEMBLER_H
#define CODE6CDISASSEMBLER_H

#include <fstream>
#include <map>
#include <string>

class Code6CDisassembler {
public:
	using offset_t = std::uint32_t;

	enum Code6CIndex {
		CODE_END           = 0x00,
		CODE_SET_NAME      = 0x01,
		CODE_CALL_ROUTINE  = 0x02,
		CODE_LOOP_ROUTINE  = 0x03,
		CODE_SET_DESTRUCT  = 0x04,
		CODE_NEW_CHILD     = 0x05,
		CODE_NEW_CHILD_BLK = 0x06,
		CODE_NEW_MAIN_BUG  = 0x07,
		CODE_UNTIL_EXISTS  = 0x08,
		CODE_END_ALL       = 0x09,
		CODE_BREAK_ALL     = 0x0A,
		CODE_LABEL         = 0x0B,
		CODE_GOTO          = 0x0C,
		CODE_JUMP          = 0x0D,
		CODE_SLEEP         = 0x0E,
		CODE_SET_MARK      = 0x0F,
		CODE_BLOCK         = 0x10,
		CODE_END_IF_DUPL   = 0x11,
		CODE_SET_BIT4      = 0x12,
		CODE_13            = 0x13,
		CODE_WHILE_ROUTINE = 0x14,
		CODE_15            = 0x15,
		CODE_CALL_ROUTINE2 = 0x16,
		CODE_END_DUPLICATE = 0x17,
		CODE_CALL_ROUTINEA = 0x18,
		CODE_19            = 0x19,

		Code6CIndex_Count
	};

	struct Code6CBit {
		std::uint_fast16_t index;
		std::uint_fast16_t sarg;
		std::uint_fast32_t larg;
	};

public:
	Code6CDisassembler(std::string sourceFileName);
	~Code6CDisassembler();

	void disassembleAt(offset_t offset);
	void print(std::ostream& out) const;

private:
	Code6CBit _getBit(offset_t offset);

	bool _handleCode(const Code6CBit& code);
	void _handleName(offset_t offset);

	static inline offset_t _fromPointer(std::uint_fast32_t ptr) { return ptr & 0xFFFFFF; }

private:
	std::ifstream mSource;

	std::map<offset_t, Code6CBit> mCodes;
	std::map<offset_t, std::string> mLabels;
	std::map<offset_t, std::string> mNames;

	int mLabelCounter = 0;
};

#endif // CODE6CDISASSEMBLER_H
