BASE_PATH := ../../../base
COMM_PATH := ../../../common
INC_PATH := $(BASE_PATH) $(BASE_PATH)/zkutil $(COMM_PATH)/aws_comm $(COMM_PATH)/aws_tbl $(COMM_PATH)/comm  $(COMM_PATH)/proto ./bussiness ./frame ./output /usr/local/include
SRC_FILES := $(COMM_PATH)/aws_comm/*.cpp $(COMM_PATH)/aws_tbl/*.cpp $(COMM_PATH)/comm/*.cpp $(COMM_PATH)/proto/*.cc ./bussiness/*.cpp ./frame/*.cpp  ./output/*.cpp 
LIB_PATH := $(BASE_PATH)/lib

project.targets += wa_dev_hu_proj_wave
project.extra_warning := -Wno-write-strings -fpermissive -Wno-unused-but-set-variable
project.optimize_flags := -DPROJNAME=\"bob\"
wa_dev_hu_proj_wave.path := ../bin
wa_dev_hu_proj_wave.name := hu
wa_dev_hu_proj_wave.defines := TIXML_USE_STL _UTF8_CODE_
wa_dev_hu_proj_wave.sources := $(wildcard $(SRC_FILES))
wa_dev_hu_proj_wave.includes := ${INC_PATH} $(BASE_PATH)/zookeeper 
wa_dev_hu_proj_wave.ldadd := /usr/local/lib/libprotobuf.a $(LIB_PATH)/libzkutil.a $(LIB_PATH)/libEncode.a $(LIB_PATH)/libtshpsdk.a $(LIB_PATH)/libSvrPub.a $(LIB_PATH)/libWtseBase.a $(LIB_PATH)/libzookeeper_mt.a $(LIB_PATH)/libjsoncpp.a -lz -pthread -lcurl			

include $(BASE_PATH)/Generic.mak

