// ****************************************************************************
//             Copyright (C) Oasys Design Systems, Inc. 2004 - 2008
//                             All rights reserved.
// ****************************************************************************
#include "VhdlMsg.h"
#include "Vhdl.h"
#include "VlogMsg.h"
#include "HdlParam.h"

using namespace Verific;

static void VhdlMessageCallback(Verific::msg_type_t, const char *, Verific::linefile_type, const char *, va_list);


void Vhdl::initVhdlMessages()
{
  // Use oasys message handler
  Message::RegisterCallBackMsg(VhdlMessageCallback);

  // Don't send messages to stdout
  Message::SetConsoleOutput(0);

  // Ignore these messages
  const char* ignoreMessages[] = {
    "VERI-1508", // The default veri library search path is now %s
    "VERI-1509", // The veri library search path for library "%s" is now "%s"
    "VHDL-1010", // analyzing architecture %s
    "VHDL-1011", // analyzing configuration %s
    "VHDL-1012", // analyzing entity %s
    "VHDL-1013", // analyzing package body %s
    "VHDL-1014", // analyzing package %s
    "VHDL-1438", // case choice must be a locally static expression
    "VHDL-1487", // Reading VHDL file %s
    "VHDL-1493", // Restoring VHDL parse-tree %s.%s from %s
    "VHDL-1504"  // The default vhdl library search path is now \"%s\"
  };

  const char* openingRtlMessages[] = {
    "VERI-1328", // analyzing included file %s
    "VERI-1482", // Analyzing Verilog file %s
    "VHDL-1481", // Analyzing VHDL file %s
  };

#define ARRAY_SIZE(arr) (sizeof arr / sizeof arr[0])

  for (unsigned int i = 0; i < ARRAY_SIZE(ignoreMessages); i++) {
    Message::SetMessageType(ignoreMessages[i], VERIFIC_IGNORE);
  }

  if (!UParam::reportOpeningRtlFiles()) {
    for (unsigned int i = 0; i < ARRAY_SIZE(openingRtlMessages); i++) {
      Message::SetMessageType(openingRtlMessages[i], VERIFIC_IGNORE);
    }
  }

  // Disable this check: case choice must be a locally static expression
  // because we want to allow globally-static case choices
  Message::SetMessageType("VHDL-1438", VERIFIC_IGNORE);
}

static void VhdlMessageCallback(msg_type_t type, const char *id, linefile_type lf, const char *msg, va_list args)
{
  switch (type) {
  case VERIFIC_NONE:
  case VERIFIC_IGNORE:
  case VERIFIC_COMMENT:
    return;
  default:
    break;
  }

  UMsgModule* msgModule = NULL;

  const char *vhdlPrefix = "VHDL-";
  const char *veriPrefix = "VERI-";
  const char *savePrefix = "VBSR-";
  uint_t vhdlPrefixLen = strlen(vhdlPrefix);
  uint_t veriPrefixLen = strlen(veriPrefix);
  uint_t savePrefixLen = strlen(savePrefix);

  int code = 0;
  bool isVhdl = false;
  bool isVeri = false;
  bool isSave = false;

  if (!id) {
    isVhdl = true;
    // probably from vhdl sorter
  } else if (0 == strncmp(vhdlPrefix, id, vhdlPrefixLen)) {
    isVhdl = true;
  } else if (0 == strncmp(veriPrefix, id, veriPrefixLen)) {
    isVeri = true;
  } else if (0 == strncmp(savePrefix, id, savePrefixLen)) {
    isSave = true;
  } else {
    // ??
    isVhdl = true;
  }

  UMsg* ptr = NULL;
  msgModule = (isSave || isVhdl) ? VhdlMsg::s_mod : VlogMsg::s_mod;
  if (id) {
    code = isSave ? 598 : isVhdl ? atoi(&id[vhdlPrefixLen]) : atoi(&id[veriPrefixLen]);
    ptr = msgModule->getMessage(code);
  }
  if (ptr) {
    if (lf) {
      UTrace trace(lf->GetFileName(), lf->GetRightLine(), lf->GetRightCol());
      ptr->vprintMessage(trace, __FILE__, __LINE__, args);
    } else {
      UTrace trace;
      ptr->vprintMessage(trace, __FILE__, __LINE__, args);
    }
  } else {
    ptr = isVhdl ? VhdlMsg::m_verificError : VlogMsg::m_verificError;
    UStrBuf tmp;
    tmp.vsprint(msg, args);
    if (lf) {
      UTrace trace(lf->GetFileName(), lf->GetRightLine(), lf->GetRightCol());
      ptr->printMessage(trace, __FILE__, __LINE__, tmp.str());
    } else {
      UTrace trace;
      ptr->printMessage(trace, __FILE__, __LINE__, tmp.str());
    }
  }
}
