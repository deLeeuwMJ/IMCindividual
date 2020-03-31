#idf_component_register(SRCS "clockHandler.c" INCLUDE_DIRS include)

set(COMPONENT_ADD_INCLUDEDIRS include)
set(COMPONENT_SRCS "clockHandler.c")
set(COMPONENT_PRIV_REQUIRES "timesync" "talkingClock" "menu" buttonHandler "mainHandler" audio_board audio_hal audio_sal esp_peripherals audio_stream input_key_service) 
register_component()