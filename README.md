# Thai Virtual Keyboard (TVK)

## Introduction

This is a Qt Widget for implementing an onscreen keyboard. By itself, it does
not do anything, it needs to be integrated into an application that will receive
and process virtual key presses.

### Screenshots

You can find more information and screenshots at the
[project page](http://www.lyndonhill.com/Projects/tvk.html).

## Features

- Resizable
- Hand drawn keys for shift etc and font dialog button in 3 sizes
- Complete Thai character set
- Key press events can be passed through to the parent widget so you can type
with an actual keyboard while focus is on TVK
- Developed against Qt 6
- Released under the [GNU General Public Licence (GPL) version 3](https://www.gnu.org/licenses/gpl-3.0.en.html)

## Usage

- Add the include and source files for the `ThaiVirtualKeyboard` class to your
  project file or make system
- In your program, call the functions `foo()` and connect the signal `bar()` to
your slot
- The test program `virtualkb` allows you to check TVK builds properly and is
working

```
  // Your class header file

  #include "ThaiVirtualKeyboard.h"

  ...
 
  // Member variables
  ThaiVirtualKeyboard *tvk;

  ...

  // Your class constructor or initialisation function
  
  tvk = new ThaiVirtualKeyboard;

  connect(tvk, SIGNAL(KeyPressed(int)),
          myWidget, SLOT(insertCharacter(int)));
  connect(tvk, SIGNAL(PassThroughkeyPressEvent(QKeyEvent *)),
          myWidget, SLOT(keyPressEvent(QKeyEvent *)));

  // (I know, this is old style signals and slots)

  ...

  // Your class function to show the keyboard

  tvk->show();
  tvk->activateWindow();
  tvk->raise();

  ...

  // Your widget that accepts Thai input

  MyWidgetType::insertCharacter(int tis620_codepoint)
  {
    // Do something with the character in this slot
  }
```

## Future Work

- Support for High DPI screens
- Better graphics for hand drawn keys and font dialog button
- There is an issue for rendering what Unicode calls "Non Spacing Markers"
(NSM). In Thai tone markers, some vowels and diacritical marks need to be
combined with a consonant in order to render. Standard practice is to show a
dotted circle as a replacement for the consonant. There is an inconsistency in
that some Mac fonts automatically render the dotted circle for you while others
don't and on Linux NSM are always automatically rendered with a dotted circle if
they are not combined with a Thai consonant.

### Work Not Planned

- No plans to implement as an input method
- No plans to refactor, only fix and improve - this is really old code

(c) 2004, Lyndon Hill.
