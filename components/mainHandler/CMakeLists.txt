#idf_component_register(SRCS "mainHandler.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "mainHandler.c")
set(COMPONENT_PRIV_REQUIRES esp_peripherals input_key_service nvs_flash)

register_component()