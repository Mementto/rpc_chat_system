# 定义了SRC_LIST，包含了该目录下所有的源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)

set(LIBS 
    yaml-cpp
    mprpc
    muduo_net 
    muduo_base 
    mysqlclient 
    pthread
    protobuf
    zookeeper_mt
    )

# 指定生成项目可执行文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST})

# 指定可执行文件链接时需要依赖的库文件
target_link_libraries(ChatServer ${LIBS})