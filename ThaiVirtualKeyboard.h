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

signals:
  /// Key press
  void KeyPressed(int tis620val);

  /// Key press to pass on to parent
  void PassThroughkeyPressEvent(QKeyEvent *e);

private:
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

  /// Area of key to be highlighted
  QRect highlightarea;

  /// Image of the keyboard
  QPixmap *thekeyboard;

  /// Image of the shift keyboard
  QPixmap *shiftkeyboard;

  /// Draw the keyboard
  void DrawKeyboard(bool shift);

  /// Calculate the minimum size of TVK, based on the current font size
  void CalculateTVKSize();

  /// Name of current font
  QString tvk_font;

  /// Size of current font
  int tvk_font_size;

  /// Previous font (in case you choose a bad size)
  QString previous_font;
  int previous_font_size;

  // Action keys
  QPixmap *pbackspace_large, *pbackspace_medium, *pbackspace_small;
  QPixmap *ptab_large, *ptab_medium, *ptab_small;
  QPixmap *penter_large, *penter_medium, *penter_small;
  QPixmap *pshift_large, *pshift_medium, *pshift_small;
  QPixmap *pfont_large, *pfont_medium, *pfont_small;
};

#endif // ThaiVirtualKeyboard_h
