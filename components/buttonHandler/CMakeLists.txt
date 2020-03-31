#idf_component_register(SRCS "buttonHandler.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "buttonHandler.c")
set(COMPONENT_PRIV_REQUIRES "rotaryEncoder" "mainHandler" "radio" esp_peripherals input_key_service)

register_component()