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

struct SceSysmoduleLoadOpt {
	SceUInt32 flags;
	SceInt32 *pRes;
	SceInt32 unused[2];
};

int main(void) {
	using namespace sce::Ini;

	// ScePaf is required for SceIniFileProcessor
	auto pafInit = ScePafInitParam{0x500000, 0xEA60, 0x40000, 0, 0, 0};
	auto loadOpt = SceSysmoduleLoadOpt();

	sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(pafInit), &pafInit, &loadOpt);
	sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_INI_FILE_PROCESSOR);

	auto allocator = MemAllocator{malloc, free};
	auto iniInit = InitParameter();
	iniInit.allocator = &allocator;

	{
		auto iniProcessor = IniFileProcessor();
		iniProcessor.initialize(&iniInit);
		if (iniProcessor.open("app0:/default.ini", "r", 0) < 0) {
			iniProcessor.terminateForError();
		} else {
			char key[0x80];
			char val[0x100];
			while (iniProcessor.parse(key, val, 0x100) != SCE_INI_FILE_PROCESSOR_PARSE_COMPLETED) {
				printf("Parsed %s=%s\n", key, val);
			}
			iniProcessor.cleanup();
			iniProcessor.terminate();
		}
	}

	return 0;
}
