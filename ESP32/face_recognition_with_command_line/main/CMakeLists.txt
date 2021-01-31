set(COMPONENT_SRCS
    app_main.cpp
    app_facenet.c
    app_camera.cpp
    )

set(COMPONENT_ADD_INCLUDEDIRS include protocol_examples_common)

set(COMPONENT_REQUIRES
    esp32-camera
    esp-face
    main
    nvs_flash
    app_update
    esp-tls
    esp_http_client
    )

register_component()
