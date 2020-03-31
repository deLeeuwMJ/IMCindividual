#idf_component_register(SRCS "sdcardrw.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "sdcardrw.c")
set(COMPONENT_PRIV_REQUIRES "clockHandler" "menu" "mainHandler" esp_peripherals input_key_service)

register_component()