vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-core
    REF v${VERSION}
    SHA512 d44bd5db995edfbe6dc56cb7ee9ed7f21377f0a5ceeeb99da996e68e46617b4ac5735d934fa539b402a3832bbc54266040cee12d459734ed42e5392929f6a6b4
)
set(BUILD_C_API OFF)
if("c-api" IN_LIST FEATURES)
  set(BUILD_C_API ON)
endif()

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DFALCON_CORE_BUILD_C_API=${BUILD_C_API}
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup()

file(INSTALL "${SOURCE_PATH}/LICENSE.txt"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)

vcpkg_copy_pdbs()
