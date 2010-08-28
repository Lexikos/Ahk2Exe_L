This is a custom build of Ahk2Exe supporting an additional command-line switch:

    /bin "Path of custom AutoHotkeySC.bin"

The included project and solution files are for Visual C++ Express 2008, despite "VC6".

Recent changes by Lexikos:
  - Added support for x64 builds of AutoHotkey_L. Stores the script as a PE resource.
  - Fixed a bug causing truncation of resource names (and possibly string resources).
  - mpress.exe is used instead of upx.exe; mainly because it supports x64.

Very old changes by Lexikos:
  - Added processing of /bin switch in application.cpp, line 648.
  - Added #define _CRT_SECURE_NO_WARNINGS in StdAfx.h to reduce compiler warnings.
  - Minor change in application.cpp to fix a compile error.

================

This is a version of the AutoIt3 compiler that has been adapted for use with AutoHotkey scripts.

See www.autohotkey.com and www.hiddensoft.com for details.

AutoIt3 Note: See the main AutoIt source.
