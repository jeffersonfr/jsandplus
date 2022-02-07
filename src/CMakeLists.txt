cmake_minimum_required (VERSION 3.0)

pkg_check_modules(jCanvas REQUIRED IMPORTED_TARGET jcanvas)

add_executable(jsandplus
    main.cpp
  )

target_link_libraries(jsandplus
  PRIVATE
    PkgConfig::jCanvas
    Threads::Threads
)
