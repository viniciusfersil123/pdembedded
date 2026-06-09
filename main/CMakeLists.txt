idf_component_register(SRCS "pdembedded.cpp"
                    "build/c/HeavyContext.cpp"
                    "build/c/Heavy_heavy.cpp"
                    "build/c/HvLightPipe.c"
                    "build/c/HvMessage.c"
                    "build/c/HvMessagePool.c"
                    "build/c/HvMessageQueue.c"
                    "build/c/HvSignalPhasor.c"
                    "build/c/HvSignalVar.c"
                    "build/c/HvTable.c"
                    "build/c/HvUtils.c"
                    "build/c/HvHeavy.cpp"
                    "drivers/uda1334a/uda1334a.cpp"
                    INCLUDE_DIRS "." "drivers" "drivers/uda1334a" "build/c" "build/hv" "build/ir"
                    REQUIRES driver)

set_source_files_properties(build/c/HvMessage.c PROPERTIES COMPILE_FLAGS "-Wno-error=format")
