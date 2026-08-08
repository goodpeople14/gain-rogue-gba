#---------------------------------------------------------------------------------------------------------------------
# TARGET is the name of the output.
# BUILD is the directory where object files & intermediate files will be placed.
# LIBBUTANO is the main directory of the Butano library.
# PYTHON is the path to the Python interpreter.
# SOURCES is a list of directories containing source code.
# INCLUDES is a list of directories containing extra header files.
# DATA is a list of directories containing binary data files with *.bin extension.
# GRAPHICS is a list of files and directories containing files to be processed by grit.
# AUDIO is a list of files and directories containing files to be processed by the audio backend.
# AUDIOBACKEND specifies the backend used for audio playback.
# DMGAUDIO is a list of files and directories containing files to be processed by the DMG audio backend.
# ROMTITLE is an uppercase ASCII string with a maximum length of 12 characters.
# ROMCODE is an uppercase ASCII string with a maximum length of 4 characters.
#---------------------------------------------------------------------------------------------------------------------

TARGET           := gain-rogue-gba
BUILD            := build

# Project:
SOURCES          := src \
                    src/game \
                    src/scene \
                    src/world \
                    src/character \
                    src/combat \
                    src/debug \
                    src/combat/melee \
                    src/combat/collision
INCLUDES         := include
DATA             :=
GRAPHICS         := graphics/characters/heroes/gunner \
                    graphics/characters/heroes/swordsman \
                    graphics/characters/enemies/caveman \
                    graphics/characters/enemies/goblin \
                    graphics/characters/enemies/ninja \
                    graphics/effects/attacks/swordsman \
                    graphics/effects/common \
                    graphics/backgrounds \
                    graphics/ui
AUDIO            := audio
DMGAUDIO         := dmg_audio

# Butano:
LIBBUTANO        := ../butano/butano
PYTHON           := python

# Audio:
AUDIOBACKEND     := maxmod
AUDIOTOOL        :=
DMGAUDIOBACKEND  := default

# ROM metadata:
ROMTITLE         := GAIN ROGUE
ROMCODE          := GRGA

# Compiler and linker options:
USERFLAGS        :=
USERCXXFLAGS     :=
USERASFLAGS      :=
USERLDFLAGS      :=
USERLIBDIRS      :=
USERLIBS         :=
DEFAULTLIBS      :=
STACKTRACE       :=
USERBUILD        :=
EXTTOOL          :=

#---------------------------------------------------------------------------------------------------------------------
# Export absolute Butano path:
#---------------------------------------------------------------------------------------------------------------------

ifndef LIBBUTANOABS
	export LIBBUTANOABS := $(realpath $(LIBBUTANO))
endif

#---------------------------------------------------------------------------------------------------------------------
# Include Butano main makefile:
#---------------------------------------------------------------------------------------------------------------------

include $(LIBBUTANOABS)/butano.mak
