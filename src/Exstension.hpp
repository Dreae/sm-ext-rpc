#ifndef INC_SEXT_EXTENSION_H
#define INC_SEXT_EXTENSION_H

#include "sdk/smsdk_ext.h"

class Extension : public SDKExtension {
public:
	virtual bool SDK_OnLoad(char *error, size_t err_max, bool late);
};

extern Extension extension;

#endif
