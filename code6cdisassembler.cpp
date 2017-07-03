#include "code6cdisassembler.h"

static std::string::value_type toHexDigit(std::uint32_t value) {
	value = (value & 0xF);

	if (value < 10)
		return '0' + value;
	return 'A' + (value - 10);
}

static std::string toHexDigits(std::uint32_t value, int digits) {
	std::string result;
	result.resize(digits);

	for (int i=0; i<digits; ++i)
		result[digits-i-1] = toHexDigit(value >> (4*i));

	return result;
}

Code6CDisassembler::Code6CDisassembler(std::string sourceFileName) {
	mSource.open(sourceFileName, std::ios::in | std::ios::binary);

	if (!mSource.is_open())
		throw std::runtime_error(std::string("Couldn't open file for binary read: ").append(sourceFileName));
}

Code6CDisassembler::~Code6CDisassembler() {
	if (mSource.is_open())
		mSource.close();
}

void Code6CDisassembler::disassembleAt(offset_t offset) {
	bool doContinue = true;

	if (mLabels.find(offset) == mLabels.end())
		mLabels[offset] = std::string("Code6C_").append(std::to_string(mLabelCounter++));

	do {
		if (mCodes.find(offset) != mCodes.end())
			return;

		Code6CBit bit = _getBit(offset);

		doContinue = _handleCode(bit);
		mCodes[offset] = bit;

		offset = offset + 8;
	} while (doContinue);
}

void Code6CDisassembler::print(std::ostream& out) const {
	static const std::array<std::string, (Code6CIndex_Count+1)> outTemplates = {
		"_6C_END",
		"_6C_SET_NAME(parg)",
		"_6C_CALL_ROUTINE(parg)",
		"_6C_LOOP_ROUTINE(parg)",
		"_6C_SET_DESTRUCTOR(parg)",
		"_6C_NEW_CHILD(parg)",
		"_6C_NEW_CHILD_BLOCKING(parg)",
		"_6C_NEW_MAIN_BUGGED(parg)",
		"_6C_UNTIL_EXISTS(parg)",
		"_6C_END_ALL(parg)",
		"_6C_BREAK_ALL_LOOP(parg)",
		"_6C_LABEL(sarg)",
		"_6C_GOTO(sarg)",
		"_6C_JUMP(parg)",
		"_6C_SLEEP(sarg)",
		"_6C_SET_MARK(sarg)",
		"_6C_BLOCK",
		"_6C_END_IF_DUPLICATE",
		"_6C_SET_BIT4",
		"_6C_13",
		"_6C_WHILE_ROUTINE(parg)",
		"_6C_15",
		"_6C_CALL_ROUTINE_2(parg)",
		"_6C_END_DUPLICATES",
		"_6C_CALL_ROUTINE_ARG(parg, sarg)",
		"_6C_19",
		"SHORT index sarg; WORD larg"
	};

	offset_t naturalCurrent = 0;

	for (std::pair<offset_t, Code6CBit> pair : mCodes) {
		if (pair.first != naturalCurrent) {
			out << "\nORG 0x" << toHexDigits(pair.first, 6) << std::endl;
			naturalCurrent = pair.first;
		}

		auto it = mLabels.find(pair.first);

		if (it != mLabels.end())
			out << it->second << ":\n";

		int index = pair.second.index;
		offset_t parg  = _fromPointer(pair.second.larg);

		std::string indexStr = std::string("0x").append(toHexDigits(pair.second.index, 4));
		std::string sargStr  = std::string("0x").append(toHexDigits(pair.second.sarg,  4));
		std::string largStr  = std::string("0x").append(toHexDigits(pair.second.larg,  8));

		std::string pargStr; {
			auto labelIt = mLabels.find(parg);

			if (labelIt != mLabels.end())
				pargStr = labelIt->second;
			else
				pargStr = std::string("0x").append(toHexDigits(parg, 6));
		}

		std::string outStr = outTemplates.at(index < Code6CIndex_Count ? index : Code6CIndex_Count);

		std::string::size_type found = std::string::npos;

		while ((found = outStr.find("index")) != std::string::npos)
			outStr.replace(found, 5, indexStr);

		while ((found = outStr.find("sarg")) != std::string::npos)
			outStr.replace(found, 4, sargStr);

		while ((found = outStr.find("larg")) != std::string::npos)
			outStr.replace(found, 4, largStr);

		while ((found = outStr.find("parg")) != std::string::npos)
			outStr.replace(found, 4, pargStr);

		out << "\t" << outStr;

		{
			auto nameIt = mNames.find(parg);
			if (nameIt != mNames.end())
				out << " // \"" << nameIt->second << "\"";
		}

		out << std::endl;

		naturalCurrent += 8;
	}
}

Code6CDisassembler::Code6CBit Code6CDisassembler::_getBit(offset_t offset) {
	Code6CBit result;

	mSource.seekg(offset);

	result.index  = mSource.get();
	result.index |= mSource.get() << 8;

	result.sarg   = mSource.get();
	result.sarg  |= mSource.get() << 8;

	result.larg   = mSource.get();
	result.larg  |= mSource.get() << 8;
	result.larg  |= mSource.get() << 16;
	result.larg  |= mSource.get() << 24;

	return result;
}

bool Code6CDisassembler::_handleCode(const Code6CBit& code) {
	switch (code.index) {
	case CODE_END:
		return false;

	case CODE_SET_NAME:
		_handleName(_fromPointer(code.larg));
		return true;

	case CODE_NEW_CHILD:
	case CODE_NEW_CHILD_BLK:
	case CODE_NEW_MAIN_BUG:
	case CODE_UNTIL_EXISTS:
	case CODE_END_ALL:
	case CODE_BREAK_ALL:
	case CODE_JUMP:
		disassembleAt(_fromPointer(code.larg));
		return true;

	default:
		return true;
	}
}

void Code6CDisassembler::_handleName(offset_t offset) {
	mSource.seekg(offset);

	std::string name;

	while (char c = mSource.get())
		name.push_back(c);

	mNames[offset] = name;
}
