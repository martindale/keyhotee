# Required packages for building on Debian Jessie:
# libboost-all-dev
# libssl-dev
# libgtk2.0-dev
# libfreetype6-dev
# libreadline-dev
# libdb++-dev

project( keyhotee )
cmake_minimum_required( VERSION 2.8.12.1 )

macro(FillQtDependentLibraries)
  GET_TARGET_PROPERTY(QT_LIB_DIR Qt5::Core LOCATION)
  GET_FILENAME_COMPONENT(QT_LIB_DIR "${QT_LIB_DIR}" PATH)

  SET (DEBUG_SUFFIX "")

  IF(WIN32)
    # On windows platform release & debug libraries have different names
    SET (DEBUG_SUFFIX "D")
  ENDIF(WIN32)
  
  # Warning this lib doesn't have an associated .cmake :-(
  find_library(Qt5Platform_LIBRARIES_RELEASE Qt5PlatformSupport HINTS "${QT_LIB_DIR}" "${QT_LIB_DIR}/../lib")
  find_library(Qt5Platform_LIBRARIES_DEBUG Qt5PlatformSupport${DEBUG_SUFFIX} HINTS "${QT_LIB_DIR}" "${QT_LIB_DIR}/../lib")

  # Warning this lib doesn't have an associated .cmake :-(
  find_library(Qt5SvgIcon_LIBRARIES_RELEASE qsvgicon HINTS "${QT_LIB_DIR}/../plugins/iconengines/")
  find_library(Qt5SvgIcon_LIBRARIES_DEBUG qsvgicon${DEBUG_SUFFIX} HINTS "${QT_LIB_DIR}/../plugins/iconengines/")
  
  IF(NOT Qt5SvgIcon_LIBRARIES_RELEASE)
    MESSAGE(WARNING "Qt5SvgIcon_LIBRARIES_RELEASE not found, QT plugin static library may not installed, could set KH_STATIC_QT to 0 to disable __STATIC_QT")
  ENDIF(NOT Qt5SvgIcon_LIBRARIES_RELEASE)
  
  if(WIN32)
    message(STATUS "Setting up additional QT Dependencies for Windows platform")
    # put here all system libraries which have to be linked 
    LIST(APPEND "QT_DEPENDENCY_LIBRARIES" "imm32.lib" "winmm.lib")
  else(WIN32)
    IF(UNIX)
      message(STATUS "Setting up additional QT Dependencies for Unix platform")

      find_library(QXcbStatic_LIBRARY xcb-static HINTS "${QT_LIB_DIR}/../plugins/platforms/xcb-static/")
      list (APPEND QT_DEPENDENCY_LIBRARIES ${QXcbStatic_LIBRARY})

      find_library(FONTCONFIG_LIBRARY fontconfig)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${FONTCONFIG_LIBRARY}")

      find_library(FREETYPE_LIBRARY freetype)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${FREETYPE_LIBRARY}")

      find_package (XCB)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${XCB_XCB_LIBRARY}" "${XCB_GLX_LIBRARY}")

      find_package(X11_XCB)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${X11_XCB_LIBRARIES}")

      find_package(X11)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${X11_ICE_LIB}" "${X11_SM_LIB}" "${X11_X11_LIB}"
        "${X11_Xext_LIB}" "${X11_Xi_LIB}" "${X11_Xrender_LIB}"
      )

      find_package(GTK2)
      list (APPEND QT_DEPENDENCY_LIBRARIES "${GTK2_GLIB_LIBRARY_RELEASE}"
       "${GTK2_GOBJECT_LIBRARY_RELEASE}" "${GTK2_GTHREAD_LIBRARY_RELEASE}"
      )
    ENDIF(UNIX)
  ENDIF(WIN32)
endmacro()

set( CMAKE_INCLUDE_CURRENT_DIR ON)
set( CMAKE_AUTOMOC ON  )
SET( BUILD_SHARED_LIBS OFF )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/BitShares/fc/GitVersionGen/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CmakeModules/")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

#  set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";$ENV{QTDIR}/lib/cmake;C:/gh/qt5.1.1/qtwebkit/WebKitBuild/Release/lib/cmake/Qt5WebKitWidgets;"  )
set( CMAKE_PREFIX_PATH  ${CMAKE_PREFIX_PATH} ";$ENV{QTDIR}/lib/cmake"  )
#message(CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH})

SET(KH_STATIC_QT 1)
SET(QT_DEPENDENCY_LIBRARIES "")
SET(ICU_LIBS_PREFIX "")

SET (ORIGINAL_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

IF( WIN32 )
  MESSAGE(platform= "Compiling on WIN32")

  IF(KH_STATIC_QT EQUAL 1)
    # Use dedicated prefix to find static versions of libraries 
    SET(ICU_LIBS_PREFIX "s")
  ENDIF()

  #Set to allow xp compatibility
  set(CMAKE_GENERATOR_TOOLSET "v110_xp" CACHE INTERNAL "XP-compatible VS 11")
  #You need to set OPENSSL_ROOT environment variable for your system on WIN32
  message("Setting up OpenSSL root and include vars on Win32 platform")
  set( OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT} )
  set( OPENSSL_INCLUDE_DIR ${OPENSSL_ROOT_DIR}/include )
ELSE()
  IF(KH_STATIC_QT EQUAL 1)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
  ENDIF()
ENDIF(WIN32)

find_package( OpenSSL )
find_package( ICU )

SET (CMAKE_FIND_LIBRARY_SUFFIXES ${ORIGINAL_LIB_SUFFIXES})

find_package( Qt5LinguistTools )
find_package( Qt5PrintSupport )
find_package( Qt5Svg )
find_package( Qt5Widgets )

IF(KH_STATIC_QT EQUAL 1)
  ADD_DEFINITIONS( -D__STATIC_QT )
  FillQtDependentLibraries()
ENDIF()

#Note: bitshares repo should be checked out under keyhotee dir or path here should be changed
set(BITSHARES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/BitShares)
message(${BITSHARES_DIR})
include_directories(${BITSHARES_DIR}/include)
include_directories(${BITSHARES_DIR}/fc/include)
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

SET (INVICTUS_ROOT $ENV{INVICTUS_ROOT})

IF("${INVICTUS_ROOT}" STREQUAL "")
  SET(INVICTUS_ROOT "./")
ENDIF("${INVICTUS_ROOT}" STREQUAL "")

message(STATUS "Using invictus root: " ${INVICTUS_ROOT})

ADD_DEFINITIONS( -DALPHA_RELEASE )

IF( WIN32 )
  ADD_DEFINITIONS( -DWIN32 )
  ADD_DEFINITIONS( -DNOMINMAX )
  ADD_DEFINITIONS( -D_SCL_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -D_WIN32_WINNT=0x0501 )
  ADD_DEFINITIONS( -D_CRT_SECURE_NO_WARNINGS )
  ADD_DEFINITIONS( -DLEVELDB_PLATFORM_WINDOWS )
  # Activate C++ exception handling inc. SEH to catch GPFs
  IF (NOT CMAKE_CXX_FLAGS MATCHES "/EHa")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHa")
  ENDIF()
  #boost
  SET(BOOST_ROOT $ENV{BOOST_ROOT})
  add_definitions(-DBOOST_ALL_NO_LIB)  # Needed to disable MSVC autolinking feature (#pragma comment)
  set(Boost_USE_DEBUG_PYTHON ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_LIBS OFF)
  set(BOOST_ALL_DYN_LINK ON) # force dynamic linking for all libraries

  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context)
  # For Boost 1.53 on windows, coroutine was not in BOOST_LIBRARYDIR and do not need it to build,  but if boost versin >= 1.54, find coroutine otherwise will cause link errors
  IF(NOT "${Boost_VERSION}" MATCHES "1.53(.*)")
    FIND_PACKAGE(Boost 1.54 REQUIRED COMPONENTS coroutine)
  ENDIF()

  include_directories( ${BITSHARES_DIR}/vendor/leveldb-win/include )
ELSE(WIN32)
  include_directories( ${BITSHARES_DIR}/vendor/leveldb-1.12.0/include )
  include_directories( ${BITSHARES_DIR}/vendor/qtmacextras/include )

  MESSAGE(STATUS "Compiling on UNIX")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fnon-call-exceptions -std=c++0x -Wall -Wno-unused-local-typedefs -fmax-errors=3 ")#-msse4.2" ) 
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unused-local-typedefs -fmax-errors=3 ") #-msse4.2" )
  #SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wno-unused-local-typedefs" ) 
  #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unused-local-typedefs" )
  SET(Boost_USE_STATIC_LIBS ON)
  FIND_PACKAGE(Boost 1.53 REQUIRED COMPONENTS thread date_time system filesystem program_options signals serialization chrono unit_test_framework context coroutine) 

  IF( NOT APPLE )
    SET( pthread pthread )
  ENDIF( NOT APPLE )
ENDIF(WIN32)

include_directories( ${OPENSSL_INCLUDE_DIR} )
include_directories( ${Boost_INCLUDE_DIR} )
SET( ALL_OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES})

add_subdirectory(BitShares)
add_subdirectory(miner)

IF( APPLE )
    SET_SOURCE_FILES_PROPERTIES(
                                "images/keyhotee.icns"
                                PROPERTIES
                                MACOSX_PACKAGE_LOCATION Resources
                                )
    SET( MACOSX_BUNDLE_BUNDLE_NAME "Keyhotee" )
    SET( MACOSX_BUNDLE_GUI_IDENTIFIER "com.invictus-innovations.Keyhotee" ) 
    set( MACOSX_BUNDLE_ICON_FILE keyhotee.icns )
    SET( MACOSX_BUNDLE_INFO_STRING "Keyhotee - version 1.0" ) 
    SET( MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0" ) 
    SET( MACOSX_BUNDLE_LONG_VERSION_STRING "1.0.0" ) 
    SET( MACOSX_BUNDLE_BUNDLE_VERSION "1.0" ) 
    SET( MACOSX_BUNDLE_COPYRIGHT "(C) 2013 Invictus Innovations" ) 
  
ENDIF( APPLE )

set( headers
        profile_wizard/ProfileWizard.hpp
    )

qt5_wrap_ui( profile_wizard/UiProfileEditPage  profile_wizard/ProfileEditPage.ui )
qt5_wrap_ui( profile_wizard/UiProfileIntroPage profile_wizard/ProfileIntroPage.ui )
qt5_wrap_ui( profile_wizard/UiNymPage          profile_wizard/ProfileNymPage.ui )
qt5_wrap_ui( UiLoginDialog                     LoginDialog.ui )
qt5_wrap_ui( UiConnectionStatusFrame           connectionstatusframe.ui )
qt5_wrap_ui( UiKeyhoteeMainWindow              KeyhoteeMainWindow.ui )
qt5_wrap_ui( Uidiagnosticdialog                diagnosticdialog.ui )
qt5_wrap_ui( UiNewIdentityDialog               AddressBook/NewIdentityDialog.ui )
qt5_wrap_ui( UiContactsTable                   AddressBook/ContactsTable.ui )
qt5_wrap_ui( UiContactView                     AddressBook/ContactView.ui )
qt5_wrap_ui( UiMailbox                         Mail/Mailbox.ui )
qt5_wrap_ui( UiMailViewer                      Mail/MailViewer.ui )
qt5_wrap_ui( UiMailEditor                      Mail/maileditorwindow.ui )
qt5_wrap_ui( UiMailFieldsWidgets               Mail/mailfieldswidget.ui )
qt5_wrap_ui( UiMoneyAttachementWidget          Mail/moneyattachementwidget.ui )
qt5_wrap_ui( UiFileAttachmentWidget            Mail/fileattachmentwidget.ui )
qt5_wrap_ui( UiFileAttachmentDialog            Mail/FileAttachmentDialog.ui )
qt5_wrap_ui( UiWallets                         wallets.ui )

set( library_sources
        AddressBook/AddressBookModel.hpp
        AddressBook/AddressBookModel.cpp )

set( sources  
        qtreusable/selfsizingmainwindow.h
        qtreusable/selfsizingmainwindow.cpp
        qtreusable/AutoUpdateProgressBar.hpp
        qtreusable/AutoUpdateProgressBar.cpp
        qtreusable/TImage.hpp
        qtreusable/TImage.cpp

        profile_wizard/ProfileEditPage.ui
        profile_wizard/ProfileIntroPage.ui
        profile_wizard/ProfileNymPage.ui
        profile_wizard/ProfileWizard.cpp

        AddressBook/NewIdentityDialog.ui
        AddressBook/NewIdentityDialog.hpp
        AddressBook/NewIdentityDialog.cpp
        AddressBook/ContactsTable.ui
        AddressBook/ContactsTable.hpp
        AddressBook/ContactsTable.cpp
        AddressBook/ContactView.ui
        AddressBook/ContactView.hpp
        AddressBook/ContactView.cpp
        AddressBook/Contact.cpp
        AddressBook/TableViewCustom.hpp
        AddressBook/TableViewCustom.cpp

        Mail/MailboxModel.hpp
        Mail/MailboxModel.cpp

        Mail/Mailbox.ui
        Mail/Mailbox.hpp
        Mail/Mailbox.cpp

        Mail/MailViewer.ui
        Mail/MailViewer.hpp
        Mail/MailViewer.cpp

        Mail/maileditorwindow.ui
        Mail/maileditorwindow.hpp
        Mail/maileditorwindow.cpp

        Mail/mailfieldswidget.ui
        Mail/mailfieldswidget.hpp
        Mail/mailfieldswidget.cpp

        Mail/moneyattachementwidget.ui
        Mail/moneyattachementwidget.hpp
        Mail/moneyattachementwidget.cpp

        Mail/fileattachmentwidget.ui
        Mail/fileattachmentwidget.hpp
        Mail/fileattachmentwidget.cpp
        Mail/FileAttachmentDialog.ui
        Mail/FileAttachmentDialog.hpp
        Mail/FileAttachmentDialog.cpp

        KeyhoteeApplication.hpp
        KeyhoteeApplication.cpp

        LoginDialog.ui
        LoginDialog.hpp
        LoginDialog.cpp

        ContactListEdit.hpp
        ContactListEdit.cpp

        TreeWidgetCustom.hpp
        TreeWidgetCustom.cpp

        KeyhoteeMainWindow.ui
        KeyhoteeMainWindow.hpp
        KeyhoteeMainWindow.cpp

        ATopLevelWindowsContainer.hpp
        ATopLevelWindowsContainer.cpp
        ATopLevelWindow.hpp
        ATopLevelWindow.cpp

        diagnosticdialog.ui
        diagnosticdialog.h
        diagnosticdialog.cpp

        utils.hpp
        utils.cpp

        connectionstatusframe.ui
        connectionstatusframe.h
        connectionstatusframe.cpp

        wallets.ui
        wallets.hpp
        wallets.cpp

        dataaccessimpl.h
        dataaccessimpl.cpp
        mailprocessorimpl.hpp
        mailprocessorimpl.cpp

        ch/connectionstatusds.h
        ch/mailprocessor.hpp
        ch/ModificationsChecker.hpp

        MenuEditControl.hpp
        MenuEditControl.cpp

        main.cpp )

set(FILES_TO_TRANSLATE ${sources})
file (GLOB TRANSLATIONS_FILES translations/*.ts)
option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts
  files (WARNING: make clean will delete the source .ts files! Danger!)")
if (UPDATE_TRANSLATIONS)
  qt5_create_translation(QM_FILES ${FILES_TO_TRANSLATE} ${TRANSLATIONS_FILES})
else (UPDATE_TRANSLATIONS)
  qt5_add_translation(QM_FILES ${TRANSLATIONS_FILES})
endif (UPDATE_TRANSLATIONS) 

add_custom_target (translations_target DEPENDS ${QM_FILES})

# Construct an appropriate resource file
SET(QM_QRC "<RCC>\n<qresource prefix=\"/\">\n")
FOREACH(QM_FILE ${QM_FILES})
FILE(RELATIVE_PATH QM_FILE ${CMAKE_CURRENT_BINARY_DIR} ${QM_FILE})
SET(QM_QRC "${QM_QRC}<file>${QM_FILE}</file>\n")
ENDFOREACH(QM_FILE ${QM_FILES})
SET(QM_QRC "${QM_QRC}</qresource>\n</RCC>\n")
SET(QM_QRC_PATH ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_trans.qrc)
FILE(WRITE ${QM_QRC_PATH} ${QM_QRC})

qt5_add_resources( KeyhoteeQRC  Keyhotee.qrc  ${QM_QRC_PATH})

set( resources
        Keyhotee.qrc
        keyhotee_trans.qrc
        images/keyhotee.icns
        qrc_keyhotee_trans.cpp 
        qrc_Keyhotee.cpp )


# Append files generated from git revision to the sources list
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/GitSHA1.cpp.in" "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" @ONLY)
list(APPEND sources "${CMAKE_CURRENT_BINARY_DIR}/GitSHA1.cpp" GitSHA1.h)

# Some unknown diff between windows and unix, this is quick fix, leave for DanL to clean-up
IF(WIN32)
ELSE(WIN32)
  #apparently this is needed for Ubuntu (see commit from jefferylee), but it causes problems for windows (lib is brought in via upnpc-static on windows at least)
  set(MINIUPNP_LIB  miniupnpc)
ENDIF(WIN32)

# Configure dependencies of keyhotee_library module
add_library( keyhotee_library ${library_sources} )
qt5_use_modules(keyhotee_library Widgets)

message(BOOST_LIBRARIES=${BOOST_LIBRARIES})
target_link_libraries( keyhotee_library upnpc-static bshare fc leveldb ${BOOST_LIBRARIES} ${PLATFORM_SPECIFIC_LIBS} ${QtMacExtras} ${APPKIT_LIBRARY} )

# Let's configure binaries output directory (by default invictus-root/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${INVICTUS_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${INVICTUS_ROOT}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${INVICTUS_ROOT}/bin)

# Configure dependencies of keyhotee main application module
add_executable( Keyhotee WIN32 MACOSX_BUNDLE ${sources} ${resources} )
add_dependencies (Keyhotee translations_target)

# Let's configure different target names for release and debug builds.
set_target_properties( Keyhotee PROPERTIES OUTPUT_NAME_DEBUG KeyhoteeD )
set_target_properties( Keyhotee PROPERTIES OUTPUT_NAME_RELEASE Keyhotee )

# Use the Widgets module from Qt 5.
qt5_use_modules(Keyhotee Svg Widgets Gui Core PrintSupport)

set(CMAKE_STATIC_TYPE true)

#fc  miniupnpc
IF (KH_STATIC_QT EQUAL 1)
  target_link_libraries( Keyhotee keyhotee_library 
    ${Qt5Gui_PLUGINS} 
    debug ${Qt5Platform_LIBRARIES_DEBUG} optimized ${Qt5Platform_LIBRARIES_RELEASE}
    debug ${Qt5SvgIcon_LIBRARIES_DEBUG} optimized ${Qt5SvgIcon_LIBRARIES_RELEASE} 
    Qt5::Svg Qt5::Widgets Qt5::Gui Qt5::PrintSupport
    bshare upnpc-static fc leveldb ${BOOST_LIBRARIES}
# Since we would like to build KH against static versions of ICU, it is needed to specify them in
# valid order (DT must be at the end)
    ${PLATFORM_SPECIFIC_LIBS} ${QtMacExtras} ${QT_DEPENDENCY_LIBRARIES} ${ICU_LIBRARIES} ${ICU_DT_LIBRARY}
    ${APPKIT_LIBRARY} ${CMAKE_DL_LIBS} 
  )
# without call FillQtDependentLibraries micro
ELSE (KH_STATIC_QT EQUAL 1)
  target_link_libraries( Keyhotee keyhotee_library 
    Qt5::Svg Qt5::Widgets Qt5::Gui Qt5::PrintSupport
    bshare upnpc-static fc leveldb ${BOOST_LIBRARIES}
    ${PLATFORM_SPECIFIC_LIBS} ${QtMacExtras} ${QT_DEPENDENCY_LIBRARIES} ${ICU_LIBRARIES}
    ${APPKIT_LIBRARY} ${CMAKE_DL_LIBS} 
  )
ENDIF (KH_STATIC_QT EQUAL 1)

if (MSVC)
  message(STATUS "Setting up debug options for MSVC build")
# Probably cmake has a bug and vcxproj generated for executable in Debug conf. has disabled debug info
  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /DEBUG")
endif(MSVC)

