#include "code6cdisassembler.h"
#include <iostream>

int main(int argc, char** argv) {
	std::string inputROM = "FE8_U.gba";
	std::string output = "--to-stdout";
	Code6CDisassembler::offset_t offset = 0;

	// offset = 0x59A1F0; // E_BMAPMAIN for testing

	if (argc >= 2)
		inputROM = argv[1];

	if (argc >= 3)
		offset = std::stol(std::string(argv[2]), nullptr, 0);

	if (argc >= 4)
		output = argv[3];

	if (offset == 0) {
		std::cerr << "USAGE: dis6c <input ROM> <6C code offset> [output file/--to-stdout]" << std::endl;
	} else {
		try {
			Code6CDisassembler dis("FE8_U.gba");
			dis.disassembleAt(offset);

			if (output == "--to-stdout") {
				dis.print(std::cout);
			} else {
				std::ofstream file;

				file.open(output);

				if (!file.is_open())
					throw std::runtime_error(std::string("Couldn't open file for write: ").append(output));

				dis.print(file);
				file.close();
			}
		} catch (std::exception& e) {
			std::cerr << "[dis6c error] " << e.what() << std::endl;
		}
	}

	return 0;
}
