#ifndef INC_SEXT_EXTENSION_H
#define INC_SEXT_EXTENSION_H

#include "sdk/smsdk_ext.h"

#define LOG_MESSAGE(format, ...) \
  smutils->LogMessage(myself, format, ##__VA_ARGS__);

class Extension : public SDKExtension {
public:
  virtual bool SDK_OnLoad(char *error, size_t err_max, bool late);
  virtual void SDK_OnUnload();
};

class SMRPCBase {
  friend class Extension;

public:
  SMRPCBase() {
    next = SMRPCBase::head;
    SMRPCBase::head = this;
  }
  virtual void OnExtLoad() { };
  virtual void OnExtUnload() { };
private:
  SMRPCBase *next;
  static SMRPCBase *head;
};

extern Extension extension;

#endif
