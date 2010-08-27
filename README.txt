This is a custom build of Ahk2Exe supporting an additional command-line switch:

    /bin "Path of custom AutoHotkeySC.bin"

The included project and solution files are for Visual C++ Express 2008, despite "VC6".

Changes by Lexikos:
  - Added processing of /bin switch in application.cpp, line 648.
  - Added #define _CRT_SECURE_NO_WARNINGS in StdAfx.h to reduce compiler warnings.
  - Minor change in application.cpp to fix a compile error.

================

This is a version of the AutoIt3 compiler that has been adapted for use with AutoHotkey scripts.

See www.autohotkey.com and www.hiddensoft.com for details.

AutoIt3 Note: See the main AutoIt source.
