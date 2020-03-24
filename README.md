# lmctf60
# LM_CTF for Quake II

This is the source code to LM_CTF Version 5.2 Tournament Edition.
The version has been bumped to 6.0 but I have no idea what the differences may be between the 1999 5.2 Edition and this one. This version came to me as 6.0 so I am keeping it there.

I've modified the code for modern ISO C (C11) and added some diagnostics. The original code had numerous warnings and the original make files write about them. There were also modules that were not properly compiled and linked. I don't work that way. I have created a new GNUmakefile to make it painless to build the code on Linux distros. The Visual Studio 2019 project is also warning-free at /W4 and has zero IntelliSense warnings. The GNUmakefile has also been modified and updated to eliminate a lot of options that are no longer needed or are now obsolete on gcc. This version compiles cleanly with no warnings on gcc version 6.3.0 20170516 (Debian 6.3.0-18+deb9u1), gcc (Ubuntu 5.4.0-6ubuntu1~16.04.11) 5.4.0 20160609 and Apple LLVM version 9.1.0 (clang-902.0.39.2). 
If you modify this code and encounter warnings they are all yours. :)

QwazyWabbit 2020.03.24
