add_executable(${PROJECT_NAME}
    main.cpp
    MinecraftHandler.cpp
    PlayerHandler.cpp
    ServerPackets.cpp
    Registry.cpp
    utils.cpp
    DataTypes/Identifier.cpp
    DataTypes/nbt.cpp)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
