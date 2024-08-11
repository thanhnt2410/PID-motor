#define phaA 2
#define phaB 4
#define motorPWM 11 // Chân PWM điều khiển tốc độ động cơ (L298N)
#define INT1 9
#define INT2 10
#define PI 3.14

// Định nghĩa các macro để chọn loại xung
#define SIN_WAVE 1
#define STEP_WAVE 2

// Chọn loại xung cần sử dụng
//#define WAVE_TYPE SIN_WAVE // Thay đổi SIN_WAVE thành STEP_WAVE để chuyển đổi
#define WAVE_TYPE STEP_WAVE
// Định nghĩa các tham số PID theo loại xung
#if WAVE_TYPE == SIN_WAVE
    double kp = 0.6, ki = 0.001, kd = 0; // Các tham số PID cho xung sin
#elif WAVE_TYPE == STEP_WAVE
    double kp = 0.04, ki = 0.0005, kd = 0.0001; // Các tham số PID cho xung step
#endif

volatile int counter = 0; // Biến đếm từ encoder
int lastCounter = 0;
float rpm, setpoint = 2000, output;
double error, lastError;
double integral = 0, derivative;
unsigned long lastTime, currentTime;
unsigned long sampleTime = 100; // 200ms
float N = 1000;           //1000 xung mỗi vòng
double maxIntegral = 1000.0; // Giới hạn cho tích phân
unsigned long batdaulaymau, ketthuclaymau;
int period = 0; // 5 giây cho một chu kỳ

double previous_filtered_output = 0;
double alpha = 0.1; // Hệ số lọc, bạn có thể điều chỉnh giá trị này

void setup()
{
    Serial.begin(9600);
    pinMode(phaA, INPUT_PULLUP); // chân đầu vào pullup bên trong 2
    pinMode(phaB, INPUT_PULLUP); // chân đầu vào pullup bên trong 3
    pinMode(motorPWM, OUTPUT);   // Chân PWM điều khiển động cơ
    pinMode(INT1, OUTPUT);       // Chân INT1 điều khiển hướng động cơ
    pinMode(INT2, OUTPUT);       // Chân INT2 điều khiển hướng động cơ
//Thiết lập ngắt   
// Xung tăng từ bộ mã hóa được kích hoạt ai0(). AttachInterrupt 0 là DigitalPin nr 2 trên hầu hết Arduino.
    attachInterrupt(0, ai0, RISING);
    lastTime = millis();
    batdaulaymau =millis();
    Serial.print("Nhap chu ky T: ");
    while(!Serial.available())
  {
    period = Serial.parseInt();
    //period = Serial.readString().toInt;
    Serial.println("Chu ky: " + String(period));
  }
}
void loop()
{
  currentTime = millis();
    
    // Tính toán setpoint theo thời gian
    
    double timeInCycle = fmod(currentTime, period);
    //------------Xung hinh xin--------------

    #if WAVE_TYPE == SIN_WAVE
    // ------------Xung hình sin--------------
    double angle = (timeInCycle / period) * 2 * PI;
    setpoint = 510 * sin(angle); // Định tỷ lệ giá trị sin từ -500 đến 500

    #elif WAVE_TYPE == STEP_WAVE
    // ------------Xung step-----------------
    if (timeInCycle <= period/2) 
    {
        // Giai đoạn 0 đến 2 giây: Tăng từ 0 lên 500
        setpoint = 0;
    }
    else if (timeInCycle > period/2 && timeInCycle <= period) {
        setpoint = 500;
    }
    else 
    {
        // Giai đoạn 4 đến 6 giây: Không phát xung
        setpoint = 0; // Giữ nguyên giá trị step
    }
    #endif


    // Tính toán các thành phần của PI
    error = setpoint - counter;
    //Serial.println(error);
    integral += error * (currentTime - lastTime) / 1000.0;
    integral = constrain(integral, -maxIntegral, maxIntegral); // Giới hạn tích phân
    derivative = (error - lastError) / ((currentTime - lastTime) / 1000.0);\
    if(kd ==0)
    {
      derivative =0;
    }
    // Tính toán đầu ra PI
    double output = kp * error + ki * integral + kd * derivative;

    // Giới hạn đầu ra từ 0 đến 255 (giá trị PWM)
    if (output > 255) output = 255;
    // Điều khiển hướng động cơ
    if (error > 0) {
      analogWrite(motorPWM, output);
        digitalWrite(INT1, HIGH);
        digitalWrite(INT2, LOW);
    } else {
      output = - output;
      analogWrite(motorPWM, output);
        digitalWrite(INT1, LOW);
        digitalWrite(INT2, HIGH);
        
    }

    // Điều khiển động cơ bằng PWM
    

    // Cập nhật giá trị cho lần sau
    lastError = error;
    lastTime = currentTime;

    //Serial.print("Counter: ");
    //Serial.println(batdaulaymau);
    if(millis() - batdaulaymau>=100)
    {
    Serial.print(counter);
    Serial.print(" ");
    //Serial.print(" Setpoint: ");
    Serial.println(setpoint);
    batdaulaymau = millis();
    }
    
    //Serial.print(" Output: ");
    //Serial.println(output);
}
void ai0()
{
    
// ai0 được kích hoạt nếu DigitalPin nr 2 chuyển từ THẤP lên CAO
    // Kiểm tra chân 3 để xác định hướng
    if (digitalRead(phaB) == LOW)
    {
        counter++;
    }
    else if(digitalRead(phaB)==HIGH)
    {
        counter--;
    }
    else{}
}
