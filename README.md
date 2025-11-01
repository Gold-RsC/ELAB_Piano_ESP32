# Piano_ESP32

## 项目简介

详见论文

## 硬件需求

- 使用esp32-wroom-32e
- 含有TTP229、SSD1306驱动的OLED、WS2812B、无源蜂鸣器

## 项目结构

Piano_ESP32/
├── main/
│   ├── CMakeLists.txt
│   ├── include/
│   │   ├── Base.hpp
│   │   ├── Buzzer.hpp
│   │   ├── font.hpp
│   │   ├── OLED.hpp
│   │   ├── TTP229.hpp
│   │   └── WS2812B.hpp
│   └── src/
│       ├── Base.cpp
│       ├── Buzzer.cpp
│       ├── main.cpp
│       ├── OLED.cpp
│       ├── TTP229.cpp
│       └── WS2812B.cpp
├── CMakeLists.txt
├── sdkconfig
├── .gitignore
├── .clangd
└── README.md

## 联系方式

- 作者：Aursc
- B站：Csrua
- QQ：310106329
