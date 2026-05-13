file(GLOB HEAVY_GENERATED_SRCS
    "../.main/output/c/*.c"
    "../.main/output/c/*.cpp")

idf_component_register(SRCS "pdembedded.cpp"
                    "sine.cpp"
                    "per/adc.cpp"
                    "drivers/uda1334a/uda1334a.cpp"
                    ${HEAVY_GENERATED_SRCS}
                    INCLUDE_DIRS "." "drivers/uda1334a" "../.main/output/c" "per"
                    REQUIRES driver)

set_source_files_properties(c/HvMessage.c PROPERTIES COMPILE_FLAGS "-Wno-error=format")
set_source_files_properties(../.main/output/c/HvMessage.c PROPERTIES COMPILE_FLAGS "-Wno-error=format")
