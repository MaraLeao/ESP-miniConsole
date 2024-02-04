/**************************************************************************
 This is an example for our Monochrome OLEDs based on SSD1306 drivers

 Pick one up today in the adafruit shop!
 ------> http://www.adafruit.com/category/63_98

 This example is for a 128x64 pixel display using I2C to communicate
 3 pins are required to interface (two I2C and one reset).

 Adafruit invests time and resources providing this open
 source code, please support Adafruit and open-source
 hardware by purchasing products from Adafruit!

 Written by Limor Fried/Ladyada for Adafruit Industries,
 with contributions from the open source community.
 BSD license, check license.txt for more information
 All text above, and the splash screen below must be
 included in any redistribution.
 **************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DIYables_IRcontroller.h> // DIYables_IRcontroller library

#define IR_RECEIVER_PIN 12 // The Arduino pin connected to IR controller
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define IR_REMOTE_KEY_UP 17
#define IR_REMOTE_KEY_LEFT 20
#define IR_REMOTE_KEY_OK 21
#define IR_REMOTE_KEY_RIGHT 22
#define IR_REMOTE_KEY_DOWN 25

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
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

class ball {
  public:
    int radius = 3;
    int w = 3;
    int h = 3;
    int x = 64;
    int y = 45;
    int init_speed = 2;
    int dir_vertical = dirDOWN;
    int dir_horizontal = dirLEFT;

    void reset() {
      x = 64;
      y = 45;
    }

    void move() {
      display.drawCircle(x, y, radius, SSD1306_WHITE);

      int speed = init_speed;
      
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
};

class paddle {
  public:
    
    int init_x = 54;
    int x = init_x;
    int y = 50;
    int width = 23;
    int height = 4;
    int score = 0;
    int max_score = 0;

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
        x += 20;
      }
      if (command == IR_REMOTE_KEY_LEFT) {
        x -= 20;
      }
  
      /*if (!r_state_btn) {
        x += 5;//move paddle for right
      }
      if (!l_state_btn) {
        x -= 5; //move paddle for left
      }*/
    }

      void collide(ball* ball_obj) {
        //paddle collide up
        if (ball_obj->y >= 47 && ball_obj->y <= (y + height)) {
          if(ball_obj->x >= x && ball_obj->x <= (x + width)) {
            ball_obj->dir_vertical = dirUP;
          } 
        }
        //paddle collide left and right
        if(ball_obj->y >= y && ball_obj->y <= (y + height)) {

          if(ball_obj->x == (x - ball_obj->radius)) {
            ball_obj->dir_horizontal = dirLEFT;
            ball_obj->dir_vertical = dirUP;

          } else if (ball_obj->x == (x + width)){
            ball_obj->dir_horizontal = dirRIGHT;
            ball_obj->dir_vertical = dirUP;
          }

        if (ball_obj->dir_vertical == dirUP && ball_obj->y > 45) {
          score++;
        }
      }
      
      display.drawRect(x, 50, 20, 4, SSD1306_WHITE);
    }
};

void drawCentreString(const char *buf, int x, int y)
{
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

    void status(int mode) {

      if(mode == 0) {
        drawCentreString("PAUSE", 64, 32);
        drawCentreString("Press OK", 64, 45);
      } else if (mode == 1) {
        drawCentreString("Press OK", 64, 32);
      } else {
        drawCentreString("GAME OVER", 64, 32);
        drawCentreString("Press OK", 64, 45);
      }
      
      /*switch(mode) {
        case 0:
          drawCentreString("PAUSE", 64, 32);
          drawCentreString("Press any button for start", 64, 32);
          break;
        case 1:
          drawCentreString("Press any button for start", 64, 32);
          break;
        case 2:
          drawCentreString("GAME OVER", 64, 32);
          drawCentreString("Press any button for start", 64, 32);
          break;
      }*/
    }
};

/*void ir_control_test(int command) {
  switch (command) {
    case IR_REMOTE_KEY_UP:
      Serial.println("UP");
      break;

    case IR_REMOTE_KEY_DOWN:
      Serial.println("DOWN");
      break;

    case IR_REMOTE_KEY_LEFT:
      Serial.println("LEFT");
      break;

    case IR_REMOTE_KEY_RIGHT:
      Serial.println("RIGHT");
      break;

    case IR_REMOTE_KEY_OK:
      Serial.println("OK");
      break;

    default:
      Serial.println("WARNING: undefined command:");
      break;
  }

}*/

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
  
  paddle paddle_obj;
  ball ball_obj;

  display.clearDisplay();
  ball_obj.reset();
  paddle_obj.reset();
}


menu MENU;
paddle paddle_obj;
ball ball_obj;

void loop() {
  int command = irController.getKey();

  //r_state_btn = digitalRead(r_button);
  //l_state_btn = digitalRead(l_button);

  if (mode == menu::start_init) {
    if (command == IR_REMOTE_KEY_OK) {
      mode = menu::in_game;
    }
  } else if (mode == menu::in_game) {
    display.clearDisplay();
    ball_obj.move();
    paddle_obj.move(command);
    paddle_obj.collide(&ball_obj);

    if(ball_obj.y >= 64) {
      mode = menu::game_over;
    }

    if (command == IR_REMOTE_KEY_OK) {
      mode = menu::pause;
    }
    drawCentreString(String(paddle_obj.score).c_str(), 100, 5);

    display.display(); 

  } else {
    display.clearDisplay();
    MENU.status(mode);
    if (command == IR_REMOTE_KEY_OK) {
      if (mode == menu::game_over) {
        ball_obj.reset();
        paddle_obj.reset();
      }
      mode = menu::in_game;
    }
    display.display();  
  }
}



