BASE_PATH := ../../../base
COMM_PATH := ../../../common
INC_PATH := $(BASE_PATH) $(COMM_PATH)/aws_comm $(COMM_PATH)/aws_tbl $(COMM_PATH)/comm .
SRC_FILES := $(COMM_PATH)/aws_comm/*.cpp $(COMM_PATH)/aws_tbl/*.cpp $(COMM_PATH)/comm/*.cpp ./*.cpp
LIB_PATH := $(BASE_PATH)/lib

project.targets += wa_dev_proj
project.extra_warning := -Wno-write-strings -fpermissive -Wno-unused-but-set-variable
project.optimize_flags := 
wa_dev_proj.path := ../bin
wa_dev_proj.name := auto_compute
wa_dev_proj.defines := TIXML_USE_STL _UTF8_CODE_
wa_dev_proj.sources := $(wildcard $(SRC_FILES))
wa_dev_proj.includes := ${INC_PATH} $(BASE_PATH)/zookeeper
wa_dev_proj.ldadd := $(LIB_PATH)/libEncode.a $(LIB_PATH)/libtshpsdk.a $(LIB_PATH)/libSvrPub.a $(LIB_PATH)/libWtseBase.a $(LIB_PATH)/libzookeeper_mt.a $(LIB_PATH)/libjsoncpp.a -pthread -lcurl	

include $(BASE_PATH)/Generic.mak

