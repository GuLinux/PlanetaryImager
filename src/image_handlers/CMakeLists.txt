file(GLOB all_image_handlers *.h)

file(GLOB image_handlers_SRCS *.cpp)
file(GLOB backend_image_handlers_SRCS backend/*.cpp)
file(GLOB frontend_image_handlers_SRCS frontend/*.cpp)


file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/all_handlers.h" "// TODO: deprecated")
foreach(filename ${all_image_handlers})
    file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/all_handlers.h" "#include \"${filename}\"\n")
endforeach()


add_library(image_handlers STATIC ${image_handlers_SRCS})
add_library(backend_image_handlers STATIC ${backend_image_handlers_SRCS})
add_library(frontend_image_handlers STATIC ${frontend_image_handlers_SRCS})

add_imager_dependencies(image_handlers)
add_backend_dependencies(backend_image_handlers)
add_frontend_dependencies(frontend_image_handlers)

add_subdirectory(output_writers)
