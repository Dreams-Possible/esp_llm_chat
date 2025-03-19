#all c file in this directory
file(GLOB SRCS "uart.c" "llm_chat.c" "main.c" "http.c" "littlefs.c" "wifi.c" "*.c")

#register component
idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS "."
)
