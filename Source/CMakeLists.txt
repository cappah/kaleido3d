cmake_minimum_required (VERSION 3.0)
project (Kaleido3D)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${Kaleido3D_SOURCE_DIR}/../Project")
set(Kaleido3D_ROOT_DIR "${Kaleido3D_SOURCE_DIR}/..")
set(K3D_THIRD_PARTY "${Kaleido3D_ROOT_DIR}/Source/ThirdParty")

if(IOS)
set(LIBRARY_OUTPUT_PATH ${Kaleido3D_ROOT_DIR}/Lib)
endif()

option(BUILD_SHARED on)
option(BUILD_WITH_TBB off)

set(LIB_TYPE STATIC)
set_property(GLOBAL PROPERTY USE_FOLDERS on)

if(BUILD_SHARED)
	add_definitions(-DLIB_BUILD)
	set(LIB_TYPE SHARED)
endif()

if(BUILD_WITH_TBB)
	add_definitions(-DUSE_TBB_MALLOC)
	include_directories(${TBB_INCLUDE_DIR})
endif()


message(STATUS "compiler is ${CMAKE_CXX_COMPILER_ID}" ) 
## C++ 11 Support
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-attributes -pthread -std=c++11")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
	if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "Apple")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-attributes -pthread -std=c++11")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
endif()

#-------------------------------------------------------------------------------
# Definitions For MSVC 2013
#-------------------------------------------------------------------------------
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_MBCS)
	add_definitions(-DMBCS)
endif(MSVC)

#-------------------------------------------------------------------------------
# Definitions For iOS
#-------------------------------------------------------------------------------
if(IOS)
    add_definitions(-DK3DPLATFORM_OS_IOS=1)
    add_definitions(-D__arm__)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${Kaleido3D_ROOT_DIR}/Binary/")


include_directories(
	.
	../Include
	${K3D_THIRD_PARTY}/SDL2-2.0.3/include
	${K3D_THIRD_PARTY}/rapidjson/include
	ThirdParty/glslang
	${K3D_THIRD_PARTY}/EASTL/include
	${K3D_THIRD_PARTY}/EASTL/test/packages/EABase/include/Common
)

include(${Kaleido3D_ROOT_DIR}/Project/pch.cmake)

if(WIN32)
    set(SDL2_DIR ${K3D_THIRD_PARTY}/SDL2-2.0.3)
    set(VULKANSDK_LOCATION ${K3D_THIRD_PARTY}/vulkan)
    set(TBB_INSTALL_DIR ${K3D_THIRD_PARTY}/tbb43_20150424)
    set(DXSDK_LOCATION ${K3D_THIRD_PARTY}/dxsdk)
    include(${Kaleido3D_ROOT_DIR}/Project/Common.cmake)
    include(${Kaleido3D_ROOT_DIR}/Project/FindVulkanSDK.cmake)
    include(${Kaleido3D_ROOT_DIR}/Project/FindSDL2.cmake)
    include(${Kaleido3D_ROOT_DIR}/Project/FindMaya.cmake)
    include(${Kaleido3D_ROOT_DIR}/Project/FindTBB.cmake)
    include(${Kaleido3D_ROOT_DIR}/Project/FindDXSDK.cmake)
	find_package(SDL2)
	find_package(OpenGL)
	find_package(Maya)
	find_package(TBB)
	find_package(DXSDK)
	#set(PROTOBUF_SRC_ROOT_FOLDER "${K3D_THIRD_PARTY}/protobuf-2.6-win")
	#find_package(Protobuf REQUIRED)
	_add_package_VulkanSDK()
endif()

add_subdirectory(Core)
add_subdirectory(Renderer)
add_subdirectory(Engine)
add_subdirectory(RHI)
add_subdirectory(Network)
# add_subdirectory(Launcher)

if(MAYA_FOUND)
	add_subdirectory(Tools/MayaDCC)
endif()
add_subdirectory(Tools/ShaderGen)
add_subdirectory(ThirdParty/glslang)
add_subdirectory(ThirdParty/Spir2Cross)
add_subdirectory(ThirdParty/EASTL)

if(WIN32)
add_subdirectory(UnitTest)
#add_subdirectory(Example)
endif()