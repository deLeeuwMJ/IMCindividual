#idf_component_register(SRCS "inputHandler.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "inputHandler.c")
set(COMPONENT_PRIV_REQUIRES "rotaryEncoder" "mainHandler" "radio" esp_peripherals input_key_service)

register_component()