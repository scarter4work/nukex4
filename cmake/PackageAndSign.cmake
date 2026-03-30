# PackageAndSign.cmake — Module signing, packaging, and repository update targets
set(NUKEX_SIGN_KEYS "" CACHE FILEPATH "Path to .xssk signing key file")
set(NUKEX_SIGN_PASS "" CACHE STRING "Signing password")

set(NUKEX_MODULE_NAME "NukeX-pxm")
set(NUKEX_REPO_DIR "${CMAKE_SOURCE_DIR}/repository")

add_custom_target(sign
    COMMAND "${PIXINSIGHT_DIR}/bin/PixInsight.sh"
        "--sign-module-file=$<TARGET_FILE:NukeX>"
        "--xssk-file=${NUKEX_SIGN_KEYS}"
        "--xssk-password=${NUKEX_SIGN_PASS}"
    DEPENDS NukeX
    COMMENT "Signing NukeX module"
    VERBATIM
)

add_custom_target(package
    COMMAND ${CMAKE_COMMAND} -E echo "=== Signing module ==="
    COMMAND "${PIXINSIGHT_DIR}/bin/PixInsight.sh"
        "--sign-module-file=$<TARGET_FILE:NukeX>"
        "--xssk-file=${NUKEX_SIGN_KEYS}"
        "--xssk-password=${NUKEX_SIGN_PASS}"

    COMMAND ${CMAKE_COMMAND} -E echo "=== Creating package tarball ==="
    COMMAND ${CMAKE_COMMAND} -E make_directory "${NUKEX_REPO_DIR}/bin"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:NukeX>" "${NUKEX_REPO_DIR}/bin/"
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE_DIR:NukeX>/${NUKEX_MODULE_NAME}.xsgn"
        "${NUKEX_REPO_DIR}/bin/"

    COMMAND ${CMAKE_COMMAND} -E echo "=== Computing SHA1 and creating tarball ==="
    COMMAND ${CMAKE_COMMAND} -E chdir "${NUKEX_REPO_DIR}"
        tar czf "${NUKEX_REPO_DIR}/$<TARGET_FILE_NAME:NukeX>.tar.gz"
        -C "${NUKEX_REPO_DIR}" bin/

    COMMAND ${CMAKE_COMMAND} -E echo "=== Signing updates.xri ==="
    COMMAND "${PIXINSIGHT_DIR}/bin/PixInsight.sh"
        "--sign-xml-file=${NUKEX_REPO_DIR}/updates.xri"
        "--xssk-file=${NUKEX_SIGN_KEYS}"
        "--xssk-password=${NUKEX_SIGN_PASS}"

    DEPENDS NukeX
    COMMENT "Full package: sign module + create tarball + sign XRI"
    VERBATIM
)
