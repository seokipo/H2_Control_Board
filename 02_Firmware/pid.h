/**
 * @file pid.h
 * @brief dsPIC33CK Embedded Closed-Loop PID Controller Module
 * 
 * @note 본 헤더는 수소 제어 보드(H2_Control_Board)의 냉각수 순환/공급 펌프 및
 *       아날로그 액추에이터의 실시간 폐루프 PID 제어 엔진을 정의합니다.
 */

#ifndef PID_H
#define PID_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float kp;               // 비례 게인
    float ki;               // 적분 게인
    float kd;               // 미분 게인
    float integral;         // 누적 적분항 (-0.25 ~ +0.25 Anti-Windup Clamping)
    float last_error;       // 직전 정규화 오차
    float out_min;          // 조작량 하한 (0.0f)
    float out_max;          // 조작량 상한 (1.0f)
    bool is_cooling;        // true: 냉각 계통 (Reverse Action, Error = PV - SP)
                            // false: 공급 계통 (Direct Action, Error = SP - PV)
} PID_Controller_t;

/**
 * @brief PID 제어기 초기화
 */
static inline void PID_Initialize(PID_Controller_t *pid, float kp, float ki, float kd, bool is_cooling) {
    if (!pid) return;
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->out_min = 0.0f;
    pid->out_max = 1.0f;
    pid->is_cooling = is_cooling;
}

/**
 * @brief PID 제어기 적분항 및 오차 리셋 (수동 전환 시 범프리스 전환)
 */
static inline void PID_Reset(PID_Controller_t *pid) {
    if (!pid) return;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
}

/**
 * @brief 실시간 PID 폐루프 연산 및 조작량 산출 (16비트 DAC RAW 반환)
 * @param pid 제어기 인스턴스
 * @param setpoint 목표값 (SP, 예: 30.0f)
 * @param process_variable 계측값 (PV, 예: 28.2f)
 * @param span 센서 유효 스팬 (예: 온도면 100.0f)
 * @param dt 제어 주기 (초 단위, 예: 0.2f)
 * @return uint16_t 16비트 DAC 조작량 RAW (0 ~ 65535)
 */
static inline uint16_t PID_Update(PID_Controller_t *pid, float setpoint, float process_variable, float span, float dt) {
    if (!pid || span <= 0.0f || dt <= 0.0f) return 0;

    // 1. 장치 특성별 제어 방향(Action Direction) 분기
    // - 냉각 계통 (열전대 TC 기반 펌프): PV > SP 시 펌프 증속 냉각 촉진 (Reverse Action)
    // - 공급 계통 (ADC 블로어/MFC): PV < SP 시 출력 증속 공급 증대 (Direct Action)
    float raw_error = pid->is_cooling ? (process_variable - setpoint) : (setpoint - process_variable);
    float norm_error = raw_error / span; // -1.0 ~ +1.0 정규화

    // 2. 비례항(P)
    float P = pid->kp * norm_error;

    // 3. 적분항(I) & 안티 와인드업(Anti-Windup Clamping)
    if (pid->is_cooling && process_variable <= setpoint) {
        // 목표 온도 이하로 냉각된 경우 적분항 즉각 리셋 (오버쿨링/결빙 방지)
        pid->integral = 0.0f;
    } else {
        pid->integral += norm_error * pid->ki * dt;
        // 적분 누적치를 ±0.25 (±25%) 범위로 엄격히 제한
        if (pid->integral > 0.25f) pid->integral = 0.25f;
        else if (pid->integral < -0.25f) pid->integral = -0.25f;
    }
    float I = pid->integral;

    // 4. 미분항(D)
    float d_err = (norm_error - pid->last_error) / dt;
    float D = pid->kd * d_err;
    pid->last_error = norm_error;

    // 5. 조작량(MV) 계산 및 컷오프(Cutoff) 처리
    float mv = 0.0f;
    if (pid->is_cooling) {
        if (process_variable <= setpoint) {
            // 냉각 컷오프: 현재 온도가 목표치 이하이면 펌프 완전 정지 (0V / 0%)
            mv = 0.0f;
        } else {
            // 현재 온도가 목표치보다 높을 때 편차에 비례하여 증속 가동
            mv = P + I + D;
            if (mv < 0.05f) mv = 0.05f; // 최소 기동 토크 확보
            if (mv > pid->out_max) mv = pid->out_max;
        }
    } else {
        // 공급 장치
        float base_mv = (setpoint / span);
        mv = base_mv + P + I + D;
        if (mv < pid->out_min) mv = pid->out_min;
        if (mv > pid->out_max) mv = pid->out_max;
    }

    // 6. 16비트 DAC RAW 값 변환 (0 ~ 65535)
    float raw_val = mv * 65535.0f;
    if (raw_val < 0.0f) raw_val = 0.0f;
    if (raw_val > 65535.0f) raw_val = 65535.0f;

    return (uint16_t)(raw_val + 0.5f);
}

#endif /* PID_H */
