/**
 * @file   ThaiVirtualKeyboard.h
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

#ifndef ThaiVirtualKeyboard_h
#define ThaiVirtualKeyboard_h

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QRect>

/// @class Thai Virtual Keyboard (TVK)
class ThaiVirtualKeyboard : public QLabel
{
  Q_OBJECT

public:
  /// Constructor
  ThaiVirtualKeyboard(QWidget *parent = NULL);

  /// Destructor
  ~ThaiVirtualKeyboard() { }

signals:
  /// Key press
  void KeyPressed(int tis620val);

  /// Key press to pass on to parent
  void PassThroughkeyPressEvent(QKeyEvent *e);

protected:
  /// Where the key was pressed
  void mousePressEvent(QMouseEvent *e);

  /// Where the key was released
  void mouseReleaseEvent(QMouseEvent *e);

  /// Intercept hits to real keyboard
  void keyPressEvent(QKeyEvent *e);

  /// Standard resize widget
  void resizeEvent(QResizeEvent *);

  /// Paint the widget
  void paintEvent(QPaintEvent *);

  /// Close the widget
  void closeEvent(QCloseEvent *);

private:
  /// Draw the keyboard
  void drawKeyboard(bool shift);

  /// Calculate the minimum size of TVK, based on the current font size
  void calculateTVKSize();

  /// Return true if font family doesn't support Thai
  bool backupFontRenderer(const QString &family) const;

  /// Width of widget, in keys
  int columns;

  /// Height of widget, in keys
  int rows;

  /// Shift key active
  bool shifted;

  /// State of press down events
  bool keydown;

  /// Key pressed row
  int keyrow;

  /// Key pressed column
  int keycol;

  /// Indicate which size action keys are in use: 0 = small, 1 = medium, 2 = large
  int actionKeySize;

  /// Area of key to be highlighted
  QRect highlightArea;

  /// Image of the keyboard
  QPixmap *thekeyboard;

  /// Image of the shift keyboard
  QPixmap *shiftkeyboard;

  /// Name of current font
  QString tvkFontName;

  /// Size of current font
  int tvkFontSize;

  // Store previous font in case you choose a bad size //

  /// Previous font name
  QString previousFontName;

  /// Previous font size
  int previousFontSize;

  // Action keys
  QPixmap *pix_backspace_large, *pix_backspace_medium, *pix_backspace_small;
  QPixmap *pix_tab_large,   *pix_tab_medium,   *pix_tab_small;
  QPixmap *pix_enter_large, *pix_enter_medium, *pix_enter_small;
  QPixmap *pix_shift_large, *pix_shift_medium, *pix_shift_small;
  QPixmap *pix_font_large,  *pix_font_medium,  *pix_font_small;

  /// Add a spacing character for NSM
  bool addSpaceNSM;

  /// True if Retina
  bool highDPI;
};

#endif  // ThaiVirtualKeyboard_h

