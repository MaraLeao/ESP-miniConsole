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
const int r_button = 14;
const int l_button = 12;

int r_state_btn = 1;
int l_state_btn = 1;

const int dirDOWN = 1;
const int dirUP = -1;

const int dirRIGHT = 1;
const int dirLEFT = -1;

int score = 0;
int best_score = 0;
clock_t timeCollide = 0;

class ball {
  public:
    int radius = 3;
    int w = 3;
    int h = 3;
    float x = 64;
    float y = 45;
    int init_speed = 2;
    int dir_vertical = dirUP;
    int dir_horizontal = dirLEFT;
    float speed = init_speed;

    void reset() {
      x = 64;
      y = 45;
      speed = init_speed;
    }

    void move() {
      if (score >= 3) {
        speed += 0.001;
      }
      
      if (x >= (SCREEN_WIDTH - radius)) {
        dir_horizontal = dirLEFT;
      } else if(x <= 0) {
        dir_horizontal = dirRIGHT;
      }

      if (y < 0) {
        dir_vertical = dirDOWN;
      }

      x += (dir_horizontal * speed);
      y += (dir_vertical * speed);
    }

    void draw() {
      display.drawCircle(x, y, radius, SSD1306_WHITE);
    }
};

class paddle {
  public:
    
    int init_x = 54;
    int x = init_x;
    int y = 50;
    int width = 23;
    int height = 4;

    void reset() {
      x = 54;
    }

    void move(int command) {

      //r_state_btn = digitalRead(r_button);
      //l_state_btn = digitalRead(l_button);

      if(x >= 106) {
        x = 105;
      }
      if (x <= 0) {
        x = 0;
      }

      if (command == IR_REMOTE_KEY_RIGHT) {
        x += 25;
      }
      if (command == IR_REMOTE_KEY_LEFT) {
        x -= 25;
      }
  
      /*if (!r_state_btn) {
        x += 5;//move paddle for right
      }
      if (!l_state_btn) {
        x -= 5; //move paddle for left
      }*/
    }

    void draw () {
      display.drawRect(x, 50, 20, 4, SSD1306_WHITE);
    }

    bool collide(ball& ball_obj) {
      if (ball_obj.y >= 47 && ball_obj.y <= (y + height)) {
        //paddle collide left and right
        if(ball_obj.x == (x - ball_obj.radius)) {
          ball_obj.dir_horizontal = dirLEFT;
          ball_obj.dir_vertical = dirUP;
          return true;
        } else if (ball_obj.x == (x + width)){
          ball_obj.dir_horizontal = dirRIGHT;
          ball_obj.dir_vertical = dirUP;
          return true;
        } else if(ball_obj.x >= x && ball_obj.x <= (x + width)) {//paddle collide up
          ball_obj.dir_vertical = dirUP;
          return true;
        } 
      }
      return false;
    }
};

void drawCentreString(const char *buf, int x, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
  display.setCursor(x - w / 2, y);
  display.println(buf);
}

int mode = 1;

class menu {
  public:
    static const int pause = 0;
    static const int start_init = 1;
    static const int game_over = 2;
    static const int in_game = 3;
    static const int new_record = 4;

    void status(int mode) {

      switch (mode) {
        case menu::pause:
          drawCentreString("PAUSE", 64, 32);
          break;
        case menu::start_init:
          drawCentreString("PRESS OK", 64, 32);
          break;
        case menu::game_over:
          drawCentreString("GAME OVER", 64, 32);
          break;
        case menu::new_record:
          drawCentreString("NEW RECORD", 64, 25);
          drawCentreString(String(best_score).c_str(), 64, 40);
          break;
      }
    }
};

void setup() {
  Serial.begin(9600);
  irController.begin();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //pinMode(r_button, INPUT);
  //pinMode(l_button, INPUT);
  display.clearDisplay();
  display.display();
}

menu MENU;
paddle paddle_obj;
ball ball_obj;

int old_command = NULL;
int command = NULL;

void loop() {
  old_command = command;
  command = irController.getKey();
  display.clearDisplay();

  //r_state_btn = digitalRead(r_button);
  //l_state_btn = digitalRead(l_button);

  switch (mode) {
    case menu::start_init:
      MENU.status(mode);

      if (command == IR_REMOTE_KEY_OK && old_command != IR_REMOTE_KEY_OK) {
        ball_obj.reset();
        paddle_obj.reset();
        ball_obj.dir_vertical = dirUP;
        score = 0;
        mode = menu::in_game;
      }
      break;

    case menu::in_game:
      ball_obj.move();
      ball_obj.draw();
      paddle_obj.move(command);
      paddle_obj.draw();
      drawCentreString(String(score).c_str(), 100, 5);

      if (paddle_obj.collide(ball_obj) && timeCollide < clock()) {
        timeCollide = clock() + 500;
        score++;
      }

      if(ball_obj.y >= 64) {
        mode = menu::game_over;
      }

      if (command == IR_REMOTE_KEY_OK && old_command != IR_REMOTE_KEY_OK) {
        mode = menu::pause;
      }
      break;

    case menu::pause:
      MENU.status(mode);
      paddle_obj.draw();
      ball_obj.draw();
      drawCentreString(String(score).c_str(), 100, 5);

      if (command == IR_REMOTE_KEY_OK && old_command != IR_REMOTE_KEY_OK) {      
        mode = menu::in_game;
      }
      break;

    case menu::game_over:
      MENU.status(mode);

      if (command == IR_REMOTE_KEY_OK && old_command != IR_REMOTE_KEY_OK) {

        if (score > best_score) {
          best_score = score;
          mode = menu::new_record;
          break;
        }

        mode = menu::start_init;
      }
      break;
    
    case menu::new_record:
      MENU.status(mode);
      if (command == IR_REMOTE_KEY_OK && old_command != IR_REMOTE_KEY_OK) {      
        mode = menu::start_init;
      }
  }

  display.display();  
}



