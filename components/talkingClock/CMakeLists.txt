#idf_component_register(SRCS "talkingClock.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "talkingClock.c")
set(COMPONENT_PRIV_REQUIRES "clockHandler" "mainHandler" audio_board audio_hal audio_sal esp_peripherals audio_stream)
register_component()
