/**
 * @file   ThaiVirtualKeyboard.cc
 * @brief  Thai Virtual Keyboard Widget
 * @author Lyndon Hill
 * @date   2009.12.16 Modified settings groups.
 *         2008.05.10 Added spaces to NSM on shifted keyboard.
 *         2008.04.22 All recent changes QT3 compatible. 
 *         2008.04.20 Added multiple size action keys, font action key. Ignore RMB.
 *         2008.04.14 Added font dialogue.
 *         2008.04.11 Use text for keys instead of images.
 *         2008.04.07 Use table for keymap.
 *         2006.05.04 Changed HighlightKey to use fromImage() properly - keyboard was not set properly.
 *         2006.04.06 QT4-ified.
 *         2004.07.30 Signals added.
 *         2004.07.13 Added yamakan and angkhanku.
 *         2004.07.11 Highlighting all keys.
 *         2004.06.20 Set minimum size.
 *         2004.06.01 Incept.
 */ 
 
#include "ThaiVirtualKeyboard.h" 

#if QT_VERSION > 0x040000
 #include <QPainter>
 #include <QPixmap>
 #include <QImage>
 #include <QMouseEvent>
 #include <QFontDialog>
 #include <QSettings>
#else
 #include <qpainter.h>
 #include <qpixmap.h>
 #include <qimage.h>
 #include <qfontdialog.h>
 #include <qsettings.h>
#endif

#include <iostream>
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
  
// using namespace std;

ThaiVirtualKeyboard::ThaiVirtualKeyboard(QWidget *parent) : QLabel(parent)
{
  // Format of the keyboard
  columns = 15;
  rows = 5;

  // Images for normal and shifted keymaps
  thekeyboard   = new QPixmap(this->width(), this->height());
  shiftkeyboard = new QPixmap(this->width(), this->height());

  // Allocate action keys
  pbackspace_large = new QPixmap(backspace_large);
  pbackspace_medium = new QPixmap(backspace_medium);
  pbackspace_small = new QPixmap(backspace_small);
  ptab_large = new QPixmap(tab_large);
  ptab_medium = new QPixmap(tab_medium);
  ptab_small = new QPixmap(tab_small);
  penter_large = new QPixmap(enter_large);
  penter_medium = new QPixmap(enter_medium);
  penter_small = new QPixmap(enter_small);
  pshift_large = new QPixmap(shift_large);
  pshift_medium = new QPixmap(shift_medium);
  pshift_small = new QPixmap(shift_small);
  pfont_large = new QPixmap(font_large);
  pfont_medium = new QPixmap(font_medium);
  pfont_small = new QPixmap(font_small);

  shifted = false;
  keydown = false;

#if QT_VERSION > 0x040000
  setFocusPolicy(Qt::StrongFocus);

  QSettings settings("lyndonhill.com", "TVK");

  tvk_font = settings.value("font/name", "Loma").toString();
  tvk_font_size = settings.value("font/size", 16).toInt();

#else
  QSettings settings;
  settings.setPath("lyndonhill.com", "TVK");

  tvk_font = settings.readEntry("/font/name", "Loma");
  tvk_font_size = settings.readNumEntry("/font/size", 16);
#endif

  previous_font = tvk_font;
  previous_font_size = tvk_font_size;

  // Get the size of the widget
  CalculateTVKSize();

  // Set the initial image
  setPixmap(*thekeyboard);
}

// Close the widget
void ThaiVirtualKeyboard::closeEvent(QCloseEvent *e)
{
  // Write font settings

#if QT_VERSION > 0x040000
  QSettings settings("lyndonhill.com", "TVK"); 
  settings.setValue("font/name", tvk_font);
  settings.setValue("font/size", tvk_font_size);
#else
  QSettings settings;
  settings.setPath("lyndonhill.com", "TVK");
  settings.writeEntry("/font/name", tvk_font);
  settings.writeEntry("/font/size", tvk_font_size);
#endif

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

  int keywidth = currentkeyboard->width()/columns;
  int keyheight = currentkeyboard->height()/rows;

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
      highlightarea.setCoords(1+press_column*keywidth, 1, currentkeyboard->width(), keyheight); 
    }
    else
      highlightarea.setCoords(1+press_column*keywidth, 1, (press_column+1)*keywidth, keyheight); 
    break;
    
    case 1:
    press_column = 0;
    press_column += (keypos.x() - (int)(keywidth*0.5)) / keywidth; 
    if(press_column == 0)
    {
      // tab key
      highlightarea.setCoords(1, keyheight, (int)(keywidth*1.5), keyheight*2); 
    }
    else if(press_column >= 13)
    {
      press_column = 13; // enter
      highlightarea.setCoords((int)(keywidth*13.5)+1, keyheight+1, currentkeyboard->width(), keyheight*2+1); 
//      highlightarea.setCoords(keywidth*13+1, keyheight*2+1, thekeyboard->width(), keyheight*3); 
    }
    else
      highlightarea.setCoords((int)(1+((press_column+0.5)*keywidth)), keyheight+1, (int)((press_column+1.5)*keywidth), keyheight*2); 
    break;

    case 2:
    if(keypos.x() >= keywidth)
    {
      press_column = keypos.x() / keywidth; 
      if(press_column >= 13)
      {
        press_column = 13; // enter
        highlightarea.setCoords((int)(keywidth*13.5)+1, keyheight+1, currentkeyboard->width(), keyheight*2+1); 
//        highlightarea.setCoords(keywidth*13+1, keyheight*2+1, thekeyboard->width(), keyheight*3); 
      }
      else
        highlightarea.setCoords(keywidth*press_column+1, keyheight*2+1, keywidth*(press_column+1)+1, keyheight*3); 
    }
    else
    {
      press_column = 0;
      highlightarea.setCoords(1, keyheight*2+1, keywidth-1, keyheight*3-1); // font  
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
        highlightarea.setCoords(1, keyheight*3+1, (int)(1.5*keywidth), keyheight*4); 
      }
      else if(press_column >= 12) 
      {
        press_column = 12; // right shift
        highlightarea.setCoords(1+(int)(12.5*keywidth), keyheight*3+1, 14*keywidth, keyheight*4); 
      }
      else
      {
        highlightarea.setCoords((int)(1+((press_column+0.5)*keywidth)), keyheight*3+1, (int)((press_column+1.5)*keywidth), keyheight*4); 
      }
    }
    break;

    case 4:
    if((keypos.x() > keywidth*5) && (keypos.x() < keywidth*10))
    {
      press_column = 0;
      highlightarea.setCoords(keywidth*5+1, keyheight*4+1, keywidth*10, keyheight*5-1); // space
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
    QFont font = QFontDialog::getFont(&ok, QFont(tvk_font, tvk_font_size), this, "Choose Keyboard Font", QFontDialog::DontUseNativeDialog);
#else
    QFont font = QFontDialog::getFont(&ok, QFont(tvk_font, tvk_font_size), this, "Choose Keyboard Font");
#endif
    if(ok)
    {
      tvk_font = font.family();
      tvk_font_size = font.pointSize();
      CalculateTVKSize();

      DrawKeyboard(true);
      DrawKeyboard(false);
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
#if QT_VERSION > 0x040000
      *keyboard = keyboard->scaled(this->width(), this->height());
#else
      keyboard->resize(this->width(), this->height());
#endif

      DrawKeyboard(shifted);
    }

  }

  keydown = false;
  update();
}

// Draw the keyboard
void ThaiVirtualKeyboard::DrawKeyboard(bool shiftengage)
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
 
  QPixmap *pbackspace, *ptab, *penter, *pshift, *pfont;
 
  int a, b;
  int tisvalue;
  QChar keycap;
  QString compoundcap;

  // Get size of standard key
  double keywidth  = (double)(keyboard->width())/(double)(columns);
  double keyheight = (double)(keyboard->height())/(double)(rows);

  // select size of action keys
  if((keywidth > 36) && (keyheight > 36))
  {
    // large
    pbackspace = pbackspace_large;
    ptab = ptab_large;
    penter = penter_large;
    pshift = pshift_large;
    pfont = pfont_large;
  }
  else if((keywidth > 20) && (keyheight > 20))
  {
    // medium
    pbackspace = pbackspace_medium;
    ptab = ptab_medium;
    penter = penter_medium;
    pshift = pshift_medium;
    pfont = pfont_medium;
  }
  else
  {
    // small
    pbackspace = pbackspace_small;
    ptab = ptab_small;
    penter = penter_small;
    pshift = pshift_small;
    pfont = pfont_small;
  }

  // Get size of keyboard image
  int kbwidth = keyboard->width()-1;
  int kbheight = keyboard->height()-1;

  QPainter *mypaint = new QPainter();
  mypaint->begin(keyboard);
  mypaint->setPen(QColor(0,0,0));
  mypaint->setFont(QFont(tvk_font, tvk_font_size));

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
  mypaint->drawLine(keywidth*5, keyheight*4, keywidth*5, kbheight);
  mypaint->drawLine(keywidth*10, keyheight*4, keywidth*10, kbheight);
 
  // Draw keycaps
 
  b = keyheight/2; // middle of first row

  QRect boundingbox;

  for(a = 0; a < 13; a++)
  {
    // compoundcap.clear();
    compoundcap = "";

    tisvalue = selectedkeymap[a];
    if(tisvalue > 211 && tisvalue < 219) compoundcap = QChar(0x25cc);

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

    if(tisvalue == 209 || (tisvalue >=  211 && tisvalue < 219) || (tisvalue == 234) || (tisvalue == 237)) compoundcap = QChar(0x25cc);

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
    if(tisvalue > 230 && tisvalue < 239) compoundcap = QChar(0x25cc);

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
    if((tisvalue > 211 && tisvalue < 219) || (tisvalue > 230 && tisvalue < 239)) compoundcap = QChar(0x25cc);

    if(tisvalue > 127) tisvalue = tisvalue - 0xa0 + 0xe00; // convert to Unicode
    keycap = QChar(tisvalue);

    compoundcap += keycap;
    mypaint->drawText(QRect(keywidth*(a+0.5), keyheight*3, keywidth, keyheight), Qt::AlignCenter, compoundcap);
  }

  // Draw action keys
  mypaint->drawPixmap((int)(keywidth*14-(pbackspace->width()/2)), (int)(keyheight*0.5-(pbackspace->height()/2)), *pbackspace);
  mypaint->drawPixmap((int)(keywidth*0.75-(ptab->width()/2)), (int)(keyheight*1.5-(ptab->height()/2)), *ptab);
  mypaint->drawPixmap((int)(keywidth*14.25-(penter->width()/2)), (int)(keyheight*2-(penter->height()/2)), *penter);
  mypaint->drawPixmap((int)(keywidth*0.5-(pfont->width()/2)), (int)(keyheight*2.5-(pfont->height()/2)), *pfont);
//  mypaint->drawPixmap((int)(keywidth*0.75-(QPixmap(shift).width()/2)), (int)(keyheight*3.5-(QPixmap(shift).height()/2)), QPixmap(shift));
//  mypaint->drawPixmap((int)(keywidth*13.25-(QPixmap(shift).width()/2)), (int)(keyheight*3.5-(QPixmap(shift).height()/2)), QPixmap(shift));

  // Fill in unused areas
  // mypaint->fillRect(1, keyheight*2+1, keywidth*1-1, keyheight-1, QColor(64,64,64));
  int row5height = kbheight-1 - (int)(keyheight*4);
  int spaceright = kbwidth-1 - (int)(keywidth*10);
  int shiftright = kbwidth-1 - (int)(keywidth*14);

  mypaint->fillRect(keywidth*14+1, keyheight*3+1, shiftright, keyheight+1, QColor(64,64,64));
  mypaint->fillRect(1, keyheight*4+1, keywidth*5-1, row5height, QColor(64,64,64));
  mypaint->fillRect(keywidth*10+1, keyheight*4+1, spaceright, row5height, QColor(64,64,64));

//  std::cerr << "Widget dimensions: " << this->width() << "x" << this->height() << " keys: " << keywidth << "x" << keyheight << "\n";

  // Highlight shift keys
  QPixmap hshift = *pshift;
  QRgb pel;
  int inverse;

  if(shiftengage == true)
  {
#if QT_VERSION > 0x040000
    QImage inverted = hshift.toImage();
#else
    QImage inverted = hshift.convertToImage();
#endif

    for(b = 0; b < inverted.height(); b++)
    {
      for(a = 0; a < inverted.width(); a++)
      {
        pel = inverted.pixel(a, b);
 
        inverse = qRed(pel);
        inverted.setPixel(a, b, qRgb(255-inverse, 255-inverse, 255-inverse));
      }
    }

    mypaint->fillRect(1, keyheight*3+1, keywidth*1.5, keyheight, QColor(0,0,0));
    mypaint->fillRect(keywidth*12.5+1, keyheight*3+1, keywidth*1.5, keyheight, QColor(0,0,0));

#if QT_VERSION > 0x040000
    hshift = QPixmap::fromImage(inverted);
#else
    hshift.convertFromImage(inverted);
#endif
  }

  mypaint->drawPixmap((int)(keywidth*0.75-(hshift.width()/2)), (int)(keyheight*3.5-(hshift.height()/2)), hshift);
  mypaint->drawPixmap((int)(keywidth*13.25-(hshift.width()/2)), (int)(keyheight*3.5-(hshift.height()/2)), hshift);

  mypaint->end();
}

// The keyboard was resized
void ThaiVirtualKeyboard::resizeEvent(QResizeEvent *)
{
  QPixmap *keyboard;

  if(shifted == true)
    keyboard = shiftkeyboard;
  else
    keyboard = thekeyboard;

#if QT_VERSION > 0x040000
  *keyboard = keyboard->scaled(this->width(), this->height());
#else
  keyboard->resize(this->width(), this->height());
#endif

  DrawKeyboard(shifted);

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
  int tisvalue;

  int keywidth, keyheight;
  QImage inverted; 
  QPixmap action, *shift, *tab, *backspace, *enter, *font;
  QRgb pel;
  int inverse;
  int a, b;
  int px, py;

  keywidth = this->width()/columns;
  keyheight = this->height()/rows;

  if((keywidth > 36) && (keyheight > 36))
  {
    backspace = pbackspace_large;
    tab = ptab_large;
    enter = penter_large;
    shift = pshift_large;
    font = pfont_large;
  }
  else if((keywidth > 20) && (keyheight > 20))
  {
    backspace = pbackspace_medium;
    tab = ptab_medium;
    enter = penter_medium;
    shift = pshift_medium;
    font = pfont_medium;
  }
  else
  {
    backspace = pbackspace_small;
    tab = ptab_small;
    enter = penter_small;
    shift = pshift_small;
    font = pfont_small;
  }

  px = 0; py = 0;
  
  if(keydown == true)
  {
    qp.fillRect(highlightarea, QColor(0,0,0));

    compoundcap = "";
    tisvalue = selectedkeymap[keyrow*columns+keycol];

    if(tisvalue > 32)
    {
      qp.setPen(QColor(255,255,255));
      qp.setFont(QFont(tvk_font, tvk_font_size));
    
      if(tisvalue > 127)
      {
        if((tisvalue == 209) || ((tisvalue >= 211) && (tisvalue < 219)) || ((tisvalue > 230) && (tisvalue < 239)))
        {
          // Add space to NSM
          compoundcap = QChar(0x25cc);
        }

        tisvalue = tisvalue - 0xa0 + 0xe00; // convert to Unicode
      }

      keycap = QChar(tisvalue);

      compoundcap += keycap;
      qp.drawText(highlightarea, Qt::AlignCenter, compoundcap);
    }
    else
    {
      // Draw action keys
      switch(tisvalue)
      {
        case 32: // space
        break;   // do nothing

        case 1: // left shift
        action = *shift;
        px = highlightarea.x() + highlightarea.width()/2 - action.width()/2;
        py = highlightarea.y() + highlightarea.height()/2 - action.height()/2;
        break;

        case 2: // right shift
        action = *shift;
        px = highlightarea.x() + highlightarea.width()/2 - action.width()/2;
        py = highlightarea.y() + highlightarea.height()/2 - action.height()/2;
        break;

        case 3: // font selector
        action = *font;
        px = highlightarea.x() + highlightarea.width()/2 - action.width()/2;
        py = highlightarea.y() + highlightarea.height()/2 - action.height()/2;
        break;

        case 8: // delete
        action = *backspace;
        px = highlightarea.x() + highlightarea.width()/2 - action.width()/2;
        py = highlightarea.y() + highlightarea.height()/2 - action.height()/2;
        break;

        case 9: // tab
        action = *tab;
        px = highlightarea.x() + highlightarea.width()/2 - action.width()/2;
        py = highlightarea.y() + highlightarea.height()/2 - action.height()/2;
        break;

        case 10: // enter
        qp.fillRect((this->width()*(13.0/columns))+1, this->height()*(2.0/rows)+1, this->width(), this->height()/rows, QColor(0,0,0)); 
        action = *enter;
        px = highlightarea.x() + highlightarea.width()/2 - action.width()/2;
        py = highlightarea.y() + highlightarea.height() - action.height()/2;
        break;
      }

      if((tisvalue < 32) && (tisvalue > 0))
      {
#if QT_VERSION > 0x040000
        inverted = action.toImage();
#else
        inverted = action.convertToImage();
#endif

        for(b = 0; b < inverted.height(); b++)
        {
          for(a = 0; a < inverted.width(); a++)
          {
            pel = inverted.pixel(a, b);
 
            inverse = qRed(pel);
            inverted.setPixel(a, b, qRgb(255-inverse, 255-inverse, 255-inverse));
          }
        }
  
#if QT_VERSION > 0x040000
        action = QPixmap::fromImage(inverted);
#else
        action.convertFromImage(inverted);
#endif

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
    tvk_font = previous_font;
    tvk_font_size = previous_font_size;
    CalculateTVKSize();
  }
#if QT_VERSION > 0x040000
  else if((e->key() == Qt::Key_Period) && (e->modifiers() == Qt::ControlModifier))
#else
  else if((e->key() == Qt::Key_Period) && (e->state() == Qt::ControlButton))
#endif
  {
    tvk_font_size++;
    CalculateTVKSize();
  }
#if QT_VERSION > 0x040000
  else if((e->key() == Qt::Key_Comma) && (e->modifiers() == Qt::ControlModifier))
#else
  else if((e->key() == Qt::Key_Comma) && (e->state() == Qt::ControlButton))
#endif
  {
    tvk_font_size--;
    CalculateTVKSize();
  }
  else
    emit PassThroughkeyPressEvent(e);

}

// Calculate and set minimum size
void ThaiVirtualKeyboard::CalculateTVKSize()
{
  int border = 2;
  int glyph_width, glyph_height;
  int min_width, min_height;
  int khomut = 0x0e5b;
  int nine = 0x0e59;
  int ying = 0x0e0d;
  int jula = 0x0e2c;
  int am = 0x0e33;
  int maimalai = 0x0e44;

  QChar testwide = khomut;
  QString testhigh;

  testhigh.append(QChar(0x0e44));
  testhigh.append(QChar(0xe1b));
  testhigh.append(QChar(0x0e26));
  testhigh.append(QChar(0x0e21));
  testhigh.append(QChar(0x0e35));
  testhigh.append(QChar(0x0e49));
  testhigh.append(QChar(0x0e1a));
  testhigh.append(QChar(0x0e39));

  QFont f(tvk_font, tvk_font_size);
  QFontMetrics fm(f);
  glyph_height = fm.boundingRect(testhigh).height();

  // Test several characters to find widest
  QString testam;
  testam.append(QChar(0x25cc)).append(QChar(am));
  glyph_width  = fm.width(testwide);
  if(fm.width(QChar(nine)) > glyph_width) glyph_width = fm.width(QChar(nine));
  if(fm.width(QChar(ying)) > glyph_width) glyph_width = fm.width(QChar(ying));
  if(fm.width(QChar(jula)) > glyph_width) glyph_width = fm.width(QChar(jula));
  if(fm.width(testam) > glyph_width) glyph_width = fm.width(testam);
  if(fm.width(QChar(maimalai)) > glyph_width) glyph_width = fm.width(QChar(maimalai));

/*
  // Verify against action keys
  int sheight = QPixmap(shift).height();
  if(sheight > glyph_height) glyph_height = sheight;

  int twidth = QPixmap(tab).width();
  if(twidth > glyph_width) glyph_width = twidth;
*/

/*
  std::cerr << "am left bearing = " << fm.leftBearing(QChar(am)) << "\n";
  if(fm.leftBearing(QChar(am)) < 0)
  {
    glyph_width -= fm.leftBearing(QChar(am));
  }
*/

  min_height = glyph_height*rows + border*rows*2;
  min_width  = glyph_width*columns + border*columns*2;

  this->setMinimumSize(min_width, min_height);

  if((this->width() > min_width) || (this->height() > min_height))
  {
//    std::cerr << "Resizing to " << min_width << " X " << min_height << "\n";
    this->resize(min_width, min_height);
  }

  // Make sure TVK stays on screen!
  if((this->pos().x() < 0) || (this->pos().y() < 0))
    this->move(0,0);
}