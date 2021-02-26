PRO_NAME = librtsp.a
PRO_ROOT = $(shell pwd)
CC = @gcc
AR = @ar
INC := -I$(PRO_ROOT)/..
CFLAGS += $(INC)
LIB = -lpthread -lrt
SRC_DIR := $(PRO_ROOT)
LIB_SRC = \
	$(SRC_DIR)/rtsp_parse.c 		\
	$(SRC_DIR)/rtsp_handler.c 		\
	$(SRC_DIR)/rtp.c				\
	$(SRC_DIR)/h264.c				\
	$(SRC_DIR)/tcp.c				\
	$(SRC_DIR)/udp.c				\
	
obj = $(LIB_SRC:%.c=%.o)
	

all:$(obj)
	$(AR)  -rcs  $(PRO_NAME) $(obj)  
clean:
	@rm $(obj)  $(PRO_NAME)
