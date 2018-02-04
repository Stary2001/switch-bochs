#include "config.h"

#if BX_WITH_SWITCH

#ifndef __QNXNTO__
extern "C" {
#endif

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#ifndef __QNXNTO__
}
#endif

#define LOG_THIS genlog->

#include "bochs.h"
#include "osdep.h"
#include "param_names.h"
#include "textconfig.h"
#include "switchconfig.h"

#define CI_PATH_LENGTH 512

#if BX_USE_GUI_CONSOLE
#define bx_printf SIM->bx_printf
#define bx_fgets  SIM->bx_gets
#else
#define bx_printf bx_printf

#include <stdarg.h>
#include <switch.h>

char bx_printf_buffer[2048];
int bx_printf(const char *format, ...)
{
  va_list arg;
  va_start(arg, format);
  int i = vsnprintf(bx_printf_buffer, 2048, format, arg);
  svcOutputDebugString(bx_printf_buffer, strlen(bx_printf_buffer));
  va_end(arg);
  return i;
}

#define bx_fgets  fgets
#endif

#include <stdarg.h>
#include <switch.h>

int bx_switch_config_interface(int menu)
{
  BX_INFO(("ok, hi"));
  return 0;
}

BxEvent* switch_notify_callback(void *unused, BxEvent *event)
{
  return event;
}

static int ci_callback(void *userdata, ci_command_t command)
{
  switch (command)
  {
    case CI_START:
      SIM->set_notify_callback(switch_notify_callback, NULL);
      if (SIM->get_param_enum(BXPN_BOCHS_START)->get() == BX_QUICK_START)
        bx_switch_config_interface(BX_CI_START_SIMULATION);
      else {
        bx_switch_config_interface(BX_CI_START_MENU);
      }
      break;
    case CI_RUNTIME_CONFIG:
      bx_switch_config_interface(BX_CI_RUNTIME);
      break;
    case CI_SHUTDOWN:
      break;
  }
  return 0;
}

int init_switch_config_interface()
{
  SIM->register_configuration_interface("switchconfig", ci_callback, NULL);
  return 0;  // success
}

#endif