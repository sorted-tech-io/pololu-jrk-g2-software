Settings = [
  {
    name: 'input_mode',
    type: :enum,
    max: 'JRK_INPUT_MODE_PULSE_WIDTH',
    default: 'JRK_INPUT_MODE_SERIAL',
    default_is_zero: true,
    english_default: 'serial',
    comment: <<EOF
The input mode setting specifies how you want to control the jrk.  It
determines the definition of the input and target variables.  The input
variable is raw measurement of the jrk's input.  The target variable is the
desired state of the system's output, and feeds into the PID feedback
algorithm.

- If the input mode is JRK_INPUT_MODE_SERIAL, the jrk gets it input and
  target settings over its USB, serial, or I2C interfaces.  You would send
  Set Target commands to the jrk to set both the input and target variables.

- If the input mode is JRK_INPUT_MODE_ANALOG, the jrk gets it input variable
  by reading the voltage on its SDA/AN pin.  A signal level of 0 V
  corresponds to an input value of 0, and a signal elvel of 5 V corresponds
  to an input value of 4092.  The jrk uses its input scaling feature to set
  the target variable.

- If the input mode is JRK_INPUT_MODE_PULSE_WIDTH, the jrk gets it input
  variable by reading RC pulses on its RC pin.  The input value is the width
  of the most recent pulse, in units of 2/3 microseconds.  The jrk uses its
  input scaling feature to set the target variable.
EOF
  },
  {
    name: 'input_disconnect_minimum',
    type: :uint16_t,
    range: 0..4095,
  },
  {
    name: 'input_disconnect_maximum',
    type: :uint16_t,
    range: 0..4095,
    default: 4095,
  },
  {
    name: 'input_minimum',
    type: :uint16_t,
    range: 0..4095,
    comment: <<EOF
This is one of the parameters of the input scaling feature, which is how the
jrk calculates its target value from its raw input.

By default, the input scaling:

1. Maps all values less than the input_minimum to the output_minimum.
2. Maps all values greater than the input_maximum to the output_maximum.
3. Maps all values between the input_neutral_min and input_neutral_max to
   the output the output_neutral.
4. Behaves linearly between those regions.

The input_invert parameter can flip that correspondence, and the
input_scaling_degree parameter can change item 4 to use higher-order curves
that give you finer control of the output near the neutral region.
EOF
  },
  {
    name: 'input_maximum',
    type: :uint16_t,
    range: 0..4095,
    default: 4095,
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'input_neutral_minimum',
    type: :uint16_t,
    range: 0..4095,
    default: 2048,
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'input_neutral_maximum',
    type: :uint16_t,
    range: 0..4095,
    default: 2048,
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'output_minimum',
    type: :uint16_t,
    range: 0..4095,
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'output_neutral',
    type: :uint16_t,
    range: 0..4095,
    default: 2048,
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'output_maximum',
    type: :uint16_t,
    range: 0..4095,
    default: 4095,
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'input_invert',
    type: :bool,
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'input_scaling_degree',
    type: :enum,
    default: 'JRK_SCALING_DEGREE_LINEAR',
    default_is_zero: true,
    english_default: 'linear',
    max: 'JRK_SCALING_DEGREE_QUINTIC',
    comment:
      "This is one of the parameters of the input scaling, as described in the\n" \
      "input_minimum documentation."
  },
  {
    name: 'input_power_with_aux',
    type: :bool,
  },
  {
    name: 'input_analog_samples_exponent',
    type: :uint8_t,
    range: 0..8,
    comment:
      "This setting specifies how many analog samples to take if the input mode\n" \
      "is analog.  The number of samples will be 4*(2^x), where x is this setting.",
    default: 5,
  },
  {
    name: 'feedback_mode',
    type: :enum,
    default: 'JRK_FEEDBACK_MODE_ANALOG',
    english_default: 'analog',
    max: 'JRK_FEEDBACK_MODE_FREQUENCY',
    # TODO: are FB_A and FB_T the real pin names on the silkscreen?
    comment: <<EOF
The feedback mode setting specifies whether the jrk is using feedback from
the output of the system, and if so defines what interface is used to
measure that feedback.

- If the feedback mode is JRK_FEEDBACK_MODE_NONE, feedback and PID
  calculations are disabled.  The duty cycle target variable is always equal
  to the target variable minus 2048, instead of being the result of a PID
  calculation.  This means that a target of 2648 corresponds to driving the
  motor full speed forward, 2048 is brake, and 1448 is full-speed reverse.

- If the feedback mode is JRK_FEEDBACK_MODE_ANALOG, the jrk gets its
  feedback by measuring the voltage on the FB_A pin.  A level of 0 V
  corresponds to a feedback value of 0, and a level of 5 V corresponds to a
  feedback value of 4092.  The feedback scaling algorithm computes the
  scaled feedback variable, and the PID algorithm uses the scaled feedback
  and the target to compute the duty cycle target.

- If the feedback mode is JRK_FEEDBACK_MODE_FREQUENCY, the jrk gets it
  feedback by counting rising edges on its FB_T pin.  When the target is
  greater than 2048, the feedback value is 2048 plus the number of rising
  edges detected during the PID period.  Otherwise, the the feedback is
  2048 minus the the number of rising edges detected during the PID period.
EOF
  },
  {
    name: 'feedback_disconnect_minimum',
    type: :uint16_t,
    range: 0..4095,
  },
  {
    name: 'feedback_disconnect_maximum',
    type: :uint16_t,
    range: 0..4095,
  },
  {
    name: 'feedback_minimum',
    type: :uint16_t,
    range: 0..4095,
    comment:
      "This is one of the parameters of the feedback scaling feature.\n\n" \
      "By default, the feedback scaling:\n\n" \
      "  1. Maps values less than or equal to feedback_minimum to 0.\n" \
      "  2. Maps values less than or equal to feedback_maximum to 4095.\n" \
      "  3. Behaves linearly between those two regions.\n\n" \
      "The feedback_invert parameter causes the mapping to be flipped.\n"
  },
  {
    name: 'feedback_maximum',
    type: :uint16_t,
    range: 0..4095,
    comment:
      "This is one of the parameters of the feedback scaling described in\n" \
      "the feedback_minimum documentation."
  },
  {
    name: 'feedback_invert',
    type: :bool,
    comment:
      "This is one of the parameters of the feedback scaling described in\n" \
      "the feedback_minimum documentation."
  },
  {
    name: 'feedback_power_with_aux',
    type: :bool,
  },
  {
    name: 'feedback_dead_zone',
    type: :uint8_t,
  },
  {
    name: 'feedback_analog_samples_exponent',
    type: :uint8_t,
    range: 0..8,
    comment:
      "This setting specifies how many analog samples to take if the feedback mode\n" \
      "is analog.  The number of samples will be 4*(2^x), where x is this setting.",
    default: 5,
  },
  {
    name: 'feedback_wraparound',
    type: :bool,
    comment:
      "Normally, the error variable used by the PID algorithm is simply the scaled\n" \
      "feedback minus the target.  With this setting enabled, the PID algorithm\n" \
      "will add or subtract 4096 from that error value to get it into the -2048 to\n" \
      "2048 range.  This is useful for systems where the output of the system wraps\n" \
      "around, so that 0 is next to 4095.  The jrk will know how to take the\n" \
      "shortest path from one point to another even if it involves wrapping around\n" \
      "from 0 to 4095 or vice versa.",
  },
  {
    name: 'serial_mode',
    type: :enum,
    default: 'JRK_SERIAL_MODE_USB_DUAL_PORT',
    default_is_zero: true,
    english_default: 'USB dual port',
    max: 'JRK_SERIAL_MODE_UART',
  },
  {
    name: 'serial_baud_rate',
    type: :uint32_t,
    custom_fix: true,
    custom_eeprom: true,
    comment:
      "This setting specifies the baud rate to use on the RX and TX pins\n" \
      "when the serial mode is UART.  It should be between\n" \
      "JRK_MIN_ALLOWED_BAUD_RATE and JRK_MAX_ALLOWED_BAUD_RATE.",
  },
  {
    name: 'serial_timeout',
    type: :uint16_t,
    comment:
      "This is the time in units of 10 milliseconds before the device considers\n" \
      "it to be an error if it has not received certain commands.  A value of 0\n" \
      "disables the command timeout feature.",
  },
  {
    name: 'serial_device_number',
    type: :uint16_t,
    max: 16383,
    default: 11,
  },
  {
    name: 'never_sleep',
    type: :bool,
    address: 'JRK_SETTING_OPTIONS_BYTE1',
    bit_address: 'JRK_OPTIONS_BYTE1_NEVER_SLEEP',
  },
  {
    name: 'serial_enable_crc',
    type: :bool,
    address: 'JRK_SETTING_OPTIONS_BYTE1',
    bit_address: 'JRK_OPTIONS_BYTE1_SERIAL_ENABLE_CRC',
  },
  {
    name: 'serial_enable_14bit_device_number',
    type: :bool,
    address: 'JRK_SETTING_OPTIONS_BYTE1',
    bit_address: 'JRK_OPTIONS_BYTE1_SERIAL_ENABLE_14BIT_DEVICE_NUMBER',
  },
  {
    name: 'serial_disable_compact_protocol',
    type: :bool,
    address: 'JRK_SETTING_OPTIONS_BYTE1',
    bit_address: 'JRK_OPTIONS_BYTE1_SERIAL_DISABLE_COMPACT_PROTOCOL',
  },
  {
    name: 'proportional_multiplier',
    type: :uint16_t,
    max: 1023,
    comment:
      "The allowed range of this setting is 0 to 1023.\n\n" \
      "In the PID algorithm, the error (the difference between scaled feedback\n" \
      "and target) is multiplied by a number called the proportional coefficient to\n" \
      "determine its effect on the motor duty cycle.\n\n" \
      "The proportional coefficient is defined by this mathematical expression:\n" \
      "  proportional_multiplier / 2^(proportional_exponent)\n"
  },
  {
    name: 'proportional_exponent',
    type: :uint8_t,
    max: 18,
    comment:
      "The allowed range of this setting is 0 to 18.\n" \
      "For more information, see the proportional_multiplier documentation."
  },
  {
    name: 'integral_multiplier',
    type: :uint16_t,
    max: 1023,
    comment:
      "The allowed range of this setting is 0 to 1023.\n\n" \
      "In the PID algorithm, the accumulated error (known as error sum)\n" \
      "is multiplied by a number called the integral coefficient to\n" \
      "determine its effect on the motor duty cycle.\n\n" \
      "The integral coefficient is defined by this mathematical expression:\n" \
      "  integral_multiplier / 2^(integral_exponent)\n\n" \
      "Note: On the original jrks (jrk 12v12 and jrk 21v3), the formula was\n" \
      "different.  Those jrks added 3 to the integral_exponent before using\n" \
      "it as a power of 2."
  },
  {
    name: 'integral_exponent',
    type: :uint8_t,
    max: 18,
    comment:
      "The allowed range of this setting is 0 to 18.\n" \
      "For more information, see the integral_multiplier documentation."
  },
  {
    name: 'derivative_multiplier',
    type: :uint16_t,
    max: 1023,
    comment:
      "The allowed range of this setting is 0 to 1023.\n\n" \
      "In the PID algorithm, the change in the error since the last PID period\n" \
      "is multiplied by a number called the derivative coefficient to\n" \
      "determine its effect on the motor duty cycle.\n\n" \
      "The derivative coefficient is defined by this mathematical expression:\n" \
      "  derivative_multiplier / 2^(derivative_exponent)\n"
  },
  {
    name: 'derivative_exponent',
    type: :uint8_t,
    max: 18,
    comment:
      "The allowed range of this setting is 0 to 18.\n" \
      "For more information, see the derivative_multiplier documentation.",
  },
  {
    name: 'pid_period',
    type: :uint16_t,
    range: 1..8191,
    default: 10,
    comment:
      "The PID period specifies how often the jrk should run its PID calculation\n" \
      "and update the motor speed, in units of milliseconds."
  },
  {
    name: 'pid_integral_limit',
    type: :uint16_t,
    default: 1000,
    max: 0x7FFF,
    comment:
      "The PID algorithm prevents the absolute value of the accumulated error\n" \
      "(known as error sum) from exceeding pid_integral_limit.",
  },
  {
    name: 'pid_reset_integral',
    type: :bool,
    comment:
      "If this setting is set to true, the PID algorithm will reset the accumulated\n" \
      "error (also known as error sum) whenever the absolute value of the\n" \
      "proportional term (see proportional_multiplier) exceeds 600."
  },
  {
    name: 'motor_pwm_frequency',
    type: :enum,
    default: 'JRK_MOTOR_PWM_FREQUENCY_20',
    default_is_zero: true,
    english_default: '20 kHz',
    max: 'JRK_MOTOR_PWM_FREQUENCY_5',
    comment:
      "This setting specifies whether to use 20 kHz (the default) or 5 kHz for the\n" \
      "motor PWM signal.  This setting should be either\n" \
      "JRK_MOTOR_PWM_FREQUENCY_20 or JRK_MOTOR_PWM_FREQUENCY_5.\n"
  },
  {
    name: 'motor_invert',
    type: :bool,
    comment:
      "By default, a positive duty cycle (which we call \"forward\") corresponds\n" \
      "to current flowing from output A to output B.  If enabled, this setting flips\n" \
      "the correspondence, so a positive duty cycle corresponds to current flowing\n" \
      "from B to A."
  },
  {
    name: 'motor_max_duty_cycle_while_feedback_out_of_range',
    type: :uint16_t,
    range: 1..600,
    default: 600,
  },
  {
    name: 'motor_max_acceleration_forward',
    type: :uint16_t,
    range: 1..600,
    default: 600,
  },
  {
    name: 'motor_max_acceleration_reverse',
    type: :uint16_t,
    range: 1..600,
    default: 600,
  },
  {
    name: 'motor_max_deceleration_forward',
    type: :uint16_t,
    range: 1..600,
    default: 600,
  },
  {
    name: 'motor_max_deceleration_reverse',
    type: :uint16_t,
    range: 1..600,
    default: 600,
  },
  {
    name: 'motor_max_duty_cycle_forward',
    type: :uint16_t,
    max: 600,
    default: 600,
  },
  {
    name: 'motor_max_duty_cycle_reverse',
    type: :uint16_t,
    max: 600,
    default: 600,
  },
  {
    name: 'motor_max_current_forward',
    type: :uint8_t,
    default: 81, # TODO: rethink this default
  },
  {
    name: 'motor_max_current_reverse',
    type: :uint8_t,
    default: 81, # TODO: rethink this default
  },
  {
    name: 'motor_current_calibration_forward',
    type: :int8_t,
  },
  {
    name: 'motor_current_calibration_reverse',
    type: :int8_t,
  },
  {
    name: 'motor_brake_duration_forward',
    type: :uint8_t,
    comment: 'Units of 5 ms.',
  },
  {
    name: 'motor_brake_duration_reverse',
    type: :uint8_t,
    comment: 'Units of 5 ms.',
  },
  {
    name: 'motor_coast_when_off',
    type: :bool,
    comment:
      "By default, the jrk drives both motor outputs low when the motor is\n" \
      "stopped (duty cycle is zero or there is an error), causing it to brake.\n" \
      "If enabled, this setting causes it to instead tri-state both inputs, making\n" \
      "the motor coast."
  },
  {
    name: 'error_enable',
    type: :uint16_t,
    comment:
      "This setting is a bitmap specifying which errors are enabled.\n" \
      "The JRK_ERROR_* specifies the bits in the bitmap.  Certain errors are\n" \
      "always enabled, so the jrk ignores the bits for those errors."
  },
  {
    name: 'error_latch',
    type: :uint16_t,
    comment:
      "This setting is a bitmap specifying which errors are enabled and latched.\n" \
      "The JRK_ERROR_* specifies the bits in the bitmap.  Certain errors are\n" \
      "always latched if they are enabled, so the jrk ignores the bits for those\n" \
      "errors."
  },
  {
    name: 'vin_calibration',
    english_name: 'VIN calibration',
    type: :int16_t,
    range: -500..500,
    comment:
      "A higher number gives you higher VIN readings, while a lower number gives\n" \
      "you lower VIN readings.",
  },
]
# TODO: comments for all these settings (for jrk.h)