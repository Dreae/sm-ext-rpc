/**
 * Copyright 2019 Dreae
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *    http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

#ifndef INC_SEXT_EXTENSION_H
#define INC_SEXT_EXTENSION_H

#include "sdk/smsdk_ext.h"

#define LOG_MESSAGE(format, ...) \
  smutils->LogMessage(myself, format, ##__VA_ARGS__);

#define PACKET_TERMINATOR '\n'

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
