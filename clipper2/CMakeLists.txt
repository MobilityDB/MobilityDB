# Vendored Clipper2 v2.0.1 — boolean clipping subset only.
# Upstream: https://github.com/AngusJohnson/Clipper2
# License: Boost Software License 1.0 (see LICENSE in this directory).
#
# We vendor 5 headers + 2 .cpp files (clipper.engine.cpp + clipper.rectclip.cpp).
# The clipper.offset / clipper.minkowski / clipper.triangulation modules are
# intentionally NOT vendored — re-add them here if MEOS ever needs offsetting,
# Minkowski sums, or triangulation.

set(CLIPPER2_SRCS
  src/clipper.engine.cpp
  src/clipper.rectclip.cpp
)

add_library(clipper2 OBJECT ${CLIPPER2_SRCS})
target_include_directories(clipper2 PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
set_target_properties(clipper2 PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE ON
)
