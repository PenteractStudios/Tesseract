#include "ModuleFiles.h"

#include "Globals.h"
#include "Utils/Logging.h"

#include "physfs.h"

#include "Utils/Leaks.h"

bool ModuleFiles::Init() {
	PHYSFS_init(nullptr);
	PHYSFS_mount(".", nullptr, 0);
	PHYSFS_setWriteDir(".");
	return true;
}

bool ModuleFiles::CleanUp() {
	PHYSFS_deinit();

	return true;
}

Buffer<char> ModuleFiles::Load(const char* filePath) const {
	Buffer<char> buffer = Buffer<char>();

	PHYSFS_File* file = PHYSFS_openRead(filePath);
	if (!file) {
		LOG("Error opening file %s (%s).\n", filePath, PHYSFS_getLastError());
		return buffer;
	}
	DEFER {
		PHYSFS_close(file);
	};

	PHYSFS_sint64 size = PHYSFS_fileLength(file);
	if (size < 0) {
		LOG("File size couldn't be determined for %s (%s).\n", filePath, PHYSFS_getLastError());
		return buffer;
	}

	buffer.Allocate(size + 1);
	char* data = buffer.Data();
	PHYSFS_sint64 numBytes = PHYSFS_readBytes(file, data, size);
	if (numBytes < size) {
		LOG("Error reading file %s (%s).\n", filePath, PHYSFS_getLastError());
		return buffer;
	}
	data[size] = '\0';

	return buffer;
}

bool ModuleFiles::Save(const char* filePath, const Buffer<char>& buffer, bool append) const {
	return Save(filePath, buffer.Data(), buffer.Size(), append);
}

bool ModuleFiles::Save(const char* filePath, const char* buffer, size_t size, bool append) const {
	PHYSFS_File* file = append ? PHYSFS_openAppend(filePath) : PHYSFS_openWrite(filePath);
	if (!file) {
		LOG("Error saving file %s (%s).\n", filePath, strerror(errno));
		return false;
	}
	DEFER {
		PHYSFS_close(file);
	};

	PHYSFS_sint64 numBytes = PHYSFS_writeBytes(file, buffer, size);
	if (numBytes < size) {
		LOG("Error writing to file %s (%s).\n", filePath, PHYSFS_getLastError());
		return false;
	}

	return true;
}

void ModuleFiles::CreateFolder(const char* folderPath) const {
	if (!PHYSFS_mkdir(folderPath)) LOG(PHYSFS_getLastError());
}

void ModuleFiles::Erase(const char* path) const {
	if (!PHYSFS_delete(path)) {
		LOG("Can't erase file %s. (%s)\n", path, PHYSFS_getLastError());
	}
}

std::vector<std::string> ModuleFiles::GetFilesInFolder(const char* folderPath) const {
	std::vector<std::string> files;
	char** filesList = PHYSFS_enumerateFiles(folderPath);

	unsigned i = 0;
	char* file = filesList[i];
	while (file != nullptr) {
		files.push_back(file);
		file = filesList[++i];
	}

	PHYSFS_freeList(filesList);
	return files;
}

bool ModuleFiles::Exists(const char* path) const {
	PHYSFS_Stat fileStats;
	return PHYSFS_stat(path, &fileStats) != 0;
}

bool ModuleFiles::IsDirectory(const char* path) const {
	PHYSFS_Stat fileStats;
	PHYSFS_stat(path, &fileStats);
	return fileStats.filetype == PHYSFS_FileType::PHYSFS_FILETYPE_DIRECTORY;
}

long long ModuleFiles::GetLocalFileModificationTime(const char* path) const {
	PHYSFS_Stat fileStats;
	PHYSFS_stat(path, &fileStats);
	return fileStats.modtime;
}
std::string ModuleFiles::GetFilePath(const char* file, bool absolute) const {
	const char* realDir = PHYSFS_getRealDir(file);

	if (realDir == nullptr) {
		realDir = "";
	}

#ifdef _DEBUG
	return std::string(PHYSFS_getBaseDir()) + "..\\..\\Game\\" + std::string(realDir) + "\\" + file;
#else
	return std::string(PHYSFS_getBaseDir()) + std::string(realDir) + "\\" + file;
#endif
}

void ModuleFiles::AddSearchPath(const char* searchPath) const {
	PHYSFS_mount(searchPath, nullptr, 1);
}
