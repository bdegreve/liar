get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${SELF_DIR}/OpenEXR.cmake")
get_filename_component(OpenEXR_INCLUDE_DIRS "${SELF_DIR}/../include" ABSOLUTE)
set(OpenEXR_LIBRARIES Half Iex IMath IlmThread IlmImf)


