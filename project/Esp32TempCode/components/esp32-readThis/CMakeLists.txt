#idf_component_register(SRCS "esp32-readThis.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "esp32-readThis.c")
set(COMPONENT_PRIV_REQUIRES esp_adc_cal)
register_component()
