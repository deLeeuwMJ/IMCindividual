#idf_component_register(SRCS "mqttReader.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "mqttReader.c")
set(COMPONENT_PRIV_REQUIRES nvs_flash esp_peripherals mqtt "menu" "mainHandler")
register_component()