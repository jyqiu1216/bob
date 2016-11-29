BASE_PATH := ../../../base
COMM_PATH := ../../../common
INC_PATH := $(BASE_PATH) $(COMM_PATH)/aws_comm $(COMM_PATH)/aws_tbl $(COMM_PATH)/comm ./bussiness ./frame $(BASE_PATH)/zkutil
SRC_FILES := $(COMM_PATH)/aws_comm/*.cpp $(COMM_PATH)/aws_tbl/*.cpp $(COMM_PATH)/comm/*.cpp ./bussiness/*.cpp ./frame/*.cpp
LIB_PATH := $(BASE_PATH)/lib

project.targets += proj_module
project.extra_warning := -Wno-write-strings -fpermissive -Wno-unused-but-set-variable
project.optimize_flags := 
proj_module.path := ../bin
proj_module.name := module
proj_module.defines := TIXML_USE_STL _UTF8_CODE_
proj_module.sources := $(wildcard $(SRC_FILES))
proj_module.includes := ${INC_PATH} $(BASE_PATH)/zookeeper 
proj_module.ldadd := $(LIB_PATH)/libEncode.a $(LIB_PATH)/libtshpsdk.a $(LIB_PATH)/libSvrPub.a $(LIB_PATH)/libWtseBase.a $(LIB_PATH)/libzookeeper_mt.a $(LIB_PATH)/libjsoncpp.a -lz -pthread -lcurl $(LIB_PATH)/libzkutil.a

include $(BASE_PATH)/Generic.mak

