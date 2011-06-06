/**
 * @file   ThaiVirtualKeyboard.h
 * @brief  Thai Virtual Keyboard Widget
 * @author Lyndon Hill
 * @date   2004.06.01 Incept
 */ 
  
#ifndef ThaiVirtualKeyboard_h
#define ThaiVirtualKeyboard_h

#if QT_VERSION > 0x040000
 #include <QWidget>
 #include <QLabel>
 #include <QPixmap>
 #include <QRect>
#else
 #include <qwidget.h>
 #include <qlabel.h>
 #include <qpixmap.h>
 #include <qrect.h>
#endif

/// @class Thai Virtual Keyboard (TVK)
class ThaiVirtualKeyboard : public QLabel
{
  Q_OBJECT

public:
  /// Constructor
  ThaiVirtualKeyboard(QWidget *parent = 0);

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
  QPixmap *pbackspace_large, *pbackspace_medium, *pbackspace_small;
  QPixmap *ptab_large, *ptab_medium, *ptab_small;
  QPixmap *penter_large, *penter_medium, *penter_small;
  QPixmap *pshift_large, *pshift_medium, *pshift_small;
  QPixmap *pfont_large, *pfont_medium, *pfont_small;
};

#endif // ThaiVirtualKeyboard_h
