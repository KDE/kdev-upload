
add_executable(permssions permissions.cpp)
target_link_libraries(permssions
    Qt5::Core
    Qt5::Widgets

    KF5::JobWidgets
    KF5::KIOCore
    KF5::KIOWidgets
)
