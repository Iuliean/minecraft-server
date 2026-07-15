set(SERVER server)

add_executable(${SERVER}
    src/main.cpp
    src/minecraft_handler.cpp
    src/player_handler.cpp
    src/registry.cpp
    src/utils.cpp
    src/data_types/identifier.cpp
    src/data_types/nbt.cpp

    include/block_state.hpp
    include/client_packets.hpp
    include/minecraft_handler.hpp
    include/packet_dispatcher.hpp
    include/packet_parser.hpp
    include/packet.hpp
    include/player_handler.hpp
    include/registry.hpp
    include/server_context.hpp
    include/server_packets.hpp
    include/server_state.hpp
    include/utils.hpp

    include/data_types/bitset.hpp
    include/data_types/identifier.hpp
    include/data_types/nbt.hpp
    include/data_types/position.hpp
    include/data_types/uuid.hpp
)

target_include_directories(${SERVER}
                        PRIVATE include/
                        PRIVATE dependencies/nlohmann-json/single_include)

target_link_libraries(${SERVER} PRIVATE SFW::SFW PRIVATE zstr::zstr PRIVATE libcoro)

target_compile_options(${SERVER} PRIVATE ${FLAGS})

install(TARGETS ${SERVER} DESTINATION ${CMAKE_INSTALL_PREFIX})
