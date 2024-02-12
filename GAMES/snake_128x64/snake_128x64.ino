#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DIYables_IRcontroller.h> // DIYables_IRcontroller library
#include <time.h>

#define IR_RECEIVER_PIN 12 // The Arduino pin connected to IR controller
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define IR_REMOTE_KEY_UP 17
#define IR_REMOTE_KEY_LEFT 20
#define IR_REMOTE_KEY_OK 21
#define IR_REMOTE_KEY_RIGHT 22
#define IR_REMOTE_KEY_DOWN 25

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DIYables_IRcontroller_17 irController(IR_RECEIVER_PIN, 200); // debounce time is 200ms

int old_command = NULL;
int command = NULL;

class snake {
  public:
    static const int DOWN = 1;
    static const int UP = -1;

    static const int RIGHT = 1;
    static const int LEFT = -1;

    int init_x = 64;
    int x = init_x;
    int init_y = 32;
    int y = init_y;
    int size = 2;
    int speed = 2;
    const int width = 4;

    void reset(){
      x = init_x;
      y = init_y;
      size = 2;
    }

    void move(int x_dir, int y_dir) {
      x += x_dir;
      y += y_dir;
    }

    /*void control_move(int command, int x_dir, int y_dir) {
      if (command == IR_REMOTE_KEY_UP && y_dir != snake::DOWN ) {
        y_dir = snake::UP;
        x_dir = 0;
      } else if (command == IR_REMOTE_KEY_DOWN && y_dir != snake::UP) {
        y_dir = snake::DOWN;
        x_dir = 0;
      } else if (command == IR_REMOTE_KEY_RIGHT && x_dir != snake::LEFT) {
        x_dir = snake::RIGHT;
        y_dir = 0;
      } else if (command == IR_REMOTE_KEY_LEFT && x_dir != snake::RIGHT) {
        x_dir = snake::LEFT;
        y_dir = 0;
      }
    }*/

    void draw() {
      display.drawRect(x, y, width, width, SSD1306_WHITE);
    }
};

class snack {
  public:

  long x = 20;
  long y = 20;
  int size = 2; 

  void randSnack() {
    x = random(128);
    y = random(64);

    Serial.println(x);
    Serial.println(y);
  }

  void draw() {
    display.drawCircle(x, y, size, SSD1306_WHITE);;
  }
};

snake snake_obj;
snack snack_obj;

bool collide(int snack_x, int snack_y, int snake_x, int snake_y) {

  if ((snake_x + 2) >= (snack_x - 1) && (snake_x - 2) <= (snack_x + 1)) {
    if ((snake_y + 2) >= (snack_y - 2) && (snake_y - 2) <= (snack_y + 2)) {
      return true;
    }
  }

  return false;
}

int x_dir = 0;
int y_dir = 0;

void setup() {
  Serial.begin(9600);
  irController.begin();
  randomSeed(analogRead(0));

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.display();
}

void loop() {
  old_command = command;
  command = irController.getKey();
  display.clearDisplay();

  if (collide(snack_obj.x, snack_obj.y, snake_obj.x, snake_obj.y)) {
    snack_obj.randSnack();
  }

  if (command == IR_REMOTE_KEY_UP && y_dir != snake::DOWN ) {
    y_dir = snake::UP;
    x_dir = 0;
  } else if (command == IR_REMOTE_KEY_DOWN && y_dir != snake::UP) {
    y_dir = snake::DOWN;
    x_dir = 0;
  } else if (command == IR_REMOTE_KEY_RIGHT && x_dir != snake::LEFT) {
    x_dir = snake::RIGHT;
    y_dir = 0;
  } else if (command == IR_REMOTE_KEY_LEFT && x_dir != snake::RIGHT) {
    x_dir = snake::LEFT;
    y_dir = 0;  
  }

  snack_obj.draw();

  snake_obj.move(x_dir, y_dir);
  snake_obj.draw();
  
  if (command == IR_REMOTE_KEY_OK) {
    snake_obj.reset();
    x_dir = 0;
    y_dir = 0; 
  }

  display.display();  
}