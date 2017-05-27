#pragma once


// copied from sensor rom
int renderMenu(String names[],int arraySize) {

  int counter = 0;
  int menuAddition = 0;
  int textSize = 0;

  if (arraySize <= 7) textSize = 2;
  else if (arraySize > 7) textSize = 1;

  tft.setTextSize(textSize);
  tft.setTextColor(WHITE);
  tft.setFont();

  while (true) {
    if (badge.getJoystickState() == JoystickState::BTN_UP) {
      counter--;
      while (badge.getJoystickState() == JoystickState::BTN_UP) delay(0);
    } else if (badge.getJoystickState() == JoystickState::BTN_DOWN) {
      counter++;
      while (badge.getJoystickState() == JoystickState::BTN_DOWN) delay(0);
    }

    if (counter > arraySize) counter = 0;
    else if (counter < 0) counter = arraySize;

    tft.fillScreen(BLACK);

    for (int i = 0; i <= arraySize; i++) {
      if (counter == i) {
        tft.fillRect( 0 , (textSize * 11) * i + 26, 128, (textSize * 8), WHITE);
        tft.setTextColor(BLACK);
      } else tft.setTextColor(WHITE);
      tft.setCursor(5, (textSize * 11) * i + 26);
      tft.print(names[i]);
    }

    tft.setTextColor(WHITE);
    tft.setCursor(5, 5);
    tft.println("Play with:");

    tft.writeFramebuffer();
    delay(0);

    if (badge.getJoystickState() == JoystickState::BTN_ENTER) {
      tft.fillScreen(BLACK);
      tft.setCursor(5,(textSize * 8) * counter + 16);
      tft.print(names[counter]);
      tft.writeFramebuffer();
      while(badge.getJoystickState() == JoystickState::BTN_ENTER) delay(0);
      tft.fillScreen(BLACK);
      return counter;
    }
  }
}
