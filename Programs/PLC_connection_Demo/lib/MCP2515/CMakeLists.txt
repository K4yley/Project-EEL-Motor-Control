
aux_source_directory(. DIR_MCP2515_SRCS)

include_directories(../Config)


add_library(MCP2515 ${DIR_MCP2515_SRCS})
target_include_directories(MCP2515 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(MCP2515 PUBLIC Config)