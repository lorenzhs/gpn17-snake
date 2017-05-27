#pragma once

template <typename T>
T max(const T& a, const T& b) {
    return a < b ? b : a;
}

#define WIDTH 42
#define HEIGHT 42

const int16_t SIZE = WIDTH * HEIGHT;

enum direction : char {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

enum field : char {
    EMPTY = 0,
    FOOD,
    SNAKE_RI,
    SNAKE_LE,
    SNAKE_UP,
    SNAKE_DN,
    SNAKE_END
};

field* data = nullptr;
int16_t head_pos, snake_len;
direction dir;

uint32_t start_time = 0;
uint32_t last_move = 0;
uint32_t target_fps = 5;
uint32_t frame_delay = 1000000 / target_fps; // .2 seconds initially

void draw_pixel(int16_t x, int16_t y, uint16_t colour) {
    tft.drawPixel(3*x,   3*y,   colour);
    tft.drawPixel(3*x,   3*y+1, colour);
    tft.drawPixel(3*x,   3*y+2, colour);
    tft.drawPixel(3*x+1, 3*y,   colour);
    tft.drawPixel(3*x+1, 3*y+1, colour);
    tft.drawPixel(3*x+1, 3*y+2, colour);
    tft.drawPixel(3*x+2, 3*y,   colour);
    tft.drawPixel(3*x+2, 3*y+1, colour);
    tft.drawPixel(3*x+2, 3*y+2, colour);
}

void set(int16_t x, int16_t y, field val) {
    data[x + y*WIDTH] = val;
    uint16_t colour = (val == EMPTY) ? BLACK : WHITE;
    draw_pixel(x, y, colour);
}

void set(int16_t pos, field val) {
    data[pos] = val;
    int16_t y = pos/WIDTH, x = pos - y * WIDTH;
    uint16_t colour = (val == EMPTY) ? BLACK : WHITE;
    draw_pixel(x, y, colour);
}

field get(int16_t x, int16_t y) {
    return data[x + y*WIDTH];
}

int16_t to_pos(int16_t x, int16_t y) {
    return x + y*WIDTH;
}

void set_target_fps(uint32_t fps) {
    target_fps = fps;
    frame_delay = 1000000 / target_fps;
}

int16_t offset(int16_t pos, direction dir) {
    switch(dir) {
    case DIR_UP: pos -= WIDTH; break;
    case DIR_DOWN: pos += WIDTH; break;
    case DIR_LEFT: if (pos % WIDTH == 0) pos += WIDTH; pos--; break;
    case DIR_RIGHT: pos++; if (pos % WIDTH == 0) pos -= WIDTH; break;
    }
    if (pos < 0) pos += (WIDTH * HEIGHT);
    if (pos >= WIDTH * HEIGHT) pos -= (WIDTH * HEIGHT);
    return pos;
}

int16_t walk_snake(int16_t pos) {
    int16_t res;
    switch(data[pos]) {
    case SNAKE_DN: res = offset(pos, DIR_DOWN); break;
    case SNAKE_UP: res = offset(pos, DIR_UP); break;
    case SNAKE_LE: res = offset(pos, DIR_LEFT); break;
    case SNAKE_RI: res = offset(pos, DIR_RIGHT); break;
    default: res = -1;
    }
    return res;
}

field dir_to_field(direction dir) {
    field f;
    switch(dir) {
    case DIR_UP: f = SNAKE_DN; break;
    case DIR_DOWN: f = SNAKE_UP; break;
    case DIR_LEFT: f = SNAKE_RI; break;
    case DIR_RIGHT: f = SNAKE_LE; break;
    }
    return f;
}

// check viability of new food placement
bool proximity_check(int16_t cand_pos) {
    if (data[cand_pos] != EMPTY ||
        data[offset(cand_pos, DIR_UP)] != EMPTY ||
        data[offset(cand_pos, DIR_DOWN)] != EMPTY ||
        data[offset(cand_pos, DIR_LEFT)] != EMPTY ||
        data[offset(cand_pos, DIR_RIGHT)] != EMPTY)
        return true;
    return false;
}

void place_new_food() {
    uint16_t cand;
    do {
        cand = random(0, SIZE);
    } while (proximity_check(cand));
    set(cand, FOOD);
}

void eat() {
    ++snake_len;
    // speed it up by 20%!
    if (snake_len % 5) {
        uint32_t fps = max(target_fps + 1,
                           static_cast<uint32_t>(1.2 * target_fps));
        set_target_fps(fps);
    }
}

void init_game() {
    if (data == nullptr) {
        data = new field[WIDTH * HEIGHT];
    }
    // clear state
    for (int16_t i = 0; i < WIDTH * HEIGHT; i++) {
        data[i] = EMPTY;
    }

    randomSeed(micros());
    set_target_fps(5);

    // blank the screen
    tft.fillScreen(BLACK);

    // place the snake
    int16_t head_x = WIDTH/4, head_y = HEIGHT/2;
    head_pos = to_pos(head_x, head_y);
    set(head_x,   head_y, SNAKE_LE);
    set(head_x-1, head_y, SNAKE_LE);
    set(head_x-2, head_y, SNAKE_END);
    snake_len = 3;
    dir = DIR_RIGHT;

    // add some food
    place_new_food();

    // reset timer
    last_move = micros();
}

void set_all_pixels(uint32_t colour, bool commit = true) {
    pixels.setPixelColor(0, colour);
    pixels.setPixelColor(1, colour);
    pixels.setPixelColor(2, colour);
    pixels.setPixelColor(3, colour);
    if (commit)
        pixels.show();
}

void reset_game(Badge &badge) {
    badge.setGPIO(VIBRATOR, HIGH);
    set_all_pixels(pixels.Color(100, 0, 0));

    delay(500);

    badge.setGPIO(VIBRATOR, LOW);
    set_all_pixels(pixels.Color(0, 0, 0));

    init_game();
}

// main game loop, called whenever the snake should move
void game_loop(Badge &badge) {
    int16_t new_head_pos = offset(head_pos, dir);
    bool grow = false;

    switch(data[new_head_pos]) {
    case FOOD: grow = true; eat(); place_new_food(); break;
    case SNAKE_UP:
    case SNAKE_DN:
    case SNAKE_LE:
    case SNAKE_RI:
    case SNAKE_END: reset_game(badge); return; /* break;*/
    default: /* empty */ break;
    }
    set(new_head_pos, dir_to_field(dir));

    if (!grow) {
        // Walk the snake and delete the last square
        int16_t last_pos = new_head_pos, pos = head_pos, new_pos;
        while (pos >= 0 && (new_pos = walk_snake(pos)) && new_pos >= 0) {
            last_pos = pos;
            pos = new_pos;
        }
        // new_pos < 0, pos and last_pos still hold old value
        if (last_pos >= 0) set(last_pos, SNAKE_END);
        if (pos >= 0) set(pos, EMPTY);
    }

    // update head position
    head_pos = new_head_pos;
}

// call this from loop()
void main_loop(Badge& badge) {
    start_time = micros();

    // process input
    delay(0);
    if (badge.getJoystickState() == JoystickState::BTN_ENTER) {
        set_all_pixels(pixels.Color(0, 30, 0));
        delay(100);
        last_move = micros();
        return;
    } else {
        set_all_pixels(pixels.Color(0, 0, 0), false);
    }

    if (badge.getJoystickState() == JoystickState::BTN_LEFT) {
        dir = DIR_LEFT;
        pixels.setPixelColor(1, pixels.Color(0, 0, 30));
    } else pixels.setPixelColor(1, pixels.Color(0, 0, 0));

    if (badge.getJoystickState() == JoystickState::BTN_UP) {
        dir = DIR_UP;
        pixels.setPixelColor(0, pixels.Color(0, 0, 30));
    } else pixels.setPixelColor(0, pixels.Color(0, 0, 0));

    if (badge.getJoystickState() == JoystickState::BTN_DOWN) {
        dir = DIR_DOWN;
        pixels.setPixelColor(3, pixels.Color(0, 0, 30));
    } else pixels.setPixelColor(3, pixels.Color(0, 0, 0));

    if (badge.getJoystickState() == JoystickState::BTN_RIGHT) {
        dir = DIR_RIGHT;
        pixels.setPixelColor(2, pixels.Color(0, 0, 30));
    } else pixels.setPixelColor(2, pixels.Color(0, 0, 0));


    if (last_move + frame_delay < start_time) {
        game_loop(badge);
        last_move = start_time;
    }

    pixels.show();

    // write framebuffer
    tft.writeFramebuffer();
}