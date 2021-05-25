#pragma once

#include "Math/float2.h"

class IMouseClickHandler { //Interace that must be implemented by those SelectableComponents that we want the MouseClick to affect
public:
	~IMouseClickHandler() {}
	virtual void OnClicked() = 0;
	virtual void OnClickedInternal() = 0;
};
