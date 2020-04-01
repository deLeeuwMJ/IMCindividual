#idf_component_register(SRCS "rotaryLED.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "rotaryLED.c")
set(COMPONENT_PRIV_REQUIRES "rotaryEncoder" "mainHandler" "clockHandler" esp_peripherals input_key_service)

register_component()