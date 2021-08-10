#pragma once

class Panel {
public:
	Panel(const char* name, bool enabled);

	virtual void Update();

public:
	const char* name = "";
	bool enabled = true;
};
