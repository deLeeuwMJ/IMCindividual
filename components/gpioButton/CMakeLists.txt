#idf_component_register(SRCS "gpioButton.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "gpioButton.c")
set(COMPONENT_REQUIRES  "mcp23017" "mainHandler" "sdcardrw" "menu" nvs_flash esp_peripherals input_key_service)
register_component()