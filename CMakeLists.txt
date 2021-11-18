# CMake powered build system for jVideo
# Create by Jaemyun Kim
# First draft is 29 Oct. 2018

CMAKE_MINIMUM_REQUIRED (VERSION 3.1.0)

PROJECT (MultiVideoCapture VERSION 0.0.1)

# set output path both execution and library files.
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


# Detect Boost ##################################################
IF(WIN32 OR MINGW)
	set(Boost_INCLUDE_DIR CACHE PATH "$ENV{Boost_DIR}")

	IF(NOT Boost_INCLUDE_DIR)
		MESSAGE(FATAL_ERROR "Please set Boost_INCLUDE_DIR in Advanced.")
	ELSE(NOT Boost_INCLUDE_DIR)
		set(CMAKE_INCLUDE_CURRENT_DIR ON)
		set(Boost_DIR ${Boost_INCLUDE_DIR})
	ENDIF(NOT Boost_INCLUDE_DIR)
	
	#set(Boost_INCLUDE_DIR ${Boost_DIR})
	#set(CMAKE_PREFIX_PATH ${Boost_DIR})
	
	set(Boost_USE_STATIC_LIBS		ON)
	set(Boost_USE_MULTITHREADED		ON)
	set(Boost_USE_STATIC_RUNTIME	OFF)
	#FIND_PACKAGE(Boost 1.47.0 COMPONENTS ${BOOST_COMPONENTS_NEEDED})\
ELSE(WIN32 OR MINGW)
ENDIF(WIN32 OR MINGW)

FIND_PACKAGE(Boost 1.54 REQUIRED COMPONENTS filesystem system chrono date_time)
IF(Boost_FOUND)
	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ENDIF(Boost_FOUND)


# Detect OpenCV ##################################################
#IF(WIN32)
#	SET(OpenCV_STATIC ON)
#ELSE(WIN32)
#	SET(OpenCV_STATIC OFF)
#ENDIF(WIN32)
FIND_PACKAGE(OpenCV REQUIRED COMPONENTS core imgproc highgui videoio)
IF(OpenCV_FOUND)
	MESSAGE(STATUS "OpenCV version: " ${OpenCV_VERSION})
	MESSAGE(STATUS "Found the following OpenCV libraries: \n  core\n  imgproc\n  highgui")
	IF(OpenCV_VERSION_MAJOR EQUAL 3)
		FIND_PACKAGE(OpenCV REQUIRED COMPONENTS imgcodecs)
		MESSAGE(STATUS "  imgcodecs")
	ENDIF(OpenCV_VERSION_MAJOR EQUAL 3)
	INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

	IF(WIN32)
		IF(MSVC)
			IF(NOT DEFINED OpenCV_LIB_DIR_DBG)
				SET(OpenCV_LIB_DIR_DBG /Debug)
			ENDIF(NOT DEFINED OpenCV_LIB_DIR_DBG)
			IF(NOT DEFINED OpenCV_LIB_DIR_OPT)
				SET(OpenCV_LIB_DIR_OPT /Release)
			ENDIF(NOT DEFINED OpenCV_LIB_DIR_OPT)
		ENDIF(MSVC)
	ENDIF(WIN32)

ENDIF(OpenCV_FOUND)


# prevent warnings on visual studio.
if(WIN32)
	if(MSVC)
		# for _CRT_SECURE_NO_WARNINGS
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
		
		# for _SCL_SECURE_NO_WARNINGS warning
		add_definitions(-D_SCL_SECURE_NO_WARNINGS)

		# We need add -DQT_WIDGETS_LIB when using QtWidgets in Qt 5.
		add_definitions(${Qt5Widgets_DEFINITIONS})

		# enable multi-processor compilation
		add_definitions(/MP)
	endif(MSVC)
endif(WIN32)


# use c++11 version
set(CMAKE_CXX_FLAGS "-std=c++11")
if(NOT WIN32)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
endif(NOT WIN32)
set(CMAKE_CXX_STANDARD 11) # C++11...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11


# include sub-directories. Target directories have to have "CMakeLists.txt" file.
ADD_SUBDIRECTORY(src)


SET(CMAKE_VERBOSE_MAKEFILE ON)
