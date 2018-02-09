/////////////////////////////////////////////////////////////////////////
// $Id: switch.cc 13043 2017-01-15 12:57:06Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2001-2017  The Bochs Project
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA


// Define BX_PLUGGABLE in files that can be compiled into plugins.  For
// platforms that require a special tag on exported symbols, BX_PLUGGABLE
// is used to know when we are exporting symbols and when we are importing.
#define BX_PLUGGABLE

#include "bochs.h"
#include "plugin.h"
#include "param_names.h"

#if BX_WITH_SWITCH
#include "icon_bochs.h"
#include "font/vga.bitmap.h"
class bx_switch_gui_c : public bx_gui_c {
public:
  bx_switch_gui_c (void) {}

  DECLARE_GUI_VIRTUAL_METHODS()
  DECLARE_GUI_NEW_VIRTUAL_METHODS()

  void vga_draw_char(int x, int y, char c);

  unsigned int guest_xchars;
  unsigned int guest_ychars;
  u32 vga_fg;
  u32 vga_bg;
};

// declare one instance of the gui object and call macro to insert the
// plugin code
static bx_switch_gui_c *theGui = NULL;
IMPLEMENT_GUI_PLUGIN_CODE(switch)

#define LOG_THIS theGui->

// This file defines stubs for the GUI interface, which is a
// place to start if you want to port bochs to a platform, for
// which there is no support for your native GUI, or if you want to compile
// bochs without any native GUI support (no output window or
// keyboard input will be possible).
// Look in 'x.cc', 'carbon.cc', and 'win32.cc' for specific
// implementations of this interface.  -Kevin


// ::SPECIFIC_INIT()
//
// Called from gui.cc, once upon program startup, to allow for the
// specific GUI code (X11, Win32, ...) to be initialized.
//
// argc, argv: these arguments can be used to initialize the GUI with
//     specific options (X11 options, Win32 options,...)
//
// headerbar_y:  A headerbar (toolbar) is display on the top of the
//     VGA window, showing floppy status, and other information.  It
//     always assumes the width of the current VGA mode width, but
//     it's height is defined by this parameter.

void bx_switch_gui_c::specific_init(int argc, char **argv, unsigned headerbar_y)
{
  BX_INFO(("switch gui module! hello!"));

  UNUSED(argc);
  UNUSED(argv);
  UNUSED(headerbar_y);

  UNUSED(bochs_icon_bits);  // global variable

  if (SIM->get_param_bool(BXPN_PRIVATE_COLORMAP)->get()) {
    BX_INFO(("private_colormap option ignored."));
  }

  clear_screen();
}


// ::HANDLE_EVENTS()
//
// Called periodically (every 1 virtual millisecond) so the
// the gui code can poll for keyboard, mouse, and other
// relevant events.

void bx_switch_gui_c::handle_events(void)
{
}


// ::FLUSH()
//
// Called periodically, requesting that the gui code flush all pending
// screen update requests.

void bx_switch_gui_c::flush(void)
{
}


// ::CLEAR_SCREEN()
//
// Called to request that the VGA region is cleared.  Don't
// clear the area that defines the headerbar.

void bx_switch_gui_c::clear_screen(void)
{
  u32 width, height;
  u8 *fb = gfxGetFramebuffer(NULL, NULL);
  memset(fb, 0, gfxGetFramebufferSize());
}


// ::TEXT_UPDATE()
//
// Called in a VGA text mode, to update the screen with
// new content.
//
// old_text: array of character/attributes making up the contents
//           of the screen from the last call.  See below
// new_text: array of character/attributes making up the current
//           contents, which should now be displayed.  See below
//
// format of old_text & new_text: each is tm_info->line_offset*text_rows
//     bytes long. Each character consists of 2 bytes.  The first by is
//     the character value, the second is the attribute byte.
//
// cursor_x: new x location of cursor
// cursor_y: new y location of cursor
// tm_info:  this structure contains information for additional
//           features in text mode (cursor shape, line offset,...)

void bx_switch_gui_c::text_update(Bit8u *old_text, Bit8u *new_text,
                      unsigned long cursor_x, unsigned long cursor_y,
                      bx_vga_tminfo_t *tm_info)
{
  // 0xRRGGBBAA is used for texture writes

  const u32 vga_colours[] =
  {
    0xff000000, // black #000000
    0xffaa0000, // blue #0000aa
    0xff00aa00, // green #00aa00
    0xffaaaa00, // cyan #00aaaa
    0xff0000aa, // red #aa0000
    0xffaa00aa, // magenta #aa00aa
    0xff0055aa, // brown #aa5500
    0xffaaaaaa, // grey #aaaaaa
    0xff555555, // dark grey #555555
    0xffff5500, // bright blue #5555ff
    0xff55ff55, // bright green #55ff55
    0xffffff55, // bright cyan #55ffff
    0xff5555ff, // bright red #ff5555
    0xffff55ff, // bright magenta #ff55ff
    0xff55ffff, // yellow #ffff55
    0xffffffff // white #ffffff
  };

  unsigned char *old_line, *new_line, *new_start;
  unsigned char cAttr;
  unsigned int hchars, rows, x, y;
  char ch;
  bx_bool force_update = 0;

  if(charmap_updated) {
    force_update = 1;
    charmap_updated = 0;
  }

  new_start = new_text;
  rows = guest_ychars;
  y = 0;
  do {
    hchars = guest_xchars;
    new_line = new_text;
    old_line = old_text;
    x = 0;
    do {
      if (force_update || (old_text[0] != new_text[0])
          || (old_text[1] != new_text[1])) {
        
        vga_fg = vga_colours[new_text[1] & 0xf];
        vga_bg = vga_colours[(new_text[1] & 0xf0) >> 4];

        ch = new_text[0];
        //if ((new_text[1] & 0x08) > 0) ch |= A_BOLD;
        //if ((new_text[1] & 0x80) > 0) ch |= A_BLINK;
        vga_draw_char(x, y, ch);
      }
      x++;
      new_text+=2;
      old_text+=2;
    } while (--hchars);
    y++;
    new_text = new_line + tm_info->line_offset;
    old_text = old_line + tm_info->line_offset;
  } while (--rows);

  if ((cursor_x<guest_xchars) && (cursor_y<guest_ychars)
      && (tm_info->cs_start <= tm_info->cs_end)) {
    if(cursor_x>0)
      cursor_x--;
    else {
      cursor_x=25-1;
      cursor_y--;
    }

    cAttr = new_start[cursor_y*tm_info->line_offset+cursor_x*2+1];
    vga_fg = vga_colours[cAttr & 0xf];
    vga_bg = vga_colours[(cAttr & 0xf0) >> 8];

    ch = new_start[cursor_y*tm_info->line_offset+cursor_x*2];
    vga_draw_char(x, y, ch);
    //curs_set(2);
  } else {
    //curs_set(0);
  }
}

bx_svga_tileinfo_t * bx_switch_gui_c::graphics_tile_info(bx_svga_tileinfo_t *info)
{
  // ok
  info->bpp = 32;
  info->pitch = 720 * 4;
  info->red_shift = 24;
  info->green_shift = 16;
  info->blue_shift = 8;
  info->red_mask =   0xff000000;
  info->green_mask = 0x00ff0000;
  info->blue_mask =  0x0000ff00;

  info->is_indexed = false;
  info->is_little_endian = 1;

  return info;
}

Bit8u * bx_switch_gui_c::graphics_tile_get(unsigned x, unsigned y, unsigned *w, unsigned *h)
{
 // Nothing to see here
  return NULL;
}

void bx_switch_gui_c::graphics_tile_update_in_place(unsigned x, unsigned y, unsigned w, unsigned h)
{
  UNUSED(x);
  UNUSED(y);
  UNUSED(w);
  UNUSED(h);

  // Just do nothing. It's fine. Really.
}

// ::GET_CLIPBOARD_TEXT()
//
// Called to get text from the GUI clipboard. Returns 1 if successful.

int bx_switch_gui_c::get_clipboard_text(Bit8u **bytes, Bit32s *nbytes)
{
  UNUSED(bytes);
  UNUSED(nbytes);
  return 0;
}


// ::SET_CLIPBOARD_TEXT()
//
// Called to copy the text screen contents to the GUI clipboard.
// Returns 1 if successful.

int bx_switch_gui_c::set_clipboard_text(char *text_snapshot, Bit32u len)
{
  UNUSED(text_snapshot);
  UNUSED(len);
  return 0;
}


// ::PALETTE_CHANGE()
//
// Allocate a color in the native GUI, for this color, and put
// it in the colormap location 'index'.
// returns: 0=no screen update needed (color map change has direct effect)
//          1=screen updated needed (redraw using current colormap)

bx_bool bx_switch_gui_c::palette_change(Bit8u index, Bit8u red, Bit8u green, Bit8u blue)
{
  UNUSED(index);
  UNUSED(red);
  UNUSED(green);
  UNUSED(blue);
  return(0);
}


// ::GRAPHICS_TILE_UPDATE()
//
// Called to request that a tile of graphics be drawn to the
// screen, since info in this region has changed.
//
// tile: array of 8bit values representing a block of pixels with
//       dimension equal to the 'x_tilesize' & 'y_tilesize' members.
//       Each value specifies an index into the
//       array of colors you allocated for ::palette_change()
// x0: x origin of tile
// y0: y origin of tile
//
// note: origin of tile and of window based on (0,0) being in the upper
//       left of the window.

void bx_switch_gui_c::graphics_tile_update(Bit8u *tile, unsigned x0, unsigned y0)
{
  UNUSED(tile);
  UNUSED(x0);
  UNUSED(y0);
}


// ::DIMENSION_UPDATE()
//
// Called when the VGA mode changes it's X,Y dimensions.
// Resize the window to this size, but you need to add on
// the height of the headerbar to the Y value.
//
// x: new VGA x size
// y: new VGA y size (add headerbar_y parameter from ::specific_init().
// fheight: new VGA character height in text mode
// fwidth : new VGA character width in text mode
// bpp : bits per pixel in graphics mode

void bx_switch_gui_c::dimension_update(unsigned x, unsigned y, unsigned fheight, unsigned fwidth, unsigned bpp)
{
  guest_textmode = (fheight > 0);
  guest_xres = x;
  guest_yres = y;
  guest_bpp = bpp;

  if(guest_textmode)
  {
    guest_xchars = x / fwidth;
    guest_ychars = y / fheight;
    /*screen_xscale = 1.0;
    screen_yscale = 1.0;
    pan_x = 0;
    pan_y = 0;*/
  }
  else
  {
    // TODO
  }

  BX_INFO(("mode switch to %i by %i at %i bpp, text mode: %i", x, y, bpp, (int)guest_textmode));
}


// ::CREATE_BITMAP()
//
// Create a monochrome bitmap of size 'xdim' by 'ydim', which will
// be drawn in the headerbar.  Return an integer ID to the bitmap,
// with which the bitmap can be referenced later.
//
// bmap: packed 8 pixels-per-byte bitmap.  The pixel order is:
//       bit0 is the left most pixel, bit7 is the right most pixel.
// xdim: x dimension of bitmap
// ydim: y dimension of bitmap

unsigned bx_switch_gui_c::create_bitmap(const unsigned char *bmap, unsigned xdim, unsigned ydim)
{
  UNUSED(bmap);
  UNUSED(xdim);
  UNUSED(ydim);
  return(0);
}


// ::HEADERBAR_BITMAP()
//
// Called to install a bitmap in the bochs headerbar (toolbar).
//
// bmap_id: will correspond to an ID returned from
//     ::create_bitmap().  'alignment' is either BX_GRAVITY_LEFT
//     or BX_GRAVITY_RIGHT, meaning install the bitmap in the next
//     available leftmost or rightmost space.
// alignment: is either BX_GRAVITY_LEFT or BX_GRAVITY_RIGHT,
//     meaning install the bitmap in the next
//     available leftmost or rightmost space.
// f: a 'C' function pointer to callback when the mouse is clicked in
//     the boundaries of this bitmap.

unsigned bx_switch_gui_c::headerbar_bitmap(unsigned bmap_id, unsigned alignment, void (*f)(void))
{
  UNUSED(bmap_id);
  UNUSED(alignment);
  UNUSED(f);
  return(0);
}


// ::SHOW_HEADERBAR()
//
// Show (redraw) the current headerbar, which is composed of
// currently installed bitmaps.

void bx_switch_gui_c::show_headerbar(void)
{
}


// ::REPLACE_BITMAP()
//
// Replace the bitmap installed in the headerbar ID slot 'hbar_id',
// with the one specified by 'bmap_id'.  'bmap_id' will have
// been generated by ::create_bitmap().  The old and new bitmap
// must be of the same size.  This allows the bitmap the user
// sees to change, when some action occurs.  For example when
// the user presses on the floppy icon, it then displays
// the ejected status.
//
// hbar_id: headerbar slot ID
// bmap_id: bitmap ID

void bx_switch_gui_c::replace_bitmap(unsigned hbar_id, unsigned bmap_id)
{
  UNUSED(hbar_id);
  UNUSED(bmap_id);
}


// ::EXIT()
//
// Called before bochs terminates, to allow for a graceful
// exit from the native GUI mechanism.

void bx_switch_gui_c::exit(void)
{
  BX_INFO(("bx_switch_gui_c::exit() not implemented yet."));
}


// ::MOUSE_ENABLED_CHANGED_SPECIFIC()
//
// Called whenever the mouse capture mode should be changed. It can change
// because of a gui event such as clicking on the mouse bitmap / button of
// the header / tool bar, toggle the mouse capture using the configured
// method with keyboard or mouse, or from the configuration interface.

void bx_switch_gui_c::mouse_enabled_changed_specific(bx_bool val)
{
}

void bx_switch_gui_c::vga_draw_char(int x, int y, char c)
{
  uint32_t bytes_per_char = 16;
  const bx_fontcharbitmap_t letter = bx_vgafont[c];
  uint32_t xp,yp = 0;

  u8 *fb = gfxGetFramebuffer(NULL, NULL);
  u32 *fb32 = (u32*)fb;

  for(xp=0; xp < 8; xp+=4)
  {
    for(yp=0 ; yp < 16; yp++)
    {
      uint32_t xx = (x * 8) + xp;
      uint32_t yy = (y * 16) + yp;

      int ind = gfxGetFramebufferDisplayOffset(xx, yy);
      for(int j = 0; j < 4; j++)
      {
        if(letter.data[yp] & (1 << (xp + j)))
        {
          fb32[ind + j] = vga_fg;
        }
        else
        {
          fb32[ind + j] = vga_bg;
        }
      }
    }
  }
}

#endif /* if BX_WITH_switch */
