#pragma once

template <typename T>
T max(const T& a, const T& b) {
    return a < b ? b : a;
}

class Snake {
public:
    Snake(Badge &badge_) : badge(badge_) {}
    // Game initialisation, call this from setup()
    void init_game();
    // Game loop, call this from loop()
    void main_loop();

private:
const int16_t WIDTH = 42, HEIGHT = 42;
const int16_t SIZE = WIDTH * HEIGHT;

// Movement directions
enum direction : char {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

// Possible states of a square on the map
enum field : char {
    EMPTY = 0,
    FOOD,
    SNAKE_RI,
    SNAKE_LE,
    SNAKE_UP,
    SNAKE_DN,
    SNAKE_END
};

// The badge object, for polling joystick and GPIO (vibrator)
Badge &badge;

// The game data
field* data = nullptr;

// position of the snake's head and its length as well as direction of movement
int16_t head_pos, snake_len;
direction dir;

// Keep track of timing
uint32_t start_time = 0;
uint32_t last_move = 0;
uint32_t frame_delay;

void draw_square(int16_t x, int16_t y) {
    field& f = data[to_pos(x, y)];
    if (f == FOOD)
        draw_food(x, y);
    else
        draw_solid(x, y, (f == EMPTY) ? BLACK : WHITE);
}

// Draw a square of the game board (3x3 pixels on the screen)
void draw_solid(int16_t x, int16_t y, uint16_t colour) {
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

// Draw food item in a diamond shape
void draw_food(int16_t x, int16_t y) {
    tft.drawPixel(3*x,   3*y,   BLACK);
    tft.drawPixel(3*x,   3*y+1, WHITE);
    tft.drawPixel(3*x,   3*y+2, BLACK);
    tft.drawPixel(3*x+1, 3*y,   WHITE);
    tft.drawPixel(3*x+1, 3*y+1, BLACK);
    tft.drawPixel(3*x+1, 3*y+2, WHITE);
    tft.drawPixel(3*x+2, 3*y,   BLACK);
    tft.drawPixel(3*x+2, 3*y+1, WHITE);
    tft.drawPixel(3*x+2, 3*y+2, BLACK);
}

// Set a square on the map and draw it on the screen, indexed by (x,y) coords
void set(int16_t x, int16_t y, field val) {
    data[x + y*WIDTH] = val;
    draw_square(x, y);
}

// Set a square on the map and draw it on the screen, indexed by data position
void set(int16_t pos, field val) {
    data[pos] = val;
    int16_t y = pos/WIDTH, x = pos - y * WIDTH;
    draw_square(x, y);

}

field get(int16_t x, int16_t y) {
    return data[x + y*WIDTH];
}

// Convert between (x,y) coordinates and map positions
int16_t to_pos(int16_t x, int16_t y) {
    return x + y*WIDTH;
}

// Get the position of the field {left,right,above,below} that of `pos`
int16_t offset(int16_t pos, direction dir) {
    switch(dir) {
    case DIR_UP: pos -= WIDTH; if (pos < 0) pos += SIZE; break;
    case DIR_DOWN: pos += WIDTH; if (pos >= SIZE) pos -= SIZE; break;
    case DIR_LEFT: if (pos % WIDTH == 0) pos += WIDTH; pos--; break;
    case DIR_RIGHT: pos++; if (pos % WIDTH == 0) pos -= WIDTH; break;
    }
    return pos;
}

// Move one step along the snake (from head to tail)
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

// Get the field type of the snake's new head
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

direction reverse(direction dir) {
    direction res;
    switch(dir) {
    case DIR_UP: res = DIR_DOWN; break;
    case DIR_DOWN: res = DIR_UP; break;
    case DIR_LEFT: res = DIR_RIGHT; break;
    case DIR_RIGHT: res = DIR_LEFT; break;
    }
    return res;
}

// check viability of new food placement (i.e. not neighbouring snake)
bool proximity_check(int16_t cand_pos) {
    if (data[cand_pos] != EMPTY ||
        data[offset(cand_pos, DIR_UP)] != EMPTY ||
        data[offset(cand_pos, DIR_DOWN)] != EMPTY ||
        data[offset(cand_pos, DIR_LEFT)] != EMPTY ||
        data[offset(cand_pos, DIR_RIGHT)] != EMPTY)
        return true;
    return false;
}

// Pick a random field that isn't snake-adjacent and place food on it
void place_new_food() {
    int16_t cand;
    do {
        cand = random(0, SIZE);
    } while (proximity_check(cand));
    set(cand, FOOD);
}

// Handle eating
void eat() {
    ++snake_len;
    // 4% speedup per food item
    frame_delay /= 1.04;
}

// Set all LEDS to the same colour
void set_all_LEDs(uint32_t colour, bool commit = true) {
    pixels.setPixelColor(0, colour);
    pixels.setPixelColor(1, colour);
    pixels.setPixelColor(2, colour);
    pixels.setPixelColor(3, colour);
    if (commit)
        pixels.show();
}

// Player lost -> vibrate, flash LEDs, and reset the game
void reset_game(Badge &badge) {
    badge.setGPIO(VIBRATOR, HIGH);
    set_all_LEDs(pixels.Color(100, 0, 0));

    delay(500);

    badge.setGPIO(VIBRATOR, LOW);
    set_all_LEDs(pixels.Color(0, 0, 0));

    init_game();
}

// main game loop, called whenever the snake should move
void game_loop() {
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

};

void Snake::init_game() {
    if (data == nullptr) {
        data = new field[WIDTH * HEIGHT];
    }
    // clear state
    for (int16_t i = 0; i < WIDTH * HEIGHT; i++) {
        data[i] = EMPTY;
    }

    randomSeed(micros());
    frame_delay = 1000000 / 8; // 8 fps

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


// call this from loop()
void Snake::main_loop() {
    start_time = micros();

    // process input
    delay(0);

    JoystickState joystick = badge.getJoystickState();
    direction new_dir = dir;
    set_all_LEDs(pixels.Color(0, 0, 0), false);
    switch(joystick) {
    case JoystickState::BTN_ENTER:
        set_all_LEDs(pixels.Color(0, 30, 0), false);
        delay(100);
        last_move = micros();
        return; //!
        break;
    case JoystickState::BTN_UP:
        new_dir = DIR_UP;
        pixels.setPixelColor(0, pixels.Color(0, 0, 30));
        break;
    case JoystickState::BTN_LEFT:
        new_dir = DIR_LEFT;
        pixels.setPixelColor(1, pixels.Color(0, 0, 30));
        break;
    case JoystickState::BTN_DOWN:
        new_dir = DIR_DOWN;
        pixels.setPixelColor(3, pixels.Color(0, 0, 30));
        break;
    case JoystickState::BTN_RIGHT:
        new_dir = DIR_RIGHT;
        pixels.setPixelColor(2, pixels.Color(0, 0, 30));
        break;
    default: break; // JoystickState::BTN_NOTHING
    }

    if (new_dir == reverse(dir)) {
        // no reversing!
        set_all_LEDs(pixels.Color(30, 30, 0), false);
    } else {
        dir = new_dir;
    }

    if (last_move + frame_delay < start_time) {
        game_loop();
        last_move = start_time;
    }

    pixels.show();

    // write framebuffer
    tft.writeFramebuffer();
}
