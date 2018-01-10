// This header file contains constants needed to use the USB, serial, and I2C
// protocols for the Jrk G2 controllers from Pololu.
//
// Note: Many of these definitions are not considered to be part of the public
// API of libpololu-jrk2, so they could change or be removed in future versions
// of the library.  The only definitions that are part of the public API are the
// ones referred to in the comments in jrk.h.

#ifndef _JRK_PROTOCOL_H
#define _JRK_PROTOCOL_H

#define JRK_VENDOR_ID 0x1FFB
// TODO: use a more real product name here when we decide it
#define JRK_PRODUCT_ID_2017 0x00B7

#define JRK_CMD_REINITIALIZE 0x10
#define JRK_CMD_SET_SETTING 0x13
#define JRK_CMD_GET_DEBUG_DATA 0x20
#define JRK_CMD_GET_VARIABLE_SERIAL 0x80
#define JRK_CMD_GET_ERROR_FLAGS_HALTING_SERIAL 0xB3
#define JRK_CMD_GET_ERROR_FLAGS_OCCURRED_SERIAL 0xB5
#define JRK_CMD_SET_TARGET_USB 0x84
#define JRK_CMD_MOTOR_OFF_USB 0x87
#define JRK_CMD_SET_TARGET_SERIAL 0xC0
#define JRK_CMD_SET_TARGET_LOW_RES_REV 0xE0
#define JRK_CMD_SET_TARGET_LOW_RES_FWD 0xE1
#define JRK_CMD_GET_SETTINGS 0xE3
#define JRK_CMD_GET_VARIABLES 0xE5
#define JRK_CMD_SET_OVERRIDABLE_SETTINGS 0xE6
#define JRK_CMD_OVERRIDE_DUTY_CYCLE 0xE9
#define JRK_CMD_GET_OVERRIDABLE_SETTINGS 0xEA
#define JRK_CMD_GET_CURRENT_CHOPPING_OCCURRENCE_COUNT 0xEC
#define JRK_CMD_MOTOR_OFF_SERIAL 0xFF
#define JRK_CMD_START_BOOTLOADER 0xFF

#define JRK_CMD_GET_VARIABLE_SERIAL_MASK 0xC0
#define JRK_CMD_SET_TARGET_SERIAL_MASK 0xE0

#define JRK_GET_VARIABLES_FLAG_CLEAR_ERROR_FLAGS_HALTING 0
#define JRK_GET_VARIABLES_FLAG_CLEAR_ERROR_FLAGS_OCCURRED 1
#define JRK_GET_VARIABLES_FLAG_CLEAR_CURRENT_CHOPPING_OCCURRENCE_COUNT 2

#define JRK_VAR_INPUT 0x00
#define JRK_VAR_TARGET 0x02
#define JRK_VAR_FEEDBACK 0x04
#define JRK_VAR_SCALED_FEEDBACK 0x06
#define JRK_VAR_ERROR_SUM 0x08
#define JRK_VAR_DUTY_CYCLE_TARGET 0x0A
#define JRK_VAR_DUTY_CYCLE 0x0C
#define JRK_VAR_CURRENT 0x0E
#define JRK_VAR_PID_PERIOD_EXCEEDED 0x0F
#define JRK_VAR_PID_PERIOD_COUNT 0x10
#define JRK_VAR_ERROR_FLAGS_HALTING 0x12
#define JRK_VAR_ERROR_FLAGS_OCCURRED 0x14
#define JRK_VAR_VIN_VOLTAGE 0x16
#define JRK_VAR_DEVICE_RESET 0x1F
#define JRK_VAR_UP_TIME 0x20
#define JRK_VAR_RC_PULSE_WIDTH 0x24
#define JRK_VAR_TACHOMETER_READING 0x26
#define JRK_VAR_ANALOG_READING_SCL 0x28
#define JRK_VAR_ANALOG_READING_SDA 0x2A
#define JRK_VAR_ANALOG_READING_TX 0x2C
#define JRK_VAR_ANALOG_READING_RX 0x2E
#define JRK_VAR_ANALOG_READING_AUX 0x30
#define JRK_VAR_ANALOG_READING_FBA 0x32
#define JRK_VAR_DIGITAL_READINGS 0x34
#define JRK_VAR_PIN_STATES 0x35
#define JRK_VAR_CURRENT_HIGH_RES 0x37
#define JRK_VAR_MAX_CURRENT 0x39
#define JRK_VAR_LAST_DUTY_CYCLE 0x3B
#define JRK_VAR_CURRENT_CHOPPING_CONSECUTIVE_COUNT 0x3D
#define JRK_VAR_CURRENT_CHOPPING_OCCURRENCE_COUNT 0x3E
#define JRK_VARIABLES_SIZE 0x3F

#define JRK_SETTING_NOT_INITIALIZED 0x00
#define JRK_SETTING_OPTIONS_BYTE1 0x01
#define JRK_SETTING_OPTIONS_BYTE2 0x02
#define JRK_SETTING_INPUT_MODE 0x03
#define JRK_SETTING_INPUT_ABSOLUTE_MINIMUM 0x04
#define JRK_SETTING_INPUT_ABSOLUTE_MAXIMUM 0x06
#define JRK_SETTING_INPUT_MINIMUM 0x08
#define JRK_SETTING_INPUT_MAXIMUM 0x0A
#define JRK_SETTING_INPUT_NEUTRAL_MAXIMUM 0x0C
#define JRK_SETTING_INPUT_NEUTRAL_MINIMUM 0x0E
#define JRK_SETTING_OUTPUT_MINIMUM 0x10
#define JRK_SETTING_OUTPUT_NEUTRAL 0x12
#define JRK_SETTING_OUTPUT_MAXIMUM 0x14
#define JRK_SETTING_INPUT_SCALING_DEGREE 0x16
#define JRK_SETTING_INPUT_ANALOG_SAMPLES_EXPONENT 0x17
#define JRK_SETTING_FEEDBACK_MODE 0x18
#define JRK_SETTING_FEEDBACK_ABSOLUTE_MINIMUM 0x19
#define JRK_SETTING_FEEDBACK_ABSOLUTE_MAXIMUM 0x1B
#define JRK_SETTING_FEEDBACK_MINIMUM 0x1D
#define JRK_SETTING_FEEDBACK_MAXIMUM 0x1F
#define JRK_SETTING_FEEDBACK_DEAD_ZONE 0x21
#define JRK_SETTING_FEEDBACK_ANALOG_SAMPLES_EXPONENT 0x22
#define JRK_SETTING_SERIAL_MODE 0x23
#define JRK_SETTING_SERIAL_BAUD_RATE_GENERATOR 0x24
#define JRK_SETTING_SERIAL_TIMEOUT 0x26
#define JRK_SETTING_SERIAL_DEVICE_NUMBER 0x28
#define JRK_SETTING_ERROR_ENABLE 0x2A
#define JRK_SETTING_ERROR_LATCH 0x2C
#define JRK_SETTING_VIN_CALIBRATION 0x2E
#define JRK_SETTING_PIN_CONFIG_SCL 0x30
#define JRK_SETTING_PIN_CONFIG_SDA 0x31
#define JRK_SETTING_PIN_CONFIG_TX 0x32
#define JRK_SETTING_PIN_CONFIG_RX 0x33
#define JRK_SETTING_PIN_CONFIG_RC 0x34
#define JRK_SETTING_PIN_CONFIG_AUX 0x35
#define JRK_SETTING_PIN_CONFIG_FBA 0x36
#define JRK_SETTING_PIN_CONFIG_FBT 0x37
#define JRK_SETTING_MOTOR_PWM_FREQUENCY 0x38
#define JRK_SETTING_MOTOR_CURRENT_CALIBRATION_FORWARD 0x39
#define JRK_SETTING_MOTOR_CURRENT_CALIBRATION_REVERSE 0x3A

#define JRK_SETTING_OPTIONS_BYTE3 0x50
#define JRK_SETTING_PROPORTIONAL_MULTIPLIER 0x51
#define JRK_SETTING_PROPORTIONAL_EXPONENT 0x53
#define JRK_SETTING_INTEGRAL_MULTIPLIER 0x54
#define JRK_SETTING_INTEGRAL_EXPONENT 0x56
#define JRK_SETTING_DERIVATIVE_MULTIPLIER 0x57
#define JRK_SETTING_DERIVATIVE_EXPONENT 0x59
#define JRK_SETTING_PID_PERIOD 0x5A
#define JRK_SETTING_PID_INTEGRAL_LIMIT 0x5C
#define JRK_SETTING_MOTOR_MAX_DUTY_CYCLE_WHILE_FEEDBACK_OUT_OF_RANGE 0x5E
#define JRK_SETTING_MOTOR_MAX_ACCELERATION_FORWARD 0x60
#define JRK_SETTING_MOTOR_MAX_ACCELERATION_REVERSE 0x62
#define JRK_SETTING_MOTOR_MAX_DECELERATION_FORWARD 0x64
#define JRK_SETTING_MOTOR_MAX_DECELERATION_REVERSE 0x66
#define JRK_SETTING_MOTOR_MAX_DUTY_CYCLE_FORWARD 0x68
#define JRK_SETTING_MOTOR_MAX_DUTY_CYCLE_REVERSE 0x6A
#define JRK_SETTING_MOTOR_MAX_CURRENT_FORWARD 0x6C
#define JRK_SETTING_MOTOR_MAX_CURRENT_REVERSE 0x6E
#define JRK_SETTING_MOTOR_BRAKE_DURATION_FORWARD 0x70
#define JRK_SETTING_MOTOR_BRAKE_DURATION_REVERSE 0x71

#define JRK_OVERRIDABLE_SETTINGS_START 0x50
#define JRK_SETTINGS_SIZE 0x72

#define JRK_OPTIONS_BYTE1_NEVER_SLEEP 0
#define JRK_OPTIONS_BYTE1_SERIAL_ENABLE_CRC 1
#define JRK_OPTIONS_BYTE1_SERIAL_ENABLE_14BIT_DEVICE_NUMBER 2
#define JRK_OPTIONS_BYTE1_SERIAL_DISABLE_COMPACT_PROTOCOL 3

#define JRK_OPTIONS_BYTE2_INPUT_INVERT 0
#define JRK_OPTIONS_BYTE2_INPUT_DETECT_DISCONNECT 1
#define JRK_OPTIONS_BYTE2_FEEDBACK_INVERT 2
#define JRK_OPTIONS_BYTE2_FEEDBACK_DETECT_DISCONNECT 3
#define JRK_OPTIONS_BYTE2_FEEDBACK_WRAPAROUND 4
#define JRK_OPTIONS_BYTE2_MOTOR_INVERT 5

#define JRK_OPTIONS_BYTE3_PID_RESET_INTEGRAL 0
#define JRK_OPTIONS_BYTE3_MOTOR_COAST_WHEN_OFF 1

#define JRK_FIRMWARE_MODIFICATION_STRING_INDEX 6

#define JRK_MAX_USB_RESPONSE_SIZE 128

#define JRK_MAX_ALLOWED_DUTY_CYCLE 600

#define JRK_INPUT_MODE_SERIAL 0
#define JRK_INPUT_MODE_ANALOG 1
#define JRK_INPUT_MODE_PULSE_WIDTH 2

#define JRK_FEEDBACK_MODE_NONE 0
#define JRK_FEEDBACK_MODE_ANALOG 1
#define JRK_FEEDBACK_MODE_FREQUENCY 2

#define JRK_SERIAL_MODE_USB_DUAL_PORT 0
#define JRK_SERIAL_MODE_USB_CHAINED 1
#define JRK_SERIAL_MODE_UART 2

#define JRK_MOTOR_PWM_FREQUENCY_20 0
#define JRK_MOTOR_PWM_FREQUENCY_5 1

#define JRK_SCALING_DEGREE_LINEAR 0
#define JRK_SCALING_DEGREE_QUADRATIC 1
#define JRK_SCALING_DEGREE_CUBIC 2
#define JRK_SCALING_DEGREE_QUARTIC 3
#define JRK_SCALING_DEGREE_QUINTIC 4

#define JRK_BRAKE_DURATION_UNITS 5
#define JRK_MAX_ALLOWED_BRAKE_DURATION 1275

#define JRK_SERIAL_TIMEOUT_UNITS 10
#define JRK_MAX_ALLOWED_SERIAL_TIMEOUT 655350

#define JRK_BAUD_RATE_GENERATOR_FACTOR 12000000

#define JRK_MIN_ALLOWED_BAUD_RATE 184
#define JRK_MAX_ALLOWED_BAUD_RATE 115385

#define JRK_PIN_PULLUP 7
#define JRK_PIN_ANALOG 6
#define JRK_PIN_FUNC_POSN 0
#define JRK_PIN_FUNC_MASK 0x0F
#define JRK_PIN_FUNC_DEFAULT 0
#define JRK_PIN_FUNC_USER_IO 1
#define JRK_PIN_FUNC_USER_INPUT 2
#define JRK_PIN_FUNC_POT_POWER 3
#define JRK_PIN_FUNC_SERIAL 4
#define JRK_PIN_FUNC_RC 5
#define JRK_PIN_FUNC_TACHOMETER 6

#define JRK_PIN_NUM_SCL 0
#define JRK_PIN_NUM_SDA 1
#define JRK_PIN_NUM_TX 2
#define JRK_PIN_NUM_RX 3
#define JRK_PIN_NUM_RC 4
#define JRK_PIN_NUM_AUX 5
#define JRK_PIN_NUM_FBA 6
#define JRK_PIN_NUM_FBT 7
#define JRK_CONTROL_PIN_COUNT 8

#define JRK_RESET_POWER_UP 0
#define JRK_RESET_BROWNOUT 1
#define JRK_RESET_RESET_LINE 2
#define JRK_RESET_WATCHDOG 4
#define JRK_RESET_SOFTWARE 8
#define JRK_RESET_STACK_OVERFLOW 16
#define JRK_RESET_STACK_UNDERFLOW 32

#define JRK_PIN_STATE_HIGH_IMPEDANCE 0
#define JRK_PIN_STATE_PULLED_UP 1
#define JRK_PIN_STATE_OUTPUT_LOW 2
#define JRK_PIN_STATE_OUTPUT_HIGH 3

#define JRK_ERROR_AWAITING_COMMAND 0
#define JRK_ERROR_NO_POWER 1
#define JRK_ERROR_MOTOR_DRIVER 2
#define JRK_ERROR_INPUT_INVALID 3
#define JRK_ERROR_INPUT_DISCONNECT 4
#define JRK_ERROR_FEEDBACK_DISCONNECT 5
#define JRK_ERROR_MAXIMUM_CURRENT_EXCEEDED 6
#define JRK_ERROR_SERIAL_SIGNAL 7
#define JRK_ERROR_SERIAL_OVERRUN 8
#define JRK_ERROR_SERIAL_BUFFER_FULL 9
#define JRK_ERROR_SERIAL_CRC 10
#define JRK_ERROR_SERIAL_PROTOCOL 11
#define JRK_ERROR_SERIAL_TIMEOUT 12

#endif
