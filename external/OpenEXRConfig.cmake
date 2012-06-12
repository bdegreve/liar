get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${SELF_DIR}/OpenEXR.cmake")
find_package(IlmBase)

get_filename_component(OpenEXR_INCLUDE_DIRS "${SELF_DIR}/../include" ABSOLUTE)
list(APPEND OpenEXR_INCLUDE_DIRS ${IlmBase_INCLUDE_DIRS})

set(OpenEXR_LIBRARIES ${IlmBase_LIBRARIES} IlmImf)


