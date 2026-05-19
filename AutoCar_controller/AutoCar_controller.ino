/*****************************************************
 * Tank_4Motor_TimeControl_FIXED.ino
 * ---------------------------------
 * 坦克履帶時間式位移控制（無 Encoder）
 * 修正：
 * - 低 PWM 不動
 * - 停止時滑動
 * - 左右速度不一致
 *****************************************************/

#include <Arduino.h>

/* ================== 腳位設定（完全沿用你原本） ================== */
#define M0     11
#define DIR0   2

#define M1     3
#define DIR1   12

#define M2     5
#define DIR2   8

#define M3     6
#define DIR3   7

/* ================== 參數設定 ================== */
#define MIN_PWM   70
float SCALE_LEFT  = 1.00;  // 左履帶補償
float SCALE_RIGHT = 1.00;  // 右履帶補償（依實測微調）

// ⭐ 獨立儲存前後左右的「左右輪」PWM（已還原為先前的預設數值）
int pwm_f_l = 80;   // 原本 move_forward_time 的左輪：75
int pwm_f_r = 138;  // 原本 move_forward_time 的右輪：-138 (取絕對值 138)
int pwm_b_l = 138;  // 原本 move_backward_time 的左輪：-138 (取絕對值 138)
int pwm_b_r = 75;   // 原本 move_backward_time 的右輪：75
int pwm_l_l = 60;   // 原本 turn_left_time 的左輪：-40 (取絕對值 40)
int pwm_l_r = 60;   // 原本 turn_left_time 的右輪：-40 (取絕對值 40)
int pwm_r_l = 205;  // 原本 turn_right_time 的左輪：180
int pwm_r_r = 205;  // 原本 turn_right_time 的右輪：150

// 4 顆馬達的「DIR=HIGH 是否代表 forward」
// 如果某顆方向反了，就把該顆的 FORWARD_* 由 true/false 互換
const bool FORWARD_DIR0_HIGH = false;
const bool FORWARD_DIR1_HIGH = false;
const bool FORWARD_DIR2_HIGH = false;
const bool FORWARD_DIR3_HIGH = false;

inline void set_motor(int pwm, int M, int DIR, bool forward_high)
{
  int p = constrain(abs(pwm), 0, 255);
  bool forward = (pwm >= 0);

  bool dir_level = forward ? forward_high : !forward_high;
  digitalWrite(DIR, dir_level ? HIGH : LOW);
  analogWrite(M, p);
}

void Tank_SetSpeed(int pwm_left, int pwm_right, bool DIR1BL , bool DIR2BL, int pwm_delay)
{
  // ===== 左右履帶速度補償 =====
  int pwm_l = pwm_left  * SCALE_LEFT;
  int pwm_r = pwm_right * SCALE_RIGHT;

  // ===== 左履帶：M0 + M2 =====
  set_motor(pwm_l, M0, DIR0, DIR1BL);
  set_motor(pwm_l, M2, DIR2, DIR1BL);
  delay(pwm_delay);
  // ===== 右履帶：M1 + M3 =====
  set_motor(pwm_r, M1, DIR1, DIR2BL);
  set_motor(pwm_r, M3, DIR3, DIR2BL);
}

/* ================== 主動煞車停止 ================== */
void Tank_Stop()
{
    // Brake：方向固定，PWM = 0
    digitalWrite(DIR0, LOW);
    digitalWrite(DIR1, LOW);
    digitalWrite(DIR2, LOW);
    digitalWrite(DIR3, LOW);

    analogWrite(M0, 0);
    analogWrite(M1, 0);
    analogWrite(M2, 0);
    analogWrite(M3, 0);

    delay(30);  // ⭐ 給驅接器時間吃掉慣性
}

/* ================== 位移 API ================== */

void move_forward_time(unsigned long ms, int pwm_left, int pwm_right)
{
    Tank_SetSpeed(pwm_left, -pwm_right, false, false, 150);
//    delay(ms);
//    Tank_Stop();
}

void move_backward_time(unsigned long ms, int pwm_left, int pwm_right)
{
    Tank_SetSpeed(-pwm_left, pwm_right, false, false, 50);
//    delay(ms);
//    Tank_Stop();
}

void turn_left_time(unsigned long ms, int pwm_left, int pwm_right)
{
    Tank_SetSpeed(-pwm_left, -pwm_right, false, false, 0);
    delay(ms);
    Tank_Stop();
}

void turn_right_time(unsigned long ms, int pwm_left, int pwm_right)
{
    Tank_SetSpeed(pwm_left, pwm_right, false, false, 0);
    delay(ms);
    Tank_Stop();
}

/* ================== Demo ================== */

void demo_path()
{
    delay(2000);
    // 90:1000:75:x
    turn_right_time(2000, 80, 80);
//    
    delay(2000);
    // 1000ms:x:6150:280
//    move_forward_time(3150, pwm_f_l, pwm_f_r);   // ⭐ PWM < 70 不要用
//    
//    delay(2000);
//    turn_right_time(500, 80, 80);
//    
//    delay(2000);
//    move_backward_time(4750, pwm_b_l, pwm_b_r);   // ⭐ PWM < 70 不要用
//
//    delay(2000);
//    turn_right_time(580, 80, 80);
//
//     delay(2000);
//    // 1000ms:380:x:2400
//    move_forward_time(6280, pwm_f_l, pwm_f_r);   // ⭐ PWM < 70 不要用
//
//    delay(2000);
    // 90:1000:75:x
//    turn_left_time(580, 80, 80);

//    delay(2000);
//    move_backward_time(4750, pwm_b_l, pwm_b_r);   // ⭐ PWM < 70 不要用
    Tank_Stop();
}

/* ================== Arduino ================== */

void setup()
{
    Serial.begin(9600);
    Serial.println("Arduino ready");
  
    cli();  // 關閉中斷（避免 Timer 抖動）

    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
    digitalWrite(M2, LOW);
    digitalWrite(M3, LOW);

    digitalWrite(DIR0, LOW);
    digitalWrite(DIR1, LOW);
    digitalWrite(DIR2, LOW);
    digitalWrite(DIR3, LOW);

    pinMode(M0, OUTPUT);
    pinMode(M1, OUTPUT);
    pinMode(M2, OUTPUT);
    pinMode(M3, OUTPUT);

    pinMode(DIR0, OUTPUT);
    pinMode(DIR1, OUTPUT);
    pinMode(DIR2, OUTPUT);
    pinMode(DIR3, OUTPUT);

    sei();  // 開中斷
}

void loop()
{
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim();
    Serial.println("RX:");
    Serial.println(msg);
  
    int spaceIdx = msg.indexOf(' ');
    String cmd = msg;
    int value1 = 0;
    int value2 = 0;
    bool hasValue2 = false;
    
    if (spaceIdx != -1) {
      cmd = msg.substring(0, spaceIdx);
      String remain = msg.substring(spaceIdx + 1);
      remain.trim();
      int spaceIdx2 = remain.indexOf(' ');
      
      if (spaceIdx2 != -1) {
        value1 = remain.substring(0, spaceIdx2).toInt();
        value2 = remain.substring(spaceIdx2 + 1).toInt();
        hasValue2 = true;
      } else {
        value1 = remain.toInt();
      }
    } else {
      value1 = msg.toInt();
    }

    if (cmd == "pwm_f") {
      pwm_f_l = value1; pwm_f_r = hasValue2 ? value2 : value1;
      Serial.print("Set Forward PWM -> L:"); Serial.print(pwm_f_l); Serial.print(" R:"); Serial.println(pwm_f_r);
    } else if (cmd == "pwm_b") {
      pwm_b_l = value1; pwm_b_r = hasValue2 ? value2 : value1;
      Serial.print("Set Backward PWM -> L:"); Serial.print(pwm_b_l); Serial.print(" R:"); Serial.println(pwm_b_r);
    } else if (cmd == "pwm_l") {
      pwm_l_l = value1; pwm_l_r = hasValue2 ? value2 : value1;
      Serial.print("Set Left PWM -> L:"); Serial.print(pwm_l_l); Serial.print(" R:"); Serial.println(pwm_l_r);
    } else if (cmd == "pwm_r") {
      pwm_r_l = value1; pwm_r_r = hasValue2 ? value2 : value1;
      Serial.print("Set Right PWM -> L:"); Serial.print(pwm_r_l); Serial.print(" R:"); Serial.println(pwm_r_r);
    } 
    else if (cmd == "speedUP") {
      pwm_f_l += value1;
      pwm_f_r -= value1;
    } 
    else if (cmd == "speedUpRun") {
      pwm_f_l += value1;
      pwm_f_r -= value1;
      move_forward_time(3150, pwm_f_l, pwm_f_r);
    } 
    else if (cmd == "speedDown") {
      pwm_f_l -= value1;
      pwm_f_r += value1;
    } 
    else if (cmd == "speedDownRun") {
      pwm_f_l -= value1;
      pwm_f_r += value1;
      move_forward_time(3150, pwm_f_l, pwm_f_r);
    } 
    else if (cmd == "init") {
      pwm_f_l = 80;
      pwm_f_r = 138;
    } 
    else if (cmd == "font") {
      move_forward_time(3150, pwm_f_l, pwm_f_r);
    } else if (cmd == "back") {
      move_backward_time(3150, pwm_b_l, pwm_b_r);
    } else if (cmd == "stop") {
      Tank_Stop();
    } else if (value1 >= 40) {
      Serial.println("L");
      turn_left_time(value1, pwm_l_l, pwm_l_r);
    } else if (value1 <= -40) {
      int m = value1 * -1;
      Serial.println(m);
      turn_right_time(m, pwm_r_l, pwm_r_r);
    } else {
      Serial.println("Unknown command");
    }
  }
}
