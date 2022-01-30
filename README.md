# LM_CTF for Quake II
This is the source code to LM_CTF Version 5.2 Tournament Edition. The version has been bumped to 6.0 but I have no idea what the differences may be between the 1999 5.2 Edition and this one. This version came to me as 6.0 and I will be incrementing from there as I make functional changes.

I've modified the code for modern ISO C (C11) and added some diagnostics. The original code had numerous warnings and the original make files write about them. There were also modules that were not properly compiled and linked. I don't work that way. I have created a new GNUmakefile to make it painless to build the code on Linux distros. The Visual Studio 2019 project is also warning-free at /W4 and has zero IntelliSense warnings. The GNUmakefile has also been modified and updated to eliminate a lot of options that are no longer needed or are now obsolete on gcc. This version compiles cleanly with no warnings on gcc version 6.3.0 20170516 (Debian 6.3.0-18+deb9u1), gcc (Ubuntu 5.4.0-6ubuntu1~16.04.11) 5.4.0 20160609 and Apple LLVM version 9.1.0 (clang-902.0.39.2), gcc 8.x on Debian Linux reports warnings about potential buffer overflows due to tightened reporting of snprintf, sprintf and strncpy functions. If you modify this code and encounter warnings they are all yours. :)

QwazyWabbit 2020.03.24

# Server CVARs
|CVAR|Default|Notes|
|-|-|-|
|dmflags|0||
|maxclients|4|Number of allowed players|
|ctfflags|0||
|refset|0||
|logrename|||
|runes|15|The number of runes to spawn|
|skinset|0|?|
|refpassword||Allows player to become referee|
|motd_file|motd.txt||
|server_file|server.cfg||
|maplist_file|maplist.txt||
|skin_file|skins.ini||
|skin_debug|0||
|disabled_weps|0||
|flag_init|0||
|fastswitch|0|0=normal, 1=super fast|
|mod_website|http://lmctf.com|The website to download paks|
|autolock|0|Automatically lock teams when match starts, unlock when ends. Pausing match will unlock, unpausing will re-lock. 0=no, 1=yes|
|countdown_time|15|Seconds to countdown when starting a match|

# Client Commands
`players` - Show the players connected

`squadboard` - 

`squad <category>` - 

`squadstatus <status>` - sets the current status

`referee <password>` - authenticate as a ref

`ctfhelp` - show help menu

`ctfmenu` - show the main menu

`users` - show who is connected to the server

`ctfkick <id>` - boot someone from the server

`fobserve` - dunno

`quadtime` - not sure

`gotomap` -

`match` -

`team <red|blue>` - join the red or blue team


# Referee Commands
`refmenu` - show the ref menu

`refcommands` - shows all ref commands and usage

`lock` - locks/unlocks the teams  (toggle)

`unlock` - alias for `lock`

`startmatch` - starts the match on the current map

`stopmatch` - stops the match on the current map

`pausematch` - pauses/unpauses the current match

`unpausematch` - alias for `pausematch`

`setpassword <passwd>` - sets the server password. leave arg blank to unset

`togglefastswitch` - turn on/off the fast weapon switching mode

`changemap <mapname>` - change the map but do not start a match

