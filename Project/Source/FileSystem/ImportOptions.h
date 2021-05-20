#pragma once

#include "FileSystem/JsonValue.h"

class ImportOptions {
public:
	virtual ~ImportOptions();

	virtual void ShowImportOptions();
	virtual void Load(JsonValue jMeta);
	virtual void Save(JsonValue jMeta);
};
