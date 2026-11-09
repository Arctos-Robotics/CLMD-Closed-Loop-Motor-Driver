idf_component_register(SRCS "main.c" "core.c" "current.c" "hardware.c" "settings.c" "mt6701.c" "mt6701_codec.c" "joint.c" "joint_storage.c"
    "tmc2209_advanced.c" INCLUDE_DIRS "." REQUIRES driver esp_adc nvs_flash esp_timer)
