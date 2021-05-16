#	NAME : code_to_music.c
#	AUTHOR : HUGO LECA and MARGUERITE FAUROUX
#	LAST MODIFICATION : 16/05/2021
#   modified from original template


# Define project name here
PROJECT = projet_e-puck

#Define path to the e-puck2_main-processor folder
GLOBAL_PATH = ../lib/e-puck2_main-processor

#Source files to include
CSRC += ./main.c          \
		./p_regulator.c  \
		./process_image.c \
		./code_to_music.c \
		./robot_management.c  \

#Header folders to include
INCDIR += ./$(GLOBAL_PATH)/src \

#Jump to the main Makefile
include $(GLOBAL_PATH)/Makefile
