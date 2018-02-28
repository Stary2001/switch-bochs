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

#include "siminterface.h"
#include "osdep.h"
#include "param_names.h"
#include "switchconfig.h"

extern "C"
{
#include "font/common.h"
}

#define CI_PATH_LENGTH 512

#if BX_USE_GUI_CONSOLE
#define bx_printf SIM->bx_printf
#define bx_fgets  SIM->bx_gets
#else
#define bx_printf printf
#define bx_fgets  fgets
#endif

#include <stdarg.h>
#include <switch.h>


static const char *startup_menu_prompt =
"------------------------------\n"
"Bochs Configuration: Main Menu\n"
"------------------------------\n"
"\n"
"This is the Bochs Configuration Interface, where you can describe the\n"
"machine that you want to simulate.  Bochs has already searched for a\n"
"configuration file (typically called bochsrc.txt) and loaded it if it\n"
"could be found.  When you are satisfied with the configuration, go\n"
"ahead and start the simulation.\n"
"\n"
"You can also start bochs with the -q option to skip these menus.\n"
"\n"
"1. Restore factory default configuration\n"
"2. Read options from...\n"
"3. Edit options\n"
"4. Save options to...\n"
"5. Restore the Bochs state from...\n"
"6. Begin simulation\n"
"7. Quit now\n"
"\n"
"Please choose one: [%d] ";

static const char *startup_options_prompt =
"------------------\n"
"Bochs Options Menu\n"
"------------------\n"
"0. Return to previous menu\n"
"1. Optional plugin control\n"
"2. Logfile options\n"
"3. Log options for all devices\n"
"4. Log options for individual devices\n"
"5. CPU options\n"
"6. CPUID options\n"
"7. Memory options\n"
"8. Clock & CMOS options\n"
"9. PCI options\n"
"10. Bochs Display & Interface options\n"
"11. Keyboard & Mouse options\n"
"12. Disk & Boot options\n"
"13. Serial / Parallel / USB options\n"
"14. Network card options\n"
"15. Sound card options\n"
"16. Other options\n"
#if BX_PLUGINS
"17. User-defined options\n"
#endif
"\n"
"Please choose one: [0] ";

static const char *runtime_menu_prompt =
"---------------------\n"
"Bochs Runtime Options\n"
"---------------------\n"
"1. Floppy disk 0: %s\n"
"2. Floppy disk 1: %s\n"
"3. CDROM runtime options\n"
"4. Log options for all devices\n"
"5. Log options for individual devices\n"
"6. USB runtime options\n"
"7. Misc runtime options\n"
"8. Save configuration\n"
"9. Continue simulation\n"
"10. Quit now\n"
"\n"
"Please choose one:  [9] ";

static const char *plugin_ctrl_prompt =
"\n-----------------------\n"
"Optional plugin control\n"
"-----------------------\n"
"0. Return to previous menu\n"
"1. Load optional plugin\n"
"2. Unload optional plugin\n"
"\n"
"Please choose one:  [0] ";

void bx_switch_config_interface_init()
{}

int bx_switch_config_interface(int menu)
{
  switch(menu)
  {
    case BX_CI_INIT:
      bx_switch_config_interface_init();
    return 0;

    case BX_CI_START_SIMULATION:
      SIM->begin_simulation(bx_startup_flags.argc, bx_startup_flags.argv);
    break;

    case BX_CI_START_MENU:
      Bit32u n_choices = 7;
      Bit32u default_choice;
      switch (SIM->get_param_enum(BXPN_BOCHS_START)->get()) {
        case BX_LOAD_START:
          default_choice = 2; break;
        case BX_EDIT_START:
          default_choice = 3; break;
        default:
          default_choice = 6; break;
      }
      printf(startup_menu_prompt, default_choice);

      int choice = default_choice;
      switch(choice)
      {
          case 6:
            bx_switch_config_interface(BX_CI_START_SIMULATION);
          break;
      }
    break;
  }
  return 0;
}

#define N_LINES 16
struct console_line
{
  int y;
  int h;
  int stage;
};

int current_line = 0;
struct console_line console_lines[N_LINES];

int console_y = 0;
uint8_t* g_framebuf;
u32 g_framebuf_width;
u32 g_framebuf_stride;
u32 g_framebuf_height;

bool flush_called = false;

int console_tick()
{
  int x, y;

  for(int i = 0; i < N_LINES; i++)
  {
    int stage = console_lines[i].stage;
    if(stage != 0)
    {
      console_lines[i].stage--;
      stage--;
      for(y = console_lines[i].y; y < console_lines[i].y + console_lines[i].h; y++)
      {
        for(x = 0; x < 1280*4; x+=4)
        {
          g_framebuf[y * g_framebuf_stride + x + 3] = (stage * 255) / N_LINES; // Set the alpha!
        }
      }
    }
  }
}

BxEvent* switch_notify_callback(void *unused, BxEvent *event)
{
  switch(event->type)
  {
    case BX_SYNC_EVT_TICK:
      if(appletMainLoop())
      {
        event->retcode = 0;
      }
      else
      {
        event->retcode = -1;
      }

      static int tick_count = 0;
      tick_count++;
      if(tick_count == 100)
      {
        tick_count = 0;
        console_tick();
      }

      return event;
    case BX_ASYNC_EVT_REFRESH:
      // Ignore.
      return event;

    case BX_ASYNC_EVT_DBG_MSG:
    case BX_ASYNC_EVT_LOG_MSG:
    {
      console_tick();

      int line_height = tahoma12->height + 2;
      console_lines[current_line].stage = N_LINES;
      console_lines[current_line].y = 0;
      console_lines[current_line].h = line_height;
      current_line++;
      if(current_line == N_LINES)
      {
        current_line = 0;
      }

      int line_bytes = g_framebuf_stride * line_height;
      memmove(g_framebuf + line_bytes, g_framebuf, g_framebuf_stride * (g_framebuf_height - line_height));
      memset(g_framebuf, 0, line_bytes);

      for(int i = current_line; i < current_line+N_LINES-1; i++)
      {
        console_lines[i % N_LINES].y += line_height;
      }

      DrawText(g_framebuf, g_framebuf_width, tahoma12, 0, 0, MakeColor(255, 255, 255, 255), event->u.logmsg.msg);

      if(!flush_called) // lol hack
      {
        u8 *real_fb = gfxGetFramebuffer(NULL, NULL);
        memcpy(real_fb, g_framebuf, g_framebuf_height * g_framebuf_stride);
        gfxFlushBuffers();
        gfxSwapBuffers();
      }

      delete [] event->u.logmsg.msg;
      return event;
    }
    default:
      fprintf(stderr, "textconfig: notify callback called with event type %04x\n", event->type);
      return event;
  }
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
      break;
    case CI_SHUTDOWN:
      break;
  }
  return 0;
}

int init_switch_config_interface()
{
  g_framebuf_width = 1280;
  g_framebuf_stride = g_framebuf_width * 4;
  g_framebuf_height = (tahoma12->height + 2) * N_LINES;
  g_framebuf = (uint8_t*)malloc(g_framebuf_stride * g_framebuf_height * 2);

  SIM->register_configuration_interface("switch", ci_callback, NULL);
  return 0;  // success
}

#endif