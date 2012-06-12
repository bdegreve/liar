get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include("${SELF_DIR}/IlmBase.cmake")
get_filename_component(IlmBase_INCLUDE_DIRS "${SELF_DIR}/../include" ABSOLUTE)
set(IlmBase_LIBRARIES Half Iex IMath IlmThread)


