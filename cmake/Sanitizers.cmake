function(enable_sanitizers TARGET_NAME)
    if(NOT MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )
        target_link_options(${TARGET_NAME} PRIVATE
            -fsanitize=address
            -fsanitize=undefined
        )
    endif()
endfunction()
