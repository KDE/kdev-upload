cmake_minimum_required(VERSION 3.0)
project(upload)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 9)
set(VERSION_PATCH 90)

configure_file( "${upload_SOURCE_DIR}/version.h.cmake" "${upload_BINARY_DIR}/version.h" @ONLY )

set(ECM_DEP_VERSION "5.15.0")
find_package (ECM ${ECM_DEP_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${upload_SOURCE_DIR}/cmake/)

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(FeatureSummary)
include(ECMQtDeclareLoggingCategory)
include(KDECMakeSettings)
include(WriteBasicConfigVersionFile)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets)
set(KF5_DEP_VERSION "5.15.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS Config TextEditor I18n KCMUtils JobWidgets Service Parts KIO CoreAddons ItemModels XmlGui)
set(KDEVPLATFORM_DEP_VERSION "5.1.0")
find_package(KDevPlatform ${KDEVPLATFORM_DEP_VERSION} CONFIG)
set_package_properties(KDevPlatform PROPERTIES
    TYPE REQUIRED
)

add_subdirectory(tests)

#plugin
set(kdevupload_PART_SRCS
   kdevuploadplugin.cpp
   allprofilesmodel.cpp
   profilesfiletree.cpp
   uploaddialog.cpp
   uploadjob.cpp
   uploadprofiledlg.cpp
   uploadprofileitem.cpp
   uploadprofilemodel.cpp
   uploadprojectmodel.cpp
   uploadpreferences.cpp
)
set(kdevupload_UI
    uploaddialog.ui
    uploadprofiledlg.ui
    uploadpreferences.ui
)

ecm_qt_declare_logging_category(projectfilter_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_PROJECTFILTER
    CATEGORY_NAME "kdevelop.plugins.upload"
)

ki18n_wrap_ui(uploadpreferences_PART_SRCS uploadpreferences.ui uploadprofiledlg.ui uploaddialog.ui)
kconfig_add_kcfg_files( kdevupload_PART_SRCS uploadconfig.kcfgc )

add_library(kdevupload_PART_SRCS ${kdevupload_UI})

qt5_add_resources(kdevupload_PART_SRCS kdevupload.qrc)
kdevplatform_add_plugin(kdevupload JSON kdevupload.json SOURCES ${kdevupload_PART_SRCS})

target_link_libraries(kdevupload
    KDev::Interfaces
    KDev::Project
    KDev::Serialization

    KF5::JobWidgets
    KF5::KCMUtils
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::KIONTLM
    KF5::CoreAddons
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
