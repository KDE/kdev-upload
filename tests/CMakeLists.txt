
kde4_add_executable(permssions permissions.cpp)
target_link_libraries(permssions
    ${KDE4_KIO_LIBS}
)
