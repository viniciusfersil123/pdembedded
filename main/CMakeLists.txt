idf_component_register(SRCS "pdembedded.cpp"
    INCLUDE_DIRS "." "drivers/uda1334a" "build/c" "build/hv" "build/ir"
)

set_source_files_properties(c/HvMessage.c PROPERTIES COMPILE_FLAGS "-Wno-error=format")
