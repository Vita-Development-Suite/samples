/*
	Vita Development Suite Samples
*/

#include <stdio.h>
#include <stdlib.h>

#include <ini_file_processor.h>
#include <libsysmodule.h>
#include <paf.h>

struct ScePafInitParam {
	SceSize globalHeapSize;
	SceInt32 unk4;
	SceInt32 unk8;
	SceInt32 cdlgMode;
	SceInt32 unk10;
	SceInt32 unk14;
};

int main(void) {
	using namespace sce::Ini;

	// ScePaf is required for SceIniFileProcessor
	auto pafInit = ScePafInitParam{0x500000, 0xEA60, 0x40000, 0, 0, 0};

	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(pafInit), &pafInit, 0, NULL);
	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_INI_FILE_PROCESSOR);

	auto allocator = MemAllocator{malloc, free};
	auto iniInit = InitParameter();
	iniInit.allocator = &allocator;

	{
		auto iniProcessor = IniFileProcessor();
		iniProcessor.initialize(&iniInit);

		if (iniProcessor.open("app0:/default.ini", "r", 0) < 0) {
			printf("Error: 0x%08X\n", iniProcessor.getLastIoError());
		} else {
			char key[SCE_INI_FILE_PROCESSOR_KEY_BUFFER_SIZE];
			char val[0x100];
			while (iniProcessor.parse(key, val, sizeof(val)) != SCE_INI_FILE_PROCESSOR_PARSE_COMPLETED) {
				printf("Parsed %s=%s\n", key, val);
			}
			iniProcessor.close();
		}

		iniProcessor.terminate();
	}

	return 0;
}
