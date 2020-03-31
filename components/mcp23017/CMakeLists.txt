#idf_component_register(SRCS "mcp23017.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "mcp23017.c")
set(COMPONENT_REQUIRES esp_peripherals input_key_service)
register_component()