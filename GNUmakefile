######################################################################
# Linux Makefile for Quake2 v1.4
# Robert "Arras" LeBlanc <rjl@renaissoft.com>
#
# This Makefile is based on the "authoritative" Makefile released
# by Zoid <zoid@idsoftware.com> and refined somewhat by Jet
# <jet@poboxes.com>, and now finally refined a bit more by me to
# make it more useful to mod authors.
#
# To use this Makefile to build your own mod, follow the steps below:
#
# Step 1: Get the 03/14 source distribution from id Software
#         and copy the contents (*.c, *.h) to a working directory.
#
# Step 2: Copy this Makefile into the working directory.  If you have
#         a Redhat Linux distribution, comment out the LDFLAGS
#         definition below--you don't need "-ldl -lm", and in fact the
#         presence of these flags will cause your build to crash and
#         burn.  Slackware users can leave them untouched.
#
# Step 3: Copy any mod-related source files of yours into the working
#         directory.
#
# Step 4: If you developed your source files on a Win32 machine, type
#         "make stripcr" to get rid of any stray carriage returns that
#         may be lurking in your files.
#         MJD - If make complains that it cannot find a separator, then
#               your makefile has been contaminated, too!  Somehow, you
#               need to run stripcr on your makefile so you can use
#               the makefile!  Neat catch-22, there.  I suggest a little
#               set of utils called unix2dos and dos2unix.
#
# Step 5: Add your custom mod-related files to the C_OBJS list below,
#         so that the Makefile knows to build them.  For every *.c
#         file in your mod you should have a corresponding *.o file
#         listed under C_OBJS.  Don't list files that are already
#         part of id's sources (e.g. g_cmds.o, p_client.o, etc.),
#         they're already known to the Makefile; just list any *.c
#         files specific to your mod.
#         MJD - If you get a bunch of messages about a function
#         already being defined, and it shows you the same line
#         twice -- that means you put the same file in the list
#         of objects twice.  Oops.
#
# Step 6: Type "make dep" to build a list of dependencies based on
#         the total set of source files in the working directory.
#
# Step 7: Type "make" to build your mod.
######################################################################

# this nice line comes from the linux kernel makefile
ARCH := $(shell uname -m | sed -e s/i.86/i386/ \
	-e s/sun4u/sparc64/ -e s/arm.*/arm/ \
	-e s/sa110/arm/ -e s/alpha/axp/)

# On 64-bit OS use the command: 'setarch i386 make' after 'make clean'
# to obtain the 32-bit binary DLL on 64-bit Linux.

# on x64 machines do this preparation:
# sudo apt-get install ia32-libs
# sudo apt-get install libc6-dev-i386
# On Ubuntu 16.x use sudo apt install libc6-dev-i386
# this will let you build 32-bits on ia64 systems
#
ifndef REV
    REV := $(shell git rev-list HEAD | wc -l)
endif

ifndef VER
    VER := r$(REV)~$(shell git rev-parse --short HEAD)
endif

# This is for native build
CFLAGS=-O3 -DARCH="$(ARCH)" -DSTDC_HEADERS -DVER='"$(VER)"'
# This is for 32-bit build on 64-bit host
ifeq ($(ARCH),i386)
CFLAGS =-m32 -O3 -DARCH="$(ARCH)" -DSTDC_HEADERS -DVER='"$(VER)"' -I/usr/include
endif

######################################################################
# C_OBJS (Custom Objects): Mod authors should use this group below to
# specify any custom files required by their mods that are not
# included in id's sources.  e.g. if your mod requires the addition
# of custom files "foo.c" and "bar.c", you'd add:
#
# C_OBJS = foo.o bar.o
#
# Leave this empty if you just want to build id's default gamei386.so.
######################################################################
C_OBJS = g_menu.o g_replace.o g_runes.o g_ctffunc.o \
         g_skins.o g_tourney.o plasma.o \
		 p_observer.o g_chase.o p_stats.o \
		 stdlog.o gslog.o bat.o g_vote.o

######################################################################
# End of user-customizable section - you shouldn't have to touch
# anything below this point.
# MJD - With the exception of if you want massive debugging turned
# on or not...
######################################################################

# Game-related objects
G_OBJS = g_ai.o g_cmds.o g_combat.o g_func.o g_items.o g_main.o \
         g_misc.o g_monster.o g_phys.o g_save.o g_spawn.o g_svcmds.o \
         g_target.o g_trigger.o g_turret.o g_utils.o g_weapon.o



# Monster-related objects
M_OBJS = m_actor.o m_berserk.o m_boss2.o m_boss3.o m_boss31.o \
         m_boss32.o m_brain.o m_chick.o m_flash.o m_flipper.o \
         m_float.o m_flyer.o m_gladiator.o m_gunner.o m_hover.o \
         m_infantry.o m_insane.o m_medic.o m_move.o m_mutant.o \
         m_parasite.o m_soldier.o m_supertank.o m_tank.o

# Player-related objects
P_OBJS = p_client.o p_hud.o p_trail.o p_view.o p_weapon.o

# Quake2-related objects
Q_OBJS = q_shared.o

# Lithium II ZBot detection object (uncomment the appropriate binary)
L_OBJS =
#L_OBJS = l2zbot/Linux_x86/zbotcheck.o
#L_OBJS = l2zbot/Linux_AXP/zbotcheck.o

# Note that the mod author's Custom Objects (C_OBJS) are built first,
# which should speed up the detection of compilation errors; if they
# build properly, the rest of id's code should build without complaint,
# at least until link time :)
#
OBJS = $(C_OBJS) $(G_OBJS) $(M_OBJS) $(P_OBJS) $(Q_OBJS)

TARGET = game$(ARCH)-lmctf-$(VER).so

CC = gcc -std=c11

SHELL = /bin/sh
#for MSYS2 or when we don't know the OS.
LIBTOOL = ldd
CFLAGS += -g -Wall

# flavors of Linux
ifeq ($(shell uname),Linux)
CFLAGS += -DLINUX
LIBTOOL = ldd -r
endif

# OS X wants to be Linux and FreeBSD too.
ifeq ($(shell uname),Darwin)
CFLAGS += -DLINUX
LIBTOOL = otool 
endif

# Linker flags for building a shared library (*.so).
#
# Redhat Linux users don't need -ldl or -lm...
# MJD - I'm not sure I buy this.  My Slackware system works fine without
# linking in the DynaLink and Math libs.  It may be more of a function of
# library version than RHS/Slackware...  Try both!
#LDFLAGS =

# but Slackware people do
LDFLAGS = -ldl -lm

SHLIBCFLAGS = -fPIC
SHLIBLDFLAGS = -shared

######################################################################
# Targets
######################################################################

all: GitRevisionInfo dep $(TARGET)

GitRevisionInfo:
	sed -e 's/\$$//g' GitRevisionInfo.tmpl | sed -e "s/WCLOGCOUNT+2/${REV}/g" | sed -e "s/WCREV=7/${VER}/g"  | sed -e "s/WCNOW=%Y/$(shell date +%Y)/g" > GitRevisionInfo.h

.c.o:
	$(CC) $(CFLAGS) $(SHLIBCFLAGS) -o $@ -c $<

$(TARGET):	$(OBJS) $(L_OBJS)
		$(CC) $(CFLAGS) $(SHLIBLDFLAGS) -o $@ $(OBJS) $(L_OBJS) $(LDFLAGS)
		$(LIBTOOL) $@

dep:
	@echo "Updating dependencies..."
	@$(CC) -MM $(OBJS:.o=.c) > .depend

stripcr:	.
		@echo "Stripping carriage returns from source files..."
	 	@for f in *.[ch]; do \
		  cat $$f | tr -d '\015' > .stripcr; \
		  mv .stripcr $$f; \
		done; \
		rm -f .stripcr

clean:
		@echo "Deleting temporary files..."
		@rm -f $(OBJS) GitRevisionInfo.h *.orig ~* core

distclean:	clean
		@echo "Deleting everything that can be rebuilt..."
		@rm -f $(TARGET) .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
