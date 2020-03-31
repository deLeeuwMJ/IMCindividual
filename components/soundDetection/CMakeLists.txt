#idf_component_register(SRCS "soundDetection.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "soundDetection.c")
set(COMPONENT_PRIV_REQUIRES "mainHandler" "menu" "clockHandler" "goertzel" esp_peripherals input_key_service)

register_component()