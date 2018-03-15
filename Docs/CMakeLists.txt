find_package(Doxygen REQUIRED)

configure_file(
${CMAKE_CURRENT_SOURCE_DIR}/conf.py
${CMAKE_CURRENT_BINARY_DIR}/conf.py
    COPYONLY
)

add_custom_target(vortex2d_docs_doxygen
    ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Build Doxygen-generated documentation"
)

add_custom_target(vortex2d_docs_sphinx
    ALL
    COMMAND sphinx-build -b html -c ${CMAKE_CURRENT_SOURCE_DIR} . ${CMAKE_CURRENT_BINARY_DIR}/html
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS vortex2d_docs_doxygen
    COMMENT "Generate HTML documentation"
    )
