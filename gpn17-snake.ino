#include <GPNBadge.hpp>
#include <FS.h>

#include "rboot.h"
#include "rboot-api.h"
#include "snake.h"

Badge badge;


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

    init_game();

    delay(300);
}

void loop() {
    main_loop(badge);
}
