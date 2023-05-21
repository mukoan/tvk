/**
 * @file   ThaiVirtualKeyboard.cc
 * @brief  Thai Virtual Keyboard Widget
 * @author Lyndon Hill
 * @date   2004.06.01 Incept
 *
    Copyright (C) 2004 Lyndon Hill

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */ 
 
#include "ThaiVirtualKeyboard.h" 

#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>
#include <QFontDialog>
#include <QSettings>
#include <QApplication>
#include <QScreen>
#include <QFontDatabase>
#include <QFont>
#include <QList>

#include <math.h>

#include "Multisize/enter_large.xpm"
#include "Multisize/enter_medium.xpm"
#include "Multisize/enter_small.xpm"
#include "Multisize/backspace_large.xpm"
#include "Multisize/backspace_medium.xpm"
#include "Multisize/backspace_small.xpm"
#include "Multisize/tab_large.xpm"
#include "Multisize/tab_medium.xpm"
#include "Multisize/tab_small.xpm"
#include "Multisize/shift_large.xpm"
#include "Multisize/shift_medium.xpm"
#include "Multisize/shift_small.xpm"
#include "Multisize/font_large.xpm"
#include "Multisize/font_medium.xpm"
#include "Multisize/font_small.xpm"

int tvk_keymap[75] = {
  239, 229,  47,  45, 192, 182, 216, 214, 164, 181, 168, 162, 170,  8,  0,
    9, 230, 228, 211, 190, 208, 209, 213, 195, 185, 194, 186, 197, 10, 10,
    3, 238, 191, 203, 161, 180, 224, 233, 232, 210, 202, 199, 167,  10, 10,
    1, 163, 188, 187, 225, 205, 212, 215, 183, 193, 227, 189,   2,  2,  0,
   32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32 };

int tvk_shifted_keymap[75] = {
  251,  43, 241, 242, 243, 244, 217, 223, 245, 246, 247, 248, 249,  8,  0,
    9, 240,  34, 174, 177, 184, 237, 234, 179, 207, 173, 176,  44, 10, 10,
    3, 250, 196, 166, 175, 226, 172, 231, 235, 201, 200, 171,  46,  10, 10, 
    1, 165,  40,  41, 169, 206, 218, 236,  63, 178, 204, 198,   2,  2,  0,
   32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32,  32, 32, 32 };
  
ThaiVirtualKeyboard::ThaiVirtualKeyboard(QWidget *parent) : QLabel(parent)
{
  actionKeySize = 0;  // temporary initialisation value

  // Format of the keyboard
  columns = 15;
  rows = 5;

  highDPI = false;

/*
  // Images for normal and shifted keymaps
  if(((QGuiApplication*)QCoreApplication::instance())->primaryScreen()->devicePixelRatio() >= 2)
  {
    highDPI = true;

    thekeyboard   = new QPixmap(this->width()*2, this->height()*2);
    shiftkeyboard = new QPixmap(this->width()*2, this->height()*2);
  }
  else
*/
  {
    thekeyboard   = new QPixmap(this->width(), this->height());
    shiftkeyboard = new QPixmap(this->width(), this->height());
  }

  // Allocate action keys
  pix_backspace_large  = new QPixmap(backspace_large);
  pix_backspace_medium = new QPixmap(backspace_medium);
  pix_backspace_small  = new QPixmap(backspace_small);
  pix_tab_large        = new QPixmap(tab_large);
  pix_tab_medium       = new QPixmap(tab_medium);
  pix_tab_small        = new QPixmap(tab_small);
  pix_enter_large      = new QPixmap(enter_large);
  pix_enter_medium     = new QPixmap(enter_medium);
  pix_enter_small      = new QPixmap(enter_small);
  pix_shift_large      = new QPixmap(shift_large);
  pix_shift_medium     = new QPixmap(shift_medium);
  pix_shift_small      = new QPixmap(shift_small);
  pix_font_large       = new QPixmap(font_large);
  pix_font_medium      = new QPixmap(font_medium);
  pix_font_small       = new QPixmap(font_small);

  shifted = false;
  keydown = false;

  setFocusPolicy(Qt::StrongFocus);

  QSettings settings("lyndonhill.com", "TVK");

  tvkFontName = settings.value("font/name", "Arial").toString();
  tvkFontSize = settings.value("font/size", 24).toInt();

  previousFontName = tvkFontName;
  previousFontSize = tvkFontSize;

  // Add dotted circle for NSM
#ifdef _WIN32
  addSpaceNSM = true;
#elif __APPLE__
  // Mac sometimes adds them, e.g. Lucida Grande, Rockwell, Thonburi, Verdana
  if(backupFontRenderer(tvkFontName))
    addSpaceNSM = false;
  else
    addSpaceNSM = true;
#else
  // Linux always add
  addSpaceNSM = true;
#endif

  // Get the size of the widget
  calculateTVKSize();

  // Set the initial image
  setPixmap(*thekeyboard);
}

// Close the widget
void ThaiVirtualKeyboard::closeEvent(QCloseEvent *e)
{
  // Write font settings

  QSettings settings("lyndonhill.com", "TVK"); 
  settings.setValue("font/name", tvkFontName);
  settings.setValue("font/size", tvkFontSize);

  e->accept();
}

// Where the key was pressed
void ThaiVirtualKeyboard::mousePressEvent(QMouseEvent *e)
{
  if(e->button() != Qt::LeftButton) return;
 
  QPixmap *currentkeyboard;

  if(shifted == true)
    currentkeyboard = shiftkeyboard;
  else
    currentkeyboard = thekeyboard;

  int actual_width  = currentkeyboard->width();
  int actual_height = currentkeyboard->height();

  float keywidth  = (float)(actual_width)/(float)(columns);
  float keyheight = (float)(actual_height)/(float)(rows);

  int tvk_code;

  // Get position of mouse
  QPoint keypos = e->pos();

  int press_row, press_column;

  press_row = keypos.y() / keyheight;
  if(press_row > 4) press_row = -1; // invalid row

  press_column = -1; // invalid key

  switch(press_row)
  {
    case 0:
    press_column = keypos.x() / keywidth;
    if(press_column >= 13)
    {
      press_column = 13; // delete
      highlightArea.setCoords(1+press_column*keywidth, 1, currentkeyboard->width(), keyheight);
    }
    else
      highlightArea.setCoords(1+press_column*keywidth, 1, (press_column+1)*keywidth, keyheight);
    break;

    case 1:
    press_column = 0;
    press_column += (keypos.x() - (int)(keywidth*0.5)) / keywidth;
    if(press_column == 0)
    {
      // tab key
      highlightArea.setCoords(1, keyheight, (int)(keywidth*1.5), keyheight*2);
    }
    else if(press_column >= 13)
    {
      press_column = 13; // enter
      highlightArea.setCoords((int)(keywidth*13.5)+1, keyheight+1, currentkeyboard->width(), keyheight*2+1);
    }
    else
      highlightArea.setCoords((int)(1+((press_column+0.5)*keywidth)), keyheight+1, (int)((press_column+1.5)*keywidth), keyheight*2);
    break;

    case 2:
    if(keypos.x() >= keywidth)
    {
      press_column = keypos.x() / keywidth;
      if(press_column >= 13)
      {
        press_column = 13; // enter
        highlightArea.setCoords((int)(keywidth*13.5)+1, keyheight+1, currentkeyboard->width(), keyheight*2+1);
      }
      else
        highlightArea.setCoords(keywidth*press_column+1, keyheight*2+1, keywidth*(press_column+1)+1, keyheight*3);
    }
    else
    {
      press_column = 0;
      highlightArea.setCoords(1, keyheight*2+1, keywidth-1, keyheight*3-1); // font
    }
    break;

    case 3:
    if(keypos.x() < keywidth*14)
    {
      press_column = 0;
      press_column += (keypos.x() - (int)(keywidth*0.5)) / keywidth;
      if(press_column == 0)
      {
        // left shift
        highlightArea.setCoords(1, keyheight*3+1, (int)(1.5*keywidth), keyheight*4);
      }
      else if(press_column >= 12)
      {
        press_column = 12; // right shift
        highlightArea.setCoords(1+(int)(12.5*keywidth), keyheight*3+1, 14*keywidth, keyheight*4);
      }
      else
      {
        highlightArea.setCoords((int)(1+((press_column+0.5)*keywidth)), keyheight*3+1, (int)((press_column+1.5)*keywidth), keyheight*4);
      }
    }
    break;

    case 4:
    if((keypos.x() > keywidth*4) && (keypos.x() < keywidth*11))
    {
      press_column = 0;
      highlightArea.setCoords(keywidth*4+1, keyheight*4+1, keywidth*11, keyheight*5-1); // space
    }
    break;
  }

  keyrow = press_row;
  keycol = press_column;

  // Key signal

  if((press_row == -1) || (press_column == -1))
    return;

  tvk_code = shifted == false ? tvk_keymap[press_row*columns+press_column] :
                               tvk_shifted_keymap[press_row*columns+press_column];

  if(tvk_code > 3)
    emit KeyPressed(tvk_code);

  keydown = true;
  update();
}

// Where the key was released
void ThaiVirtualKeyboard::mouseReleaseEvent(QMouseEvent *e)
{
  if(e->button() != Qt::LeftButton) return;

  bool originalshift = shifted;
  QPixmap *keyboard;

  // Check if shift key was pressed - this way keyboard only changes
  // when shift key is released.
  if((keyrow == 3) && ((keycol == 0) || (keycol == 12)))
  {
    shifted = !shifted;
  }
  else if((keyrow == 2) && (keycol == 0))
  {
    bool ok;
#ifdef __APPLE__
    QFont font = QFontDialog::getFont(&ok, QFont(tvkFontName, tvkFontSize), this, "Choose Keyboard Font", QFontDialog::DontUseNativeDialog);
#else
    QFont font = QFontDialog::getFont(&ok, QFont(tvkFontName, tvkFontSize), this, "Choose Keyboard Font");
#endif
    if(ok)
    {
      tvkFontName = font.family();
      tvkFontSize = font.pointSize();
      calculateTVKSize();

#if __APPLE__
      if(backupFontRenderer(tvkFontName))
        addSpaceNSM = false;
      else
        addSpaceNSM = true;
#endif

      drawKeyboard(true);
      drawKeyboard(false);
    }
  }
  else
    shifted = false;

  keyboard = (shifted == true) ? shiftkeyboard : thekeyboard;

  if(originalshift != shifted)
  {
    // Redraw keyboard if size changed since last time it was drawn
    if((this->width() != keyboard->width()) || (this->height() != keyboard->height()))
    {
      *keyboard = keyboard->scaled(this->width()*keyboard->devicePixelRatio(), this->height()*keyboard->devicePixelRatio());

      drawKeyboard(shifted);
    }
  }

  keydown = false;
  update();
}

// Draw the keyboard
void ThaiVirtualKeyboard::drawKeyboard(bool shiftengage)
{
  QPixmap *keyboard;
  int *selectedkeymap;

  if(shiftengage == true)
  {
    keyboard = shiftkeyboard;
    selectedkeymap = tvk_shifted_keymap;
  }
  else
  {
    keyboard = thekeyboard;
    selectedkeymap = tvk_keymap;
  }

  keyboard->fill();
 
  QPixmap *pix_backspace, *pix_tab, *pix_enter, *pix_shift, *pix_font;
 
  int a, b;
  int tisvalue;
  QChar keycap;
  QString compoundcap;

  // Get size of standard key

  int actual_width  = keyboard->width();
  int actual_height = keyboard->height();

  float keywidth  = (float)(actual_width)/(float)(columns);
  float keyheight = (float)(actual_height)/(float)(rows);

  // select size of action keys
  switch(actionKeySize)
  {
    case 0:
    pix_backspace = pix_backspace_small;
    pix_tab       = pix_tab_small;
    pix_enter     = pix_enter_small;
    pix_shift     = pix_shift_small;
    pix_font      = pix_font_small;
    break;

    default:
    case 1:
    pix_backspace = pix_backspace_medium;
    pix_tab       = pix_tab_medium;
    pix_enter     = pix_enter_medium;
    pix_shift     = pix_shift_medium;
    pix_font      = pix_font_medium;
    break;

    case 2:
    pix_backspace = pix_backspace_large;
    pix_tab       = pix_tab_large;
    pix_enter     = pix_enter_large;
    pix_shift     = pix_shift_large;
    pix_font      = pix_font_large;
    break;
  }

  // Get size of keyboard image
  int kbwidth  = actual_width;
  int kbheight = actual_height;

  QPainter *mypaint = new QPainter();
  mypaint->begin(keyboard);
  mypaint->setPen(QColor(0,0,0));
  mypaint->setFont(QFont(tvkFontName, tvkFontSize));

  // Draw outline
  mypaint->drawLine(0, 0, kbwidth, 0);
  mypaint->drawLine(0, 0, 0, kbheight);
  mypaint->drawLine(0, kbheight, kbwidth, kbheight);
  mypaint->drawLine(kbwidth, 0, kbwidth, kbheight);

  // Draw rows
  mypaint->drawLine(0, keyheight, kbwidth, keyheight);
  mypaint->drawLine(0, keyheight*2, (int)(keywidth*13.5), keyheight*2);
  mypaint->drawLine(0, keyheight*3, kbwidth, keyheight*3);
  mypaint->drawLine(0, keyheight*4, keywidth*14, keyheight*4);

  // Draw columns

  for(a = 0; a < 14; a++)
    mypaint->drawLine(a*keywidth, 0, a*keywidth, keyheight);

  for(a = 0; a < 13; a++)
    mypaint->drawLine((int)((a+1.5)*keywidth), keyheight, (int)((a+1.5)*keywidth), keyheight*2);

  for(a = 0; a < 13; a++)
    mypaint->drawLine((a+1)*keywidth, keyheight*2, (a+1)*keywidth, keyheight*3);
  
  for(a = 0; a < 12; a++)
    mypaint->drawLine((int)((a+1.5)*keywidth), keyheight*3, (int)((a+1.5)*keywidth), keyheight*4);

  mypaint->drawLine(keywidth*14, keyheight*3, keywidth*14, keyheight*4);
  mypaint->drawLine(keywidth*4, keyheight*4, keywidth*4, kbheight);
  mypaint->drawLine(keywidth*11, keyheight*4, keywidth*11, kbheight);

  // Draw keycaps

  b = keyheight/2; // middle of first row

  QRect boundingbox;

  for(a = 0; a < 13; a++)
  {
    // compoundcap.clear();
    compoundcap = "";

    tisvalue = selectedkeymap[a];
    if(addSpaceNSM)
    {
      if(tisvalue > 211 && tisvalue < 219) compoundcap = QChar(0x25cc);
    }

    if(tisvalue > 127) tisvalue = tisvalue - 0xa0 + 0xe00; // convert to Unicode
    keycap = QChar(tisvalue);

    compoundcap += keycap;
    mypaint->drawText(QRect(keywidth*a, 0, keywidth, keyheight), Qt::AlignCenter, compoundcap);
  }

  b = (int)(keyheight*1.5); // middle of second row

  for(a = 1; a < 13; a++)
  {
    compoundcap = "";

    tisvalue = selectedkeymap[a+15];

    if(addSpaceNSM)
    {
#ifdef __linux
      // Nasty hack for SARA AM (211). Should not need this ifdef but Linux font renderer always
      // adds 0x25cc to SARA AM.
      if(tisvalue == 209 || (tisvalue > 211 && tisvalue < 219) || (tisvalue == 234) || (tisvalue == 237)) compoundcap = QChar(0x25cc);
#else
      if(tisvalue == 209 || (tisvalue >= 211 && tisvalue < 219) || (tisvalue == 234) || (tisvalue == 237)) compoundcap = QChar(0x25cc);
#endif
    }

    if(tisvalue > 127) tisvalue = tisvalue - 0xa0 + 0xe00; // convert to Unicode
    keycap = QChar(tisvalue);

    compoundcap += keycap;
    mypaint->drawText(QRect(keywidth*(a+0.5), keyheight, keywidth, keyheight), Qt::AlignCenter, compoundcap);
  }

  b = (int)(keyheight*2.5); // middle of third row

  for(a = 1; a < 13; a++)
  {
    compoundcap = "";

    tisvalue = selectedkeymap[a+30];

    if(addSpaceNSM)
    {
      if(tisvalue > 230 && tisvalue < 239) compoundcap = QChar(0x25cc);
    }

    if(tisvalue > 127) tisvalue = tisvalue - 0xa0 + 0xe00; // convert to Unicode
    keycap = QChar(tisvalue);

    compoundcap += keycap;
    mypaint->drawText(QRect(keywidth*a, keyheight*2, keywidth, keyheight), Qt::AlignCenter, compoundcap);
  }

  b = (int)(keyheight*3.5);

  for(a = 1; a < 12; a++)
  {
    compoundcap = "";

    tisvalue = selectedkeymap[a+45];

    if(addSpaceNSM)
    {
      if((tisvalue > 211 && tisvalue < 219) || (tisvalue > 230 && tisvalue < 239)) compoundcap = QChar(0x25cc);
    }

    if(tisvalue > 127) tisvalue = tisvalue - 0xa0 + 0xe00; // convert to Unicode
    keycap = QChar(tisvalue);

    compoundcap += keycap;
    mypaint->drawText(QRect(keywidth*(a+0.5), keyheight*3, keywidth, keyheight), Qt::AlignCenter, compoundcap);
  }

  // Draw action keys
  mypaint->drawPixmap((int)(keywidth*14-(pix_backspace->width()/2)), (int)(keyheight*0.5-(pix_backspace->height()/2)), *pix_backspace);
  mypaint->drawPixmap((int)(keywidth*0.75-(pix_tab->width()/2)), (int)(keyheight*1.5-(pix_tab->height()/2)), *pix_tab);
  mypaint->drawPixmap((int)(keywidth*14.25-(pix_enter->width()/2)), (int)(keyheight*2-(pix_enter->height()/2)), *pix_enter);
  mypaint->drawPixmap((int)(keywidth*0.5-(pix_font->width()/2)), (int)(keyheight*2.5-(pix_font->height()/2)), *pix_font);

  // Fill in unused areas
  int row5height = kbheight-1 - (int)(keyheight*4);
  int spaceright = kbwidth-1 - (int)(keywidth*11);
  int shiftright = kbwidth-1 - (int)(keywidth*14);

  mypaint->fillRect(keywidth*14+1, keyheight*3+1, shiftright, keyheight+1, QColor(64,64,64));
  mypaint->fillRect(1, keyheight*4+1, keywidth*4-1, row5height, QColor(64,64,64));
  mypaint->fillRect(keywidth*11+1, keyheight*4+1, spaceright, row5height, QColor(64,64,64));

  // Highlight shift keys
  QPixmap hshift = *pix_shift;
  QRgb pel;

  if(shiftengage == true)
  {
    QImage inverted = hshift.toImage();

    for(b = 0; b < inverted.height(); b++)
    {
      for(a = 0; a < inverted.width(); a++)
      {
        pel = inverted.pixel(a, b);

        int inverse = qRed(pel);
        inverted.setPixel(a, b, qRgb(255-inverse, 255-inverse, 255-inverse));
      }
    }

    mypaint->fillRect(1, keyheight*3+1, keywidth*1.5, keyheight, QColor(0,0,0));
    mypaint->fillRect(keywidth*12.5+1, keyheight*3+1, keywidth*1.5, keyheight, QColor(0,0,0));

    hshift = QPixmap::fromImage(inverted);
  }

  mypaint->drawPixmap((int)(keywidth*0.75-(hshift.width()/2)), (int)(keyheight*3.5-(hshift.height()/2)), hshift);
  mypaint->drawPixmap((int)(keywidth*13.25-(hshift.width()/2)), (int)(keyheight*3.5-(hshift.height()/2)), hshift);

  mypaint->end();

/*
  // TODO for Retina
  if(highDPI)
  {
    thekeyboard->setDevicePixelRatio(2.0);
    shiftkeyboard->setDevicePixelRatio(2.0);
  }
*/
}

// The keyboard was resized
void ThaiVirtualKeyboard::resizeEvent(QResizeEvent *)
{
  QPixmap *keyboard;

  if(shifted == true)
    keyboard = shiftkeyboard;
  else
    keyboard = thekeyboard;

  *keyboard = keyboard->scaled(this->width()*keyboard->devicePixelRatio(), this->height()*keyboard->devicePixelRatio());

  drawKeyboard(shifted);

  update();
}

// Repaint the widget
void ThaiVirtualKeyboard::paintEvent(QPaintEvent *p)
{
  QRegion refreshregion = p->region();
  QPainter qp(this);

  if(!refreshregion.isEmpty())
  {
    if(shifted == true)
      qp.drawPixmap(0, 0, *shiftkeyboard);
    else
      qp.drawPixmap(0, 0, *thekeyboard);
  }

  // Paint highlighted keys
  int *selectedkeymap;
  selectedkeymap = (shifted == true) ? tvk_shifted_keymap : tvk_keymap;

  QString compoundcap;
  QChar keycap;

  QImage inverted;
  QPixmap action, *shift, *tab, *backspace, *enter, *font;
  QRgb pel;
  int px, py;

/*
  int keywidth  = this->width()/columns;
  int keyheight = this->height()/rows;
*/

  if(actionKeySize == 2)
  {
    backspace = pix_backspace_large;
    tab       = pix_tab_large;
    enter     = pix_enter_large;
    shift     = pix_shift_large;
    font      = pix_font_large;
  }
  else if(actionKeySize == 1)
  {
    backspace = pix_backspace_medium;
    tab       = pix_tab_medium;
    enter     = pix_enter_medium;
    shift     = pix_shift_medium;
    font      = pix_font_medium;
  }
  else
  {
    backspace = pix_backspace_small;
    tab       = pix_tab_small;
    enter     = pix_enter_small;
    shift     = pix_shift_small;
    font      = pix_font_small;
  }

  px = 0; py = 0;
  
  if(keydown == true)
  {
    qp.fillRect(highlightArea, QColor(0,0,0));

    compoundcap = "";
    int tisvalue = selectedkeymap[keyrow*columns+keycol];

    if(tisvalue > 32)
    {
      qp.setPen(QColor(255,255,255));
      qp.setFont(QFont(tvkFontName, tvkFontSize));

      if(tisvalue > 127)
      {
        if(addSpaceNSM)
        {
          if((tisvalue == 209) || ((tisvalue >= 211) && (tisvalue < 219)) || ((tisvalue > 230) && (tisvalue < 239)))
          {
            // Add space to NSM
            compoundcap = QChar(0x25cc);
          }
        }

        tisvalue = tisvalue - 0xa0 + 0xe00; // convert to Unicode
      }

      keycap = QChar(tisvalue);

      compoundcap += keycap;
      qp.drawText(highlightArea, Qt::AlignCenter, compoundcap);
    }
    else
    {
      // Draw action keys : get positioning of graphic
      switch(tisvalue)
      {
        case 32: // space
        break;   // do nothing - no graphic

        case 1: // left shift
        action = *shift;
        px = highlightArea.x() + highlightArea.width()/2 - action.width()/2;
        py = highlightArea.y() + highlightArea.height()/2 - action.height()/2;
        break;

        case 2: // right shift
        action = *shift;
        px = highlightArea.x() + highlightArea.width()/2 - action.width()/2;
        py = highlightArea.y() + highlightArea.height()/2 - action.height()/2;
        break;

        case 3: // font selector
        action = *font;
        px = highlightArea.x() + highlightArea.width()/2 - action.width()/2;
        py = highlightArea.y() + highlightArea.height()/2 - action.height()/2;
        break;

        case 8: // delete
        action = *backspace;
        px = highlightArea.x() + highlightArea.width()/2 - action.width()/2;
        py = highlightArea.y() + highlightArea.height()/2 - action.height()/2;
        break;

        case 9: // tab
        action = *tab;
        px = highlightArea.x() + highlightArea.width()/2 - action.width()/2;
        py = highlightArea.y() + highlightArea.height()/2 - action.height()/2;
        break;

        case 10: // enter
        qp.fillRect((this->width()*(13.0/columns))+1, this->height()*(2.0/rows)+1, this->width(), this->height()/rows, QColor(0,0,0));
        action = *enter;
        px = highlightArea.x() + highlightArea.width()/2 - action.width()/2;
        py = highlightArea.y() + highlightArea.height() - action.height()/2;
        break;
      }

      if((tisvalue < 32) && (tisvalue > 0))
      {
        inverted = action.toImage();

        int a, b;
        for(b = 0; b < inverted.height(); b++)
        {
          for(a = 0; a < inverted.width(); a++)
          {
            pel = inverted.pixel(a, b);

            int inverse = qRed(pel);
            inverted.setPixel(a, b, qRgb(255-inverse, 255-inverse, 255-inverse));
          }
        }

        action = QPixmap::fromImage(inverted);

        qp.drawPixmap(px-1, py-1, action);
      }
    }
  }

  qp.end();
}

// Pass through
void ThaiVirtualKeyboard::keyPressEvent(QKeyEvent *e)
{
#if QT_VERSION > 0x040000
  if((e->key() == Qt::Key_0) && (e->modifiers() == Qt::ControlModifier))
#else
  if((e->key() == Qt::Key_0) && (e->state() == Qt::ControlButton))
#endif
  {
    tvkFontName = previousFontName;
    tvkFontSize = previousFontSize;

#if __APPLE__
  if(backupFontRenderer(tvkFontName))
    addSpaceNSM = false;
  else
    addSpaceNSM = true;
#endif

    calculateTVKSize();
  }
  else if((e->key() == Qt::Key_9) && (e->modifiers() == Qt::ControlModifier))
  {
    tvkFontSize++;
    calculateTVKSize();
  }
  else if((e->key() == Qt::Key_8) && (e->modifiers() == Qt::ControlModifier))
  {
    tvkFontSize--;
    calculateTVKSize();
  }
  else
    emit PassThroughkeyPressEvent(e);
}

// Calculate and set minimum size
void ThaiVirtualKeyboard::calculateTVKSize()
{
  int border = 2;
  int glyph_width, glyph_height;
  int min_width, min_height;
  int khomut   = 0x0e5b;
  int nine     = 0x0e59;
  int ying     = 0x0e0d;
  int jula     = 0x0e2c;
  int am       = 0x0e33;
  int maimalai = 0x0e44;

  QChar testwide = QChar(khomut);
  QString testhigh;

  testhigh.append(QChar(0x0e44));
  testhigh.append(QChar(0x0e1b));
  testhigh.append(QChar(0x0e26));
  testhigh.append(QChar(0x0e21));
  testhigh.append(QChar(0x0e35));
  testhigh.append(QChar(0x0e49));
  testhigh.append(QChar(0x0e1a));
  testhigh.append(QChar(0x0e39));

  QFont f(tvkFontName, tvkFontSize);
  QFontMetrics fm(f);
  glyph_height = fm.boundingRect(testhigh).height();

  // Test several characters to find widest
  QString testam;
  if(addSpaceNSM)
    testam.append(QChar(0x25cc));
  testam.append(QChar(am));

  glyph_width  = fm.boundingRect(testwide).width();

  int test_glyph_width = fm.boundingRect(QChar(nine)).width();
  if(test_glyph_width > glyph_width) glyph_width = test_glyph_width;

  test_glyph_width = fm.boundingRect(QChar(ying)).width();
  if(test_glyph_width > glyph_width) glyph_width = test_glyph_width;

  test_glyph_width = fm.boundingRect(QChar(jula)).width();
  if(test_glyph_width > glyph_width) glyph_width = test_glyph_width;

  test_glyph_width = fm.boundingRect(testam).width();
  if(test_glyph_width > glyph_width) glyph_width = test_glyph_width;

  test_glyph_width = fm.boundingRect(QChar(maimalai)).width();
  if(test_glyph_width > glyph_width) glyph_width = test_glyph_width;

  if((glyph_width > 36) && (glyph_height > 36))
  {
    border = 8;
    actionKeySize = 2;
  }
  else if((glyph_width > 20) && (glyph_height > 20))
  {
    border = 4;
    actionKeySize = 1;
  }
  else
  {
    border = 2;
    actionKeySize = 0;
  }

  min_height = glyph_height*rows + border*rows*2;
  min_width  = glyph_width*columns + border*columns*2;

  this->setMinimumSize(min_width, min_height);
  this->resize(min_width, min_height);

  // Make sure TVK stays on screen!
  if((this->pos().x() < 0) || (this->pos().y() < 0))
    this->move(0,0);
}

// Return true if font family doesn't support Thai
bool ThaiVirtualKeyboard::backupFontRenderer(const QString &family) const
{
  QFontDatabase qdb;
  QList<QFontDatabase::WritingSystem> sys = qdb.writingSystems(family);

  QListIterator<QFontDatabase::WritingSystem> it(sys);
  while(it.hasNext())
  {
    if(it.next() == QFontDatabase::Thai)
    {
      // Font has Thai glyphs
      return(false);
    }
  }

  // Font does not have Thai glyphs
  return(true);
}

