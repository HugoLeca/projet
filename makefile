
#This is a template to build your own project with the e-puck2_main-processor folder as a library.
#Simply adapt the lines below to be able to compile

# Define project name here
PROJECT = projet_epuck

#Define path to the e-puck2_main-processor folder
GLOBAL_PATH = ../../../lib/e-puck2_main-processor

#Source files to include
CSRC += ./main.c          \
		./pi_regulator.c  \
		./process_image.c \
		./process_distance.c \
		./test_audio.c    \
		./code_to_music.c \

#Header folders to include
INCDIR += ./$(GLOBAL_PATH)/src \

#Jump to the main Makefile
include $(GLOBAL_PATH)/Makefile
