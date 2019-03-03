# - Try to find the LibEvent config processing library
# Once done this will define
#
# LIBZ_FOUND - System has LibEvent
# LIBZ_INCLUDE_DIR - the LibEvent include directory
# LIBZ_LIBRARIES - The libraries needed to use LibEvent

find_path     (LIBZ_INCLUDE_DIR NAMES zlib.h)
find_library  (LIBZ_LIBRARY     NAMES z)

include (FindPackageHandleStandardArgs)

set (LIBZ_INCLUDE_DIRS ${LIBZ_INCLUDE_DIR})
set (LIBZ_LIBRARIES ${LIBZ_LIBRARY})

find_package_handle_standard_args (LIBZ DEFAULT_MSG LIBZ_LIBRARIES LIBZ_INCLUDE_DIR)
mark_as_advanced(LIBZ_INCLUDE_DIRS LIBZ_LIBRARIES)
