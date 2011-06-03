/**
 * @file   virtualkb.cc
 * @brief  TVK test
 * @author Lyndon Hill
 * @date   2008.04.06 Adapted to QT3/QT4
 */

#if QT_VERSION > 0x040000
 #include <QApplication>
#else
 #include <qapplication.h>
#endif

#include "ThaiVirtualKeyboard.h"

int main(int argc, char **argv)
{
  QApplication a(argc, argv);

  ThaiVirtualKeyboard mykb;
//  mykb.resize(420,160);
  mykb.show();

#if QT_VERSION < 0x040000
  a.setMainWidget(&mykb);
#endif

  return(a.exec());
}

