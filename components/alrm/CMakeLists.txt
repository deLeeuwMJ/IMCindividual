#idf_component_register(SRCS "alrm.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "alrm.c")
set(COMPONENT_PRIV_REQUIRES "clockHandler" "mainHandler" "sdcardrw" esp_peripherals input_key_service)

register_component()