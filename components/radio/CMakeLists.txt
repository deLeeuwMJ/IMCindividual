#idf_component_register(SRCS "radio.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "radio.c")
set(COMPONENT_PRIV_REQUIRES "menu" nvs_flash audio_pipeline mainHandler audio_stream esp_peripherals)
register_component()