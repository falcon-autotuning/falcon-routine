vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO falcon-autotuning/falcon-database
    REF v${VERSION}
    SHA512 34bfd1de79aea6c1b878e2aaf8f115395fe7d966e5b3d4482b698be297d5509f5c33480820dd636e7b553a4ab6d44e5f45987e9d6188dc4b04aded955167df12
)
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)
vcpkg_cmake_install()
vcpkg_cmake_config_fixup()
file(INSTALL "${SOURCE_PATH}/LICENSE"
     DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
     RENAME copyright)
vcpkg_copy_pdbs()
