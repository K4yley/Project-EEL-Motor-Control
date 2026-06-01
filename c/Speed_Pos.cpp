#include <stdint.h>
#include <math.h>
#include <stdio.h>

/* ---------------- USER SETTINGS ---------------- */
#define TICKS_PER_REV   1024
#define WHEEL_CIRC_M    0.314

#define MAX_PWM         255
#define DT              0.01f   // 10 ms loop

#define MAX_SPEED       1.0f    // m/s
#define ACCEL           2.0f    // m/s^2
#define DECEL           2.5f    // m/s^2

/* ---------------- ENCODER ---------------- */
volatile int32_t encoderTicks = 0;

void encoderISR(int dir)
{
    encoderTicks += dir;
}

/* ---------------- MOTOR OUTPUT ---------------- */
void setPWM(int pwm)
{
    if (pwm > 255) pwm = 255;
    if (pwm < 0) pwm = 0;
    printf("PWM: %d\n", pwm);
}

/* ---------------- POSITION ---------------- */
float getPosition(void)
{
    return ((float)encoderTicks / TICKS_PER_REV) * WHEEL_CIRC_M;
}

/* ---------------- SPEED ESTIMATION ---------------- */
float lastPos = 0;
float getSpeed(float pos)
{
    float speed = (pos - lastPos) / DT;
    lastPos = pos;
    return speed;
}

/* ---------------- PID CONTROLLER ---------------- */
typedef struct {
    float kp, ki, kd;
    float integral;
    float prevError;
} PID;

float pidUpdate(PID *pid, float target, float measured)
{
    float error = target - measured;

    pid->integral += error * DT;
    float derivative = (error - pid->prevError) / DT;

    pid->prevError = error;

    return (pid->kp * error) +
           (pid->ki * pid->integral) +
           (pid->kd * derivative);
}

/* ---------------- MOTION PROFILE ---------------- */
float computeTargetSpeed(float targetPos, float pos)
{
    float error = targetPos - pos;
    float dist = fabsf(error);

    float speed;

    /* accelerate / cruise / decelerate logic */
    float brakeDist = (MAX_SPEED * MAX_SPEED) / (2.0f * DECEL);

    if (dist > brakeDist)
    {
        // accelerate to max
        speed = MAX_SPEED;
    }
    else
    {
        // decelerate smoothly
        speed = sqrtf(2.0f * DECEL * dist);
        if (speed > MAX_SPEED)
            speed = MAX_SPEED;
    }

    if (error < 0)
        speed = -speed;

    return speed;
}

/* ---------------- MAIN ---------------- */
int main(void)
{
    float targetPos = 5.0f;

    PID pid = {
        .kp = 120.0f,
        .ki = 10.0f,
        .kd = 2.0f,
        .integral = 0,
        .prevError = 0
    };

    while (1)
    {
        /* --- SENSOR --- */
        float pos = getPosition();
        float speed = getSpeed(pos);

        /* --- OUTER LOOP: motion profile --- */
        float targetSpeed = computeTargetSpeed(targetPos, pos);

        /* --- INNER LOOP: PID speed control --- */
        float control = pidUpdate(&pid, targetSpeed, speed);

        /* convert control → PWM */
        int pwm = (int)control;

        if (pwm > MAX_PWM) pwm = MAX_PWM;
        if (pwm < -MAX_PWM) pwm = -MAX_PWM;

        setPWM(pwm);

        /* stop condition */
        if (fabsf(targetPos - pos) < 0.01f &&
            fabsf(speed) < 0.01f)
        {
            setPWM(0);
            printf("Reached target\n");
            break;
        }

        /* simulate loop delay */
    }

    return 0;
}