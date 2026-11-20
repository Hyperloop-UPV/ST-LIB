set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(STM32_CLT_REQUIRED_VERSION "1.21.0" CACHE STRING "Required STM32CubeCLT version")
set(TOOLCHAIN_PREFIX arm-none-eabi-)

if(CMAKE_HOST_WIN32)
    set(_stm32_host_os "Windows")
    set(_stm32_host_exe_suffix ".exe")
elseif(CMAKE_HOST_APPLE)
    set(_stm32_host_os "macOS")
    set(_stm32_host_exe_suffix "")
else()
    set(_stm32_host_os "Linux")
    set(_stm32_host_exe_suffix "")
endif()

function(_stm32_resolve_path _input _out_var)
    if("${_input}" STREQUAL "")
        set(${_out_var} "" PARENT_SCOPE)
        return()
    endif()

    if(IS_ABSOLUTE "${_input}")
        set(_absolute "${_input}")
    else()
        get_filename_component(_absolute "${_input}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_LIST_DIR}")
    endif()

    get_filename_component(_resolved "${_absolute}" REALPATH)
    set(${_out_var} "${_resolved}" PARENT_SCOPE)
endfunction()

function(_stm32_extract_version _path _out_var)
    _stm32_resolve_path("${_path}" _resolved)
    string(TOUPPER "${_resolved}" _resolved_upper)
    string(REGEX MATCH "STM32CUBECLT[_-]([0-9]+\\.[0-9]+\\.[0-9]+)" _match "${_resolved_upper}")
    set(${_out_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
endfunction()

function(_stm32_normalize_root _input _out_var)
    _stm32_resolve_path("${_input}" _resolved)
    if(NOT EXISTS "${_resolved}")
        set(${_out_var} "" PARENT_SCOPE)
        return()
    endif()

    set(_root "")

    if(EXISTS "${_resolved}/GNU-tools-for-STM32/bin/${TOOLCHAIN_PREFIX}gcc${_stm32_host_exe_suffix}")
        set(_root "${_resolved}")
    elseif(IS_DIRECTORY "${_resolved}")
        get_filename_component(_resolved_name "${_resolved}" NAME)
        if(_resolved_name STREQUAL "GNU-tools-for-STM32")
            get_filename_component(_root "${_resolved}" DIRECTORY)
        elseif(_resolved_name STREQUAL "bin")
            get_filename_component(_gnu_dir "${_resolved}" DIRECTORY)
            get_filename_component(_gnu_name "${_gnu_dir}" NAME)
            if(_gnu_name STREQUAL "GNU-tools-for-STM32")
                get_filename_component(_root "${_gnu_dir}" DIRECTORY)
            endif()
        endif()
    else()
        get_filename_component(_filename "${_resolved}" NAME)
        if(_filename STREQUAL "${TOOLCHAIN_PREFIX}gcc${_stm32_host_exe_suffix}"
           OR _filename STREQUAL "${TOOLCHAIN_PREFIX}gcc")
            get_filename_component(_bin_dir "${_resolved}" DIRECTORY)
            get_filename_component(_gnu_dir "${_bin_dir}" DIRECTORY)
            get_filename_component(_gnu_name "${_gnu_dir}" NAME)
            if(_gnu_name STREQUAL "GNU-tools-for-STM32")
                get_filename_component(_root "${_gnu_dir}" DIRECTORY)
            endif()
        endif()
    endif()

    if(_root STREQUAL "")
        set(_root "${_resolved}")
    endif()

    _stm32_resolve_path("${_root}" _resolved_root)
    if(EXISTS "${_resolved_root}/GNU-tools-for-STM32/bin/${TOOLCHAIN_PREFIX}gcc${_stm32_host_exe_suffix}")
        set(${_out_var} "${_resolved_root}" PARENT_SCOPE)
    else()
        set(${_out_var} "" PARENT_SCOPE)
    endif()
endfunction()

function(_stm32_validate_explicit_root _env_name _env_value _out_var)
    _stm32_normalize_root("${_env_value}" _root)
    if(NOT _root)
        message(FATAL_ERROR
            "${_env_name} is set, but it does not resolve to a STM32CubeCLT installation root, "
            "GNU-tools bin directory, or arm-none-eabi-gcc binary:\n  ${_env_value}"
        )
    endif()

    _stm32_extract_version("${_root}" _version)
    if(NOT _version)
        message(FATAL_ERROR
            "${_env_name} resolves to a STM32CubeCLT installation, but its version cannot be "
            "inferred from the real path:\n  ${_root}"
        )
    endif()

    if(_version VERSION_LESS STM32_CLT_REQUIRED_VERSION)
        message(FATAL_ERROR
            "${_env_name} resolves to STM32CubeCLT ${_version}, but version "
            "${STM32_CLT_REQUIRED_VERSION} or newer is required:\n  ${_root}"
        )
    endif()

    set(${_out_var} "${_root}" PARENT_SCOPE)
endfunction()

set(STM32_CLT_ROOT "")

if(DEFINED ENV{HYPER_STM32CLT_ROOT} AND NOT "$ENV{HYPER_STM32CLT_ROOT}" STREQUAL "")
    _stm32_validate_explicit_root("HYPER_STM32CLT_ROOT" "$ENV{HYPER_STM32CLT_ROOT}" STM32_CLT_ROOT)
elseif(DEFINED ENV{STM32_CLT_ROOT} AND NOT "$ENV{STM32_CLT_ROOT}" STREQUAL "")
    _stm32_validate_explicit_root("STM32_CLT_ROOT" "$ENV{STM32_CLT_ROOT}" STM32_CLT_ROOT)
endif()

if(NOT STM32_CLT_ROOT)
    set(_stm32_clt_search_bases "")

    if(CMAKE_HOST_WIN32)
        if(DEFINED ENV{ProgramFiles} AND NOT "$ENV{ProgramFiles}" STREQUAL "")
            list(APPEND _stm32_clt_search_bases
                "$ENV{ProgramFiles}/STMicroelectronics/STM32Cube"
                "$ENV{ProgramFiles}/STMicroelectronics"
            )
        endif()
        list(APPEND _stm32_clt_search_bases
            "C:/ST"
        )
    elseif(CMAKE_HOST_APPLE)
        list(APPEND _stm32_clt_search_bases
            "/Applications/STMicroelectronics/STM32Cube"
            "/Applications/STMicroelectronics"
            "/opt/ST"
            "$ENV{HOME}/ST"
        )
    else()
        list(APPEND _stm32_clt_search_bases
            "/opt/[Ss][Tt]"
            "$ENV{HOME}/[Ss][Tt]"
        )
    endif()

    list(REMOVE_DUPLICATES _stm32_clt_search_bases)

    set(_stm32_clt_candidate_roots "")
    foreach(_base_dir IN LISTS _stm32_clt_search_bases)
        if("${_base_dir}" STREQUAL "")
            continue()
        endif()

        list(APPEND _stm32_clt_candidate_roots
            "${_base_dir}/STM32CubeCLT_${STM32_CLT_REQUIRED_VERSION}"
            "${_base_dir}/STM32CubeCLT-${STM32_CLT_REQUIRED_VERSION}"
        )

        file(GLOB _stm32_clt_globbed LIST_DIRECTORIES true
            "${_base_dir}/[Ss][Tt][Mm]32[Cc][Uu][Bb][Ee][Cc][Ll][Tt][_-]*"
        )
        list(APPEND _stm32_clt_candidate_roots ${_stm32_clt_globbed})
    endforeach()

    list(REMOVE_DUPLICATES _stm32_clt_candidate_roots)

    foreach(_candidate IN LISTS _stm32_clt_candidate_roots)
        _stm32_normalize_root("${_candidate}" _candidate_root)
        if(NOT _candidate_root)
            continue()
        endif()

        _stm32_extract_version("${_candidate_root}" _candidate_version)
        if(_candidate_version AND NOT _candidate_version VERSION_LESS STM32_CLT_REQUIRED_VERSION)
            set(STM32_CLT_ROOT "${_candidate_root}")
            break()
        endif()
    endforeach()
endif()

if(NOT STM32_CLT_ROOT)
    find_program(_stm32_arm_gcc
        NAMES
            ${TOOLCHAIN_PREFIX}gcc
            ${TOOLCHAIN_PREFIX}gcc${_stm32_host_exe_suffix}
    )
    if(NOT _stm32_arm_gcc)
        message(FATAL_ERROR
            "STM32CubeCLT ${STM32_CLT_REQUIRED_VERSION} or newer is required, but arm-none-eabi-gcc was not found. "
            "Install the required CLT and/or set HYPER_STM32CLT_ROOT or STM32_CLT_ROOT."
        )
    endif()

    _stm32_normalize_root("${_stm32_arm_gcc}" _stm32_path_root)
    if(NOT _stm32_path_root)
        message(FATAL_ERROR
            "STM32CubeCLT ${STM32_CLT_REQUIRED_VERSION} or newer is required, but the compiler found in PATH "
            "does not belong to a STM32CubeCLT GNU-tools-for-STM32 layout:\n  ${_stm32_arm_gcc}"
        )
    endif()

    _stm32_extract_version("${_stm32_path_root}" _stm32_path_version)
    if(NOT _stm32_path_version)
        message(FATAL_ERROR
            "STM32CubeCLT ${STM32_CLT_REQUIRED_VERSION} or newer is required, but the compiler found in PATH "
            "comes from a CLT install whose real path does not expose a version:\n  ${_stm32_path_root}"
        )
    endif()

    if(_stm32_path_version VERSION_LESS STM32_CLT_REQUIRED_VERSION)
        message(FATAL_ERROR
            "STM32CubeCLT ${STM32_CLT_REQUIRED_VERSION} or newer is required, but PATH resolves to "
            "${_stm32_path_version}:\n  ${_stm32_path_root}"
        )
    endif()

    set(STM32_CLT_ROOT "${_stm32_path_root}")
endif()

set(STM32_CLT_ROOT "${STM32_CLT_ROOT}" CACHE PATH "Resolved STM32CubeCLT root" FORCE)

set(ARM_TOOLCHAIN_DIR "${STM32_CLT_ROOT}/GNU-tools-for-STM32/bin")
set(CMAKE_C_COMPILER "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc${_stm32_host_exe_suffix}")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}g++${_stm32_host_exe_suffix}")

set(CMAKE_OBJCOPY "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy${_stm32_host_exe_suffix}" CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objdump${_stm32_host_exe_suffix}" CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE_UTIL "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size${_stm32_host_exe_suffix}" CACHE INTERNAL "size tool")
set(CMAKE_NM "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}nm${_stm32_host_exe_suffix}" CACHE INTERNAL "nm tool")
set(CMAKE_STRIP "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}strip${_stm32_host_exe_suffix}" CACHE INTERNAL "strip tool")

if(EXISTS "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc-ar${_stm32_host_exe_suffix}")
    set(CMAKE_C_COMPILER_AR "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc-ar${_stm32_host_exe_suffix}")
    set(CMAKE_CXX_COMPILER_AR "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc-ar${_stm32_host_exe_suffix}")
endif()
if(EXISTS "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc-ranlib${_stm32_host_exe_suffix}")
    set(CMAKE_C_COMPILER_RANLIB "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc-ranlib${_stm32_host_exe_suffix}")
    set(CMAKE_CXX_COMPILER_RANLIB "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc-ranlib${_stm32_host_exe_suffix}")
endif()

if(NOT DEFINED _STM32_CLT_STATUS_EMITTED)
    message(STATUS "Using STM32CubeCLT ${STM32_CLT_REQUIRED_VERSION} on ${_stm32_host_os}: ${STM32_CLT_ROOT}")
    set(_STM32_CLT_STATUS_EMITTED TRUE CACHE INTERNAL "STM32 toolchain status emitted" FORCE)
endif()

set(CMAKE_FIND_ROOT_PATH "${STM32_CLT_ROOT}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
