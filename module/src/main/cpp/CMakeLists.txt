cmake_minimum_required(VERSION 3.4.1)

if (NOT DEFINED MODULE_NAME)
    message(FATAL_ERROR "MODULE_NAME is not set")
else ()
    project(${MODULE_NAME})
endif ()

add_definitions(-DRIRU_MODULE)

configure_file(template/config.cpp config.cpp)

message("Build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CXX_STANDARD 11)

set(LINKER_FLAGS "-ffixed-x18 -Wl,--hash-style=both")
set(C_FLAGS "-Werror=format -fdata-sections -ffunction-sections")
set(CXX_FLAGS "${CXX_FLAGS} -fno-exceptions -fno-rtti")

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(C_FLAGS "${C_FLAGS} -O2 -fvisibility=hidden -fvisibility-inlines-hidden")
    set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,-exclude-libs,ALL -Wl,--gc-sections -Wl,--strip-all")
else ()
    set(C_FLAGS "${C_FLAGS} -O0")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${C_FLAGS} ${CXX_FLAGS}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${LINKER_FLAGS}")


find_package(riru REQUIRED CONFIG)

enable_language(C ASM)

set(asm_syscall lib/${ANDROID_ABI}/syscall.s)


include_directories(include
        include/ConfigManger
        include/filedata
        include/netlink
        include/imgui
        include/cJson
        include/strnormalize
        include/MapTools
        include/MemoryTools
        include/Logger
        include/RecorderTools
        include/curl
        include/HookFramework
        include/PlatformUtil
        include/TencentSafe
        include/searchCall
        include/PtraceInject
        #include/HideLibrary
        include/MainLooper
        include/input
        src)

include_directories(include/shadowhook
        include/shadowhook/common
        include/shadowhook/arch/${ARCH})

include_directories(include/third_party/bsd
        include/third_party/lss
        include/third_party/xdl)

file(GLOB xdl include/third_party/xdl/*.c)
file(GLOB shadowhook include/shadowhook/*.c include/shadowhook/arch/${ARCH}/*.c include/shadowhook/common/*.c)
file(GLOB bhook include/bhook/*.c)
file(GLOB libseccomp include/SeccompTools/*.c)

add_library(local_curl STATIC IMPORTED)
set_target_properties(local_curl PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/lib/${ANDROID_ABI}/libcurl.a)

add_library(${MODULE_NAME} SHARED main.cpp ${CMAKE_CURRENT_BINARY_DIR}/config.cpp
        src/Draw.cpp
        src/Menu.cpp
        src/GameUtils.cpp

        include/input/touch_helper.cpp

        include/cJson/cJSON.c
        include/strnormalize/strnormalize.cpp
        include/MapTools/MapTools.cpp
        include/MemoryTools/MemoryTools.cpp

        include/imgui/imgui.cpp
        include/imgui/imgui_expand.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_tables.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/stb_image.cpp
        include/imgui/backends/imgui_impl_android.cpp
        include/imgui/backends/imgui_impl_opengl3.cpp

        include/netlink/bionic_netlink.cpp
        include/netlink/ifaddrs.cpp

        include/RecorderTools/RecorderTools.cpp
        include/MainLooper/MainLooper.cpp

        include/ConfigManger/ConfigManger.cpp

        include/filedata/drawicon.cpp
        include/filedata/font.cpp

        include/base64/base64.cpp

        include/HookFramework/hde64.c
        include/HookFramework/SubstrateDebug.cpp
        include/HookFramework/SubstrateHook.cpp
        include/HookFramework/SubstratePosixMemory.cpp
        include/HookFramework/And64InlineHook.cpp
        #include/HookFramework/elf_util.cpp

        #include/TencentSafe/TencentSafe.cpp

        include/searchCall/searchCall.cpp
        #include/HideLibrary/hide_utils.cpp

        ${xdl}

        ${bhook}

        ${asm_syscall}
        )
target_link_libraries(${MODULE_NAME}
        log
        riru::riru
        GLESv3
        android
        local_curl
        EGL)

if (NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
            COMMAND ${CMAKE_STRIP} --strip-all --remove-section=.comment "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/lib${MODULE_NAME}.so")
endif ()
