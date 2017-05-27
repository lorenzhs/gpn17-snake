#include <GPNBadge.hpp>
#include <FS.h>

#include "rboot.h"
#include "rboot-api.h"
#include "snake.h"

Badge badge;

// omfg include here :O (needs Badge object)
#include "menu.h"

Snake snake(badge);

String titles[2] = {"Gyroscope", "Joystick"};

void setup() {
    badge.init();
    SPIFFS.begin();

    badge.setBacklight(true);
    tft.fillScreen(BLACK);
    tft.writeFramebuffer();

    badge.setGPIO(IR_EN, LOW);
    badge.setAnalogMUX(MUX_JOY);

    rboot_config rboot_config = rboot_get_config();
    File f = SPIFFS.open("/rom" + String(rboot_config.current_rom), "w");
    f.println("Snake\n");
    f.close();

    int choice = renderMenu(titles, (sizeof(titles) / sizeof(*titles)) - 1);
    if (choice == 0)
        snake.init_game(/* use_gyro */ true);
    else
        snake.init_game(/* use_gyro */ false);

    delay(300);
}

void loop() {
    snake.main_loop();
}
