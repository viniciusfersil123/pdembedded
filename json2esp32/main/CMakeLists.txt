idf_component_register(SRCS "json2esp32.cpp"
                    "c/HeavyContext.cpp"
                    "c/Heavy_heavy.cpp"
                    "c/HvLightPipe.c"
                    "c/HvMessage.c"
                    "c/HvMessagePool.c"
                    "c/HvMessageQueue.c"
                    "c/HvSignalPhasor.c"
                    "c/HvSignalVar.c"
                    "c/HvTable.c"
                    "c/HvUtils.c"
                    "drivers/uda1334a/uda1334a.cpp"
                    INCLUDE_DIRS "." "drivers/uda1334a" "c" "hv" "ir"
                    REQUIRES driver)

set_source_files_properties(c/HvMessage.c PROPERTIES COMPILE_FLAGS "-Wno-error=format")
