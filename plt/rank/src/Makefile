BASE_PATH := ../../../base
COMM_PATH := ../../../common
INC_PATH := $(BASE_PATH) $(BASE_PATH)/zkutil $(COMM_PATH)/aws_comm $(COMM_PATH)/aws_tbl $(COMM_PATH)/comm $(COMM_PATH)/proto ./bussiness ./frame /usr/local/include
SRC_FILES := $(COMM_PATH)/aws_comm/*.cpp $(COMM_PATH)/aws_tbl/*.cpp $(COMM_PATH)/comm/*.cpp $(COMM_PATH)/proto/*.cc ./bussiness/*.cpp ./frame/*.cpp
LIB_PATH := $(BASE_PATH)/lib

project.targets += rank_proj
project.extra_warning := -Wno-write-strings -fpermissive -Wno-unused-but-set-variable
project.optimize_flags := 
rank_proj.path := ../bin
rank_proj.name := rank
rank_proj.defines := TIXML_USE_STL _UTF8_CODE_
rank_proj.sources := $(wildcard $(SRC_FILES))
rank_proj.includes := ${INC_PATH} 
rank_proj.ldadd := /usr/local/lib/libprotobuf.a $(LIB_PATH)/libzkutil.a $(LIB_PATH)/libzookeeper_mt.a $(LIB_PATH)/libEncode.a $(LIB_PATH)/libtshpsdk.a $(LIB_PATH)/libSvrPub.a $(LIB_PATH)/libWtseBase.a $(LIB_PATH)/libjsoncpp.a -lz -pthread -lcurl

include $(BASE_PATH)/Generic.mak

