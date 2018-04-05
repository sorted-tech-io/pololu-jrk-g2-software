#include "main_controller.h"
#include "main_window.h"
#include <file_util.h>
#include <to_string.h>

#include <cassert>
#include <cmath>
#include <sstream>
#include <iostream>  // tmphax

// This is how often we fetch the variables from the device.
static const uint32_t UPDATE_INTERVAL_MS = 50;

// Only update the device list once per second to save CPU time.
static const uint32_t UPDATE_DEVICE_LIST_DIVIDER = 20;

void main_controller::set_window(main_window * window)
{
  this->window = window;
}

void main_controller::start()
{
  assert(!connected());

  // Start the update timer so that update() will be called regularly.
  window->set_update_timer_interval(UPDATE_INTERVAL_MS);
  window->start_update_timer();

  handle_model_changed();
}

// Returns the device that matches the specified OS ID from the list, or a null
// device if none match.
static jrk::device device_with_os_id(
  std::vector<jrk::device> const & device_list,
  std::string const & id)
{
  for (jrk::device const & candidate : device_list)
  {
    if (candidate.get_os_id() == id)
    {
      return candidate;
    }
  }
  return jrk::device(); // null device
}

void main_controller::connect_device_with_os_id(const std::string & id)
{
  if (disconnect_device())
  {
    if (id.size() != 0)
    {
      connect_device(device_with_os_id(device_list, id));
    }
  }
  else
  {
    // User canceled disconnect when prompted about settings that have not been
    // applied. Reset the 'Connected to' dropdown.
    handle_model_changed();
  }
}

bool main_controller::disconnect_device()
{
  if (!connected()) { return true; }

  if (settings_modified)
  {
    std::string question =
      "The settings you changed have not been applied to the device.  "
      "If you disconnect from the device now, those changes will be lost.  "
      "Are you sure you want to disconnect?";
    if (!window->confirm(question))
    {
      return false;
    }
  }

  really_disconnect();
  disconnected_by_user = true;
  connection_error = false;
  handle_model_changed();
  return true;
}

void main_controller::connect_device(jrk::device const & device)
{
  assert(device.is_present());

  try
  {
    // Close the old handle in case one is already open.
    device_handle.close();

    connection_error = false;
    disconnected_by_user = false;

    // Open a handle to the specified device.
    device_handle = jrk::handle(device);
  }
  catch (const std::exception & e)
  {
    set_connection_error("Failed to connect to device.");
    show_exception(e, "There was an error connecting to the device.");
    handle_model_changed();
    return;
  }

  // Get the command port name.
  try
  {
    cmd_port = device.get_cmd_port_name();
  }
  catch (const jrk::error &)
  {
    cmd_port = "?";
  }

  // Get the TTL port name.
  try
  {
    ttl_port = device.get_ttl_port_name();
  }
  catch (const jrk::error &)
  {
    ttl_port = "?";
  }

  // Load the settings from the device.
  try
  {
    settings = device_handle.get_settings();
    handle_settings_loaded();
  }
  catch (const std::exception & e)
  {
    show_exception(e, "There was an error loading settings from the device.");
  }

  // Clear the variables read from the device because they don't apply anymore.
  variables.pointer_reset();
  current_chopping_count = 0;

  window->reset_graph();

  window->reset_error_counts();

  handle_model_changed();
}

void main_controller::disconnect_device_by_error(std::string const & error_message)
{
  really_disconnect();
  disconnected_by_user = false;
  set_connection_error(error_message);
}

void main_controller::really_disconnect()
{
  device_handle.close();
  settings_modified = false;
}

void main_controller::set_connection_error(std::string const & error_message)
{
  connection_error = true;
  connection_error_message = error_message;
}

void main_controller::reload_settings(bool ask)
{
  if (!connected()) { return; }

  std::string question = "Are you sure you want to reload settings from the "
    "device and discard your recent changes?";
  if (ask && !window->confirm(question))
  {
    return;
  }

  try
  {
    settings = device_handle.get_settings();
    handle_settings_loaded();
  }
  catch (std::exception const & e)
  {
    settings_modified = true;
    show_exception(e, "There was an error loading the settings from the device.");
  }
  handle_settings_changed();
}

void main_controller::restore_default_settings()
{
  if (!connected()) { return; }

  std::string question = "This will reset all of your device's settings "
    "back to their default values.  "
    "You will lose your custom settings.  "
    "Are you sure you want to continue?";
  if (!window->confirm(question))
  {
    return;
  }

  bool restore_success = false;
  try
  {
    device_handle.restore_defaults();
    restore_success = true;
  }
  catch (std::exception const & e)
  {
    show_exception(e);
  }

  // This takes care of reloading the settings and telling the view to update.
  reload_settings(false);

  if (restore_success)
  {
    window->show_info_message(
      "Your device's settings have been reset to their default values.");
  }
}

void main_controller::upgrade_firmware()
{
  if (connected())
  {
    std::string question =
      "This action will restart the device in bootloader mode, which "
      "is used for firmware upgrades.  The device will disconnect "
      "and reappear to your system as a new device.\n\n"
      "Are you sure you want to proceed?";
    if (!window->confirm(question))
    {
      return;
    }

    try
    {
      device_handle.start_bootloader();
    }
    catch (const std::exception & e)
    {
      show_exception(e);
    }

    really_disconnect();
    disconnected_by_user = true;
    connection_error = false;
    handle_model_changed();
  }

  window->open_bootloader_window();
}

void main_controller::upgrade_firmware_complete()
{
  // After a firmware upgrade is complete, allow the GUI to reconnect to the
  // device automatically.
  disconnected_by_user = false;
}

// Returns true if the device list includes the specified device.
static bool device_list_includes(
  const std::vector<jrk::device> & device_list,
  const jrk::device & device)
{
  return device_with_os_id(device_list, device.get_os_id()).is_present();
}

void main_controller::update()
{
  // This is called regularly by the view when it is time to check for
  // updates to the state of USB devices.  This runs on the same thread as
  // everything else, so we should be careful not to do anything too slow
  // here.  If the user tries to use the UI at all while this function is
  // running, the UI cannot respond until this function returns.

  bool successfully_updated_list = false;
  if (--update_device_list_counter == 0)
  {
    update_device_list_counter = UPDATE_DEVICE_LIST_DIVIDER;

    successfully_updated_list = update_device_list();
    if (successfully_updated_list && device_list_changed)
    {
      window->set_device_list_contents(device_list);
      if (connected())
      {
        window->set_device_list_selected(device_handle.get_device());
      }
      else
      {
        window->set_device_list_selected(jrk::device()); // show "Not connected"
      }
    }
  }

  if (connected())
  {
    // First, see if the device we are connected to is still available.
    // Note: It would be nice if the libusbp::generic_handle class had a
    // function that tests if the actual handle we are using is still valid.
    // This would be better for tricky cases like if someone unplugs and
    // plugs the same device in very fast.
    bool device_still_present = device_list_includes(
      device_list, device_handle.get_device());

    if (device_still_present)
    {
      // Reload the variables from the device.
      try
      {
        reload_variables();
      }
      catch (std::exception const & e)
      {
        // Ignore the exception.  The model provides other ways to tell that
        // the variable update failed, and the exact message is probably
        // not that useful since it is probably just a generic problem with
        // the USB connection.
      }
      handle_variables_changed();
    }
    else
    {
      // The device is gone.
      disconnect_device_by_error("The connection to the device was lost.");
      handle_model_changed();
    }
  }
  else
  {
    // We are not connected, so consider auto-connecting to a device.

    if (connection_error)
    {
      // There is an error related to a previous connection or connection
      // attempt, so don't automatically reconnect.  That would be
      // confusing, because the user might be looking away and not notice
      // that the connection was lost and then regained, or they could be
      // trying to read the error message.
    }
    else if (disconnected_by_user)
    {
      // The user explicitly disconnected the last connection, so don't
      // automatically reconnect.
    }
    else if (successfully_updated_list && (device_list.size() == 1))
    {
      // Automatically connect if there is only one device and we were not
      // recently disconnected from a device.
      connect_device(device_list.at(0));
    }
  }
}

bool main_controller::exit()
{
  if (connected() && settings_modified)
  {
    std::string question =
      "The settings you changed have not been applied to the device.  "
      "If you exit now, those changes will be lost.  "
      "Are you sure you want to exit?";
    return window->confirm(question);
  }
  else
  {
    return true;
  }
}

static bool device_lists_different(std::vector<jrk::device> const & list1,
  std::vector<jrk::device> const & list2)
{
  if (list1.size() != list2.size())
  {
    return true;
  }
  else
  {
    for (int i = 0; i < list1.size(); i++)
    {
      if (list1.at(i).get_os_id() != list2.at(i).get_os_id())
      {
        return true;
      }
    }
    return false;
  }
}

bool main_controller::update_device_list()
{
  try
  {
    std::vector<jrk::device> new_device_list = jrk::list_connected_devices();
    if (device_lists_different(device_list, new_device_list))
    {
      device_list_changed = true;
    }
    else
    {
      device_list_changed = false;
    }
    device_list = new_device_list;
    return true;
  }
  catch (std::exception const & e)
  {
    set_connection_error("Failed to get the list of devices.");
    show_exception(e, "There was an error getting the list of devices.");
    return false;
  }
}

void main_controller::show_exception(std::exception const & e,
    std::string const & context)
{
  std::string message;
  if (context.size() > 0)
  {
    message += context;
    message += "  ";
  }
  message += e.what();
  window->show_error_message(message);
}

void main_controller::handle_model_changed()
{
  handle_device_changed();
  handle_variables_changed();
  handle_settings_changed();
}

void main_controller::handle_device_changed()
{
  if (connected())
  {
    const jrk::device & device = device_handle.get_device();

    window->set_device_list_selected(device);

    window->set_device_name(jrk_look_up_product_name_ui(device.get_product()), true);
    window->set_serial_number(device.get_serial_number());
    window->set_firmware_version(device_handle.get_firmware_version_string());

    window->set_cmd_port(cmd_port);
    window->set_ttl_port(ttl_port);

    window->set_connection_status("", false);
  }
  else
  {
    window->set_device_list_selected(jrk::device()); // show "Not connected"

    std::string na = "N/A";
    window->set_device_name(na, false);
    window->set_serial_number(na);
    window->set_firmware_version(na);
    window->set_cmd_port(na);
    window->set_ttl_port(na);
    window->set_motor_status_message("");

    if (connection_error)
    {
      window->set_connection_status(connection_error_message, true);
    }
    else
    {
      window->set_connection_status("", false);
    }
  }

  window->set_disconnect_enabled(connected());
  window->set_open_save_settings_enabled(connected());
  window->set_reload_settings_enabled(connected());
  window->set_restore_defaults_enabled(connected());
  window->set_tab_pages_enabled(connected());
}

void main_controller::handle_variables_changed()
{
  window->set_device_reset(
    jrk_look_up_device_reset_name_ui(variables.get_device_reset()));

  window->set_up_time(variables.get_up_time());

  window->set_input(variables.get_input());
  // TODO: also show nice units in parentheses

  window->set_target(variables.get_target());

  if (cached_settings.get_feedback_mode() == JRK_FEEDBACK_MODE_NONE)
  {
    window->set_feedback_not_applicable();
  }
  else
  {
    window->set_feedback(variables.get_feedback());
    // TODO: also show version with nice units in parentheses

    window->set_scaled_feedback(variables.get_scaled_feedback());
    window->set_error(variables.get_error());
    window->set_integral(variables.get_integral());
  }
  window->set_duty_cycle_target(variables.get_duty_cycle_target());
  window->set_duty_cycle(variables.get_duty_cycle());

  // Note: The cached_settings we have here might not correspond to the
  // variables we have fetched if the settings were just applied, so this
  // calculation might be off at that time, but it's not a big deal.
  window->set_current(
    jrk::calculate_measured_current_ma(cached_settings, variables));
  window->set_raw_current_mv(
    jrk::calculate_raw_current_mv64(cached_settings, variables) / 64);

  // Tell the window if current chopping is happening now.
  window->set_current_chopping_now(variables.get_current_chopping_occurrence_count() > 0);

  // Give it the running tally of current chopping events.
  window->set_current_chopping_count(current_chopping_count);

  window->set_vin_voltage(variables.get_vin_voltage());
  window->set_pid_period_count(variables.get_pid_period_count());
  window->set_pid_period_exceeded(variables.get_pid_period_exceeded());

  window->set_error_flags_halting(variables.get_error_flags_halting());
  window->increment_errors_occurred(variables.get_error_flags_occurred());

  bool error_active = variables.get_error_flags_halting() != 0;
  window->set_stop_motor_enabled(connected());
  window->set_run_motor_enabled(connected() && error_active);

  if (connected() && variables.is_present())
  {
    window->update_graph(variables.get_up_time());
    window->set_motor_status_message(jrk::diagnose(cached_settings, variables),
      variables.get_error_flags_halting());
  }
}

void main_controller::handle_settings_changed()
{
  window->set_input_mode(settings.get_input_mode());
  window->set_input_analog_samples_exponent(settings.get_input_analog_samples_exponent());
  window->set_input_detect_disconnect(settings.get_input_detect_disconnect());
  window->set_input_serial_mode(settings.get_serial_mode());
  window->set_input_baud_rate(settings.get_serial_baud_rate());
  window->set_input_enable_crc(settings.get_serial_enable_crc());
  window->set_input_device_number(settings.get_serial_device_number());
  window->set_input_enable_device_number(settings.get_serial_enable_14bit_device_number());
  window->set_input_serial_timeout(settings.get_serial_timeout());
  window->set_input_compact_protocol(settings.get_serial_disable_compact_protocol());
  window->set_input_never_sleep(settings.get_never_sleep());
  window->set_input_invert(settings.get_input_invert());
  window->set_input_error_minimum(settings.get_input_error_minimum());
  window->set_input_error_maximum(settings.get_input_error_maximum());
  window->set_input_minimum(settings.get_input_minimum());
  window->set_input_maximum(settings.get_input_maximum());
  window->set_input_neutral_minimum(settings.get_input_neutral_minimum());
  window->set_input_neutral_maximum(settings.get_input_neutral_maximum());
  window->set_input_output_minimum(settings.get_output_minimum());
  window->set_input_output_neutral(settings.get_output_neutral());
  window->set_input_output_maximum(settings.get_output_maximum());
  window->set_input_scaling_degree(settings.get_input_scaling_degree());
  window->set_input_scaling_order_warning_label();
  window->set_feedback_mode(settings.get_feedback_mode());
  window->set_feedback_invert(settings.get_feedback_invert());
  window->set_feedback_error_minimum(settings.get_feedback_error_minimum());
  window->set_feedback_error_maximum(settings.get_feedback_error_maximum());
  window->set_feedback_maximum(settings.get_feedback_maximum());
  window->set_feedback_minimum(settings.get_feedback_minimum());
  window->set_feedback_scaling_order_warning_label();
  window->set_feedback_mode(settings.get_feedback_mode());
  window->set_feedback_analog_samples_exponent(settings.get_feedback_analog_samples_exponent());
  window->set_feedback_detect_disconnect(settings.get_feedback_detect_disconnect());
  window->set_feedback_wraparound(settings.get_feedback_wraparound());

  window->set_pid_period(settings.get_pid_period());
  window->set_integral_limit(settings.get_integral_limit());
  window->set_reset_integral(settings.get_reset_integral());
  window->set_feedback_dead_zone(settings.get_feedback_dead_zone());

  window->set_pid_proportional(settings.get_proportional_multiplier(),
    settings.get_proportional_exponent());
  window->set_pid_integral(settings.get_integral_multiplier(),
    settings.get_integral_exponent());
  window->set_pid_derivative(settings.get_derivative_multiplier(),
    settings.get_derivative_exponent());

  window->set_pwm_frequency(settings.get_pwm_frequency());
  window->set_motor_invert(settings.get_motor_invert());

  window->set_max_duty_cycle_while_feedback_out_of_range(
    settings.get_max_duty_cycle_while_feedback_out_of_range());

  window->set_coast_when_off(settings.get_coast_when_off());
  window->set_motor_invert(settings.get_motor_invert());

  window->set_motor_asymmetric(motor_asymmetric);

  window->set_max_duty_cycle_reverse(settings.get_max_duty_cycle_reverse());
  window->set_max_acceleration_reverse(settings.get_max_acceleration_reverse());
  window->set_max_deceleration_reverse(settings.get_max_deceleration_reverse());
  window->set_brake_duration_reverse(settings.get_brake_duration_reverse());
  window->set_current_limit_code_reverse(settings.get_current_limit_code_reverse());
  window->set_max_current_reverse(settings.get_max_current_reverse());

  window->set_max_duty_cycle_forward(settings.get_max_duty_cycle_forward());
  window->set_max_acceleration_forward(settings.get_max_acceleration_forward());
  window->set_max_deceleration_forward(settings.get_max_deceleration_forward());
  window->set_brake_duration_forward(settings.get_brake_duration_forward());
  window->set_current_limit_code_forward(settings.get_current_limit_code_forward());
  window->set_max_current_forward(settings.get_max_current_forward());

  {
    std::ostringstream meaning;
    meaning << "(";
    meaning << convert_current_limit_ma_to_string(
      jrk::current_limit_code_to_ma(
        settings, settings.get_current_limit_code_forward()));
    if (motor_asymmetric)
    {
      meaning << ", ";
      meaning << convert_current_limit_ma_to_string(
        jrk::current_limit_code_to_ma(
          settings, settings.get_current_limit_code_reverse()));
    }
    meaning << ")";
    window->set_current_limit_meaning(meaning.str().c_str());
  }

  window->set_current_offset_calibration(settings.get_current_offset_calibration());
  window->set_current_scale_calibration(settings.get_current_scale_calibration());
  window->set_current_samples_exponent(settings.get_current_samples_exponent());
  window->set_overcurrent_threshold(settings.get_overcurrent_threshold());

  window->set_error_enable(settings.get_error_enable(), settings.get_error_latch());

  window->set_apply_settings_enabled(connected() && settings_modified);
}

void main_controller::handle_settings_loaded()
{
  recalculate_motor_asymmetric();

  window->set_manual_target_enabled(
    settings.get_input_mode() == JRK_INPUT_MODE_SERIAL);

  if (settings.get_feedback_mode() == JRK_FEEDBACK_MODE_NONE)
  {
    window->set_manual_target_range(1448, 2648);
  }
  else
  {
    window->set_manual_target_range(0, 4095);
  }

  cached_settings = settings;

  settings_modified = false;
}

void main_controller::recalculate_motor_asymmetric()
{
  motor_asymmetric =
    (settings.get_max_duty_cycle_forward() !=
      settings.get_max_duty_cycle_reverse()) ||
    (settings.get_max_acceleration_forward() !=
      settings.get_max_acceleration_reverse()) ||
    (settings.get_max_deceleration_forward() !=
      settings.get_max_deceleration_reverse()) ||
    (settings.get_brake_duration_forward() !=
      settings.get_brake_duration_reverse()) ||
    (settings.get_current_limit_code_forward() !=
      settings.get_current_limit_code_reverse()) ||
    (settings.get_max_current_forward() !=
      settings.get_max_current_reverse());
}

void main_controller::handle_input_mode_input(uint8_t input_mode)
{
  if (!connected()) { return; }
  settings.set_input_mode(input_mode);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_analog_samples_exponent_input(uint8_t exponent)
{
  if (!connected()) { return; }
  settings.set_input_analog_samples_exponent(exponent);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_detect_disconnect_input(bool detect_disconnect)
{
  if (!connected()) { return; }
  settings.set_input_detect_disconnect(detect_disconnect);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_serial_mode_input(uint8_t value)
{
  if (!connected()) { return; }
  settings.set_serial_mode(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_uart_fixed_baud_input(uint32_t value)
{
  if (!connected()) { return; }
  settings.set_serial_baud_rate(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_enable_crc_input(bool value)
{
  if (!connected()) { return; }
  settings.set_serial_enable_crc(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_device_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_serial_device_number(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_device_number_input(bool value)
{
  if (!connected()) { return; }
  settings.set_serial_enable_14bit_device_number(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_timeout_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_serial_timeout(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_disable_compact_protocol_input(bool value)
{
  if (!connected()) { return; }
  settings.set_serial_disable_compact_protocol(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_never_sleep_input(bool value)
{
  if (!connected()) { return; }
  settings.set_never_sleep(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_invert_input(bool input_invert)
{
  if (!connected()) { return; }
  settings.set_input_invert(input_invert);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_error_minimum_input(uint16_t input_error_minimum)
{
  if (!connected()) { return; }
  settings.set_input_error_minimum(input_error_minimum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_error_maximum_input(uint16_t input_error_maximum)
{
  if (!connected()) { return; }
  settings.set_input_error_maximum(input_error_maximum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_minimum_input(uint16_t input_minimum)
{
  if (!connected()) { return; }
  settings.set_input_minimum(input_minimum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_maximum_input(uint16_t input_maximum)
{
  if (!connected()) { return; }
  settings.set_input_maximum(input_maximum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_neutral_minimum_input(uint16_t input_neutral_minimum)
{
  if (!connected()) { return; }
  settings.set_input_neutral_minimum(input_neutral_minimum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_neutral_maximum_input(uint16_t input_neutral_maximum)
{
  if (!connected()) { return; }
  settings.set_input_neutral_maximum(input_neutral_maximum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_output_minimum_input(uint16_t output_minimum)
{
  if (!connected()) { return; }
  settings.set_output_minimum(output_minimum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_output_neutral_input(uint16_t output_neutral)
{
  if (!connected()) { return; }
  settings.set_output_neutral(output_neutral);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_output_maximum_input(uint16_t output_maximum)
{
  if (!connected()) { return; }
  settings.set_output_maximum(output_maximum);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_input_scaling_degree_input(uint8_t input_scaling_degree)
{
  if (!connected()) { return; }
  settings.set_input_scaling_degree(input_scaling_degree);
  settings_modified = true;
  handle_settings_changed();
}

bool main_controller::check_settings_applied_before_wizard()
{
  if (settings_modified)
  {
    if (!window->apply_and_continue("This wizard cannot be used right now because"
      "the settings you changed have not been applied to the device.\n\n"
      "Please click \"Apply settings\" to apply your changes to the device or "
      "select \"Reload settings from device\" in the Device menu to discard "
      "your changes, then try again."))
    {
      return false;
    }
    else
    {
      apply_settings();
      return true;
    }
  }

  return true;
}

void main_controller::update_motor_status_message(bool prompt_to_resume)
{

}


void main_controller::handle_input_learn()
{
  if (!connected()) { return; }
  if (!check_settings_applied_before_wizard()) { return; }

  if (settings.get_input_mode() != JRK_INPUT_MODE_ANALOG &&
    settings.get_input_mode() != JRK_INPUT_MODE_PULSE_WIDTH)
  {
    // Should not happen because the button is disabled.
    window->show_info_message(
      "This wizard helps you set the input scaling parameters for the "
      "pulse width (RC) or analog input.  "
      "Please change the input mode to pulse width or analog, then try again.");
    return;
  }

  // Stop the motor so it isn't moving while people are learning the input.
  stop_motor();

  window->run_input_wizard(settings.get_input_mode());
}

void main_controller::handle_feedback_mode_input(uint8_t feedback_mode)
{
  if (!connected()) { return; }
  settings.set_feedback_mode(feedback_mode);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_invert_input(bool invert_feedback)
{
  if (!connected()) { return; }
  settings.set_feedback_invert(invert_feedback);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_error_minimum_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_feedback_error_minimum(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_error_maximum_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_feedback_error_maximum(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_maximum_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_feedback_maximum(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_minimum_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_feedback_minimum(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_analog_samples_exponent_input(uint8_t exponent)
{
  if (!connected()) { return; }
  settings.set_feedback_analog_samples_exponent(exponent);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_detect_disconnect_input(bool value)
{
  if (!connected()) { return; }
  settings.set_feedback_detect_disconnect(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_wraparound_input(bool value)
{
  if (!connected()) { return; }
  settings.set_feedback_wraparound(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_pid_proportional_input(uint16_t multiplier, uint8_t exponent)
{
  if (!connected()) { return; }
  settings.set_proportional_multiplier(multiplier);
  settings.set_proportional_exponent(exponent);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_pid_integral_input(uint16_t multiplier, uint8_t exponent)
{
  if (!connected()) { return; }
  settings.set_integral_multiplier(multiplier);
  settings.set_integral_exponent(exponent);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_pid_derivative_input(uint16_t multiplier, uint8_t exponent)
{
  if (!connected()) { return; }
  settings.set_derivative_multiplier(multiplier);
  settings.set_derivative_exponent(exponent);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_pid_period_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_pid_period(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_integral_limit_input(uint16_t value)
{
  if (!connected()) { return; }
  settings.set_integral_limit(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_reset_integral_input(bool value)
{
  if (!connected()) { return; }
  settings.set_reset_integral(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_feedback_dead_zone_input(uint8_t value)
{
  if (!connected()) { return; }
  settings.set_feedback_dead_zone(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_pwm_frequency_input(uint8_t pwm_frequency)
{
  if (!connected()) { return; }
  settings.set_pwm_frequency(pwm_frequency);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_motor_invert_input(bool motor_invert)
{
  if (!connected()) { return; }
  settings.set_motor_invert(motor_invert);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_motor_asymmetric_input(bool asymmetric)
{
  if (!connected()) { return; }
  motor_asymmetric = asymmetric;

  if (!motor_asymmetric)
  {
    settings.set_max_duty_cycle_reverse(settings.get_max_duty_cycle_forward());
    settings.set_max_acceleration_reverse(settings.get_max_acceleration_forward());
    settings.set_max_deceleration_reverse(settings.get_max_deceleration_forward());
    settings.set_brake_duration_reverse(settings.get_brake_duration_forward());
    settings.set_current_limit_code_reverse(settings.get_current_limit_code_forward());
    settings.set_max_current_reverse(settings.get_max_current_forward());
  }

  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_duty_cycle_forward_input(uint16_t duty_cycle)
{
  if (!connected()) { return; }
  settings.set_max_duty_cycle_forward(duty_cycle);
  if (!motor_asymmetric)
  {
    settings.set_max_duty_cycle_reverse(duty_cycle);
  }
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_duty_cycle_reverse_input(uint16_t duty_cycle)
{
  if (!connected()) { return; }
  settings.set_max_duty_cycle_reverse(duty_cycle);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_acceleration_forward_input(uint16_t acceleration)
{
  if (!connected()) { return; }
  settings.set_max_acceleration_forward(acceleration);
  if (!motor_asymmetric)
  {
    settings.set_max_acceleration_reverse(acceleration);
  }
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_acceleration_reverse_input(uint16_t acceleration)
{
  if (!connected()) { return; }
  settings.set_max_acceleration_reverse(acceleration);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_deceleration_forward_input(uint16_t deceleration)
{
  if (!connected()) { return; }
  settings.set_max_deceleration_forward(deceleration);
  if (!motor_asymmetric)
  {
    settings.set_max_deceleration_reverse(deceleration);
  }
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_deceleration_reverse_input(uint16_t deceleration)
{
  if (!connected()) { return; }
  settings.set_max_deceleration_reverse(deceleration);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_brake_duration_forward_input(uint32_t deceleration)
{
  if (!connected()) { return; }
  settings.set_brake_duration_forward(deceleration);
  if (!motor_asymmetric)
  {
    settings.set_brake_duration_reverse(deceleration);
  }
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_brake_duration_reverse_input(uint32_t deceleration)
{
  if (!connected()) { return; }
  settings.set_brake_duration_reverse(deceleration);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_current_limit_forward_input(uint16_t current)
{
  if (!connected()) { return; }
  settings.set_current_limit_code_forward(current);
  if (!motor_asymmetric)
  {
    settings.set_current_limit_code_reverse(current);
  }
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_current_limit_reverse_input(uint16_t current)
{
  if (!connected()) { return; }
  settings.set_current_limit_code_reverse(current);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_current_forward_input(uint16_t current)
{
  if (!connected()) { return; }
  settings.set_max_current_forward(current);
  if (!motor_asymmetric)
  {
    settings.set_max_current_reverse(current);
  }
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_current_reverse_input(uint16_t current)
{
  if (!connected()) { return; }
  settings.set_max_current_reverse(current);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_current_offset_calibration_input(int16_t cal)
{
  if (!connected()) { return; }
  settings.set_current_offset_calibration(cal);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_current_scale_calibration_input(int16_t cal)
{
  if (!connected()) { return; }
  settings.set_current_scale_calibration(cal);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_current_samples_exponent_input(uint8_t exponent)
{
  if (!connected()) { return; }
  settings.set_current_samples_exponent(exponent);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_overcurrent_threshold_input(uint8_t threshold)
{
  if (!connected()) { return; }
  settings.set_overcurrent_threshold(threshold);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_max_duty_cycle_while_feedback_out_of_range_input(
  uint16_t value)
{
  if (!connected()) { return; }
  settings.set_max_duty_cycle_while_feedback_out_of_range(value);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_coast_when_off_input(bool motor_coast)
{
  if (!connected()) { return; }
  settings.set_coast_when_off(motor_coast);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_error_enable_input(int index, int id)
{
  if (!connected()) { return; }
  uint16_t enable_value = settings.get_error_enable();
  uint16_t latch_value = settings.get_error_latch();

  if (id == 0)
  {
    enable_value &= ~(1 << index);
    latch_value &= ~(1 << index);
  }
  else if (id == 1)
  {
    enable_value |= 1 << index;
    latch_value &= ~(1 << index);
  }
  else if (id == 2)
  {
    enable_value |= 1 << index;
    latch_value |= 1 << index;
  }
  settings.set_error_enable(enable_value);
  settings.set_error_latch(latch_value);

  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_clear_errors_input()
{
  if (!connected()) { return; }
  window->set_error_flags_halting(device_handle.clear_errors());
}

void main_controller::handle_reset_counts_input()
{
  if (!connected()) { return; }
  window->reset_error_counts();
}

void main_controller::handle_never_sleep_input(bool never_sleep)
{
  if (!connected()) { return; }
  jrk_settings_set_never_sleep(settings.get_pointer(), never_sleep);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_vin_calibration_input(int16_t vin_calibration)
{
  if (!connected()) { return; }
  jrk_settings_set_vin_calibration(settings.get_pointer(), vin_calibration);
  settings_modified = true;
  handle_settings_changed();
}

void main_controller::handle_upload_complete()
{
  // After a firmware upgrade is complete, allow the GUI to reconnect to the
  // device automatically.
  disconnected_by_user = false;
}

void main_controller::apply_settings()
{
  if (!connected()) { return; }

  try
  {
    assert(connected());

    jrk::settings fixed_settings = settings;
    std::string warnings;
    fixed_settings.fix(&warnings);
    if (warnings.empty() ||
      window->confirm(warnings.append("\nAccept these changes and apply settings?")))
    {
      settings = fixed_settings;
      device_handle.set_settings(settings);
      device_handle.reinitialize();
      cached_settings = settings;
      settings_modified = false;  // this must be last in case exceptions are thrown
    }
  }
  catch (const std::exception & e)
  {
    show_exception(e);
  }
  handle_settings_loaded();
  handle_settings_changed();
}

void main_controller::stop_motor()
{
  if (!connected()) { return; }

  try
  {
    device_handle.stop_motor();
  }
  catch (const std::exception & e)
  {
    show_exception(e);
  }

  if (cached_settings.get_feedback_mode() == JRK_FEEDBACK_MODE_NONE)
  {
    // Set the inputs back to 2048 so it is easy to smoothly drag them to a new
    // speed, starting at a speed of zero.
    //
    // If the user wants to go back to the speed they were previously using, they
    // can just press 'Run motor' instead of messing with the scroll bar.
    window->set_manual_target_inputs(2048);
  }
}

void main_controller::run_motor()
{
  if (!connected()) { return; }

  try
  {
    device_handle.run_motor();
  }
  catch (const std::exception & e)
  {
    show_exception(e);
  }
}

void main_controller::set_target(uint16_t target)
{
  if (!connected()) { return; }

  try
  {
    device_handle.set_target(target);
  }
  catch (const std::exception & e)
  {
    show_exception(e);
  }
}

void main_controller::clear_current_chopping_count()
{
  current_chopping_count = 0;
  handle_variables_changed();
}

void main_controller::force_duty_cycle_target_nocatch(int16_t duty_cycle)
{
  device_handle.force_duty_cycle_target(duty_cycle);
}

void main_controller::clear_errors_nocatch()
{
  device_handle.clear_errors();
}

void main_controller::open_settings_from_file(std::string filename)
{
  if (!connected()) { return; }

  try
  {
    assert(connected());

    std::string settings_string = read_string_from_file(filename);
    jrk::settings fixed_settings = jrk::settings::read_from_string(settings_string);
    std::string warnings;
    fixed_settings.fix(&warnings);
    if (warnings.empty() ||
      window->confirm(warnings.append("\nAccept these changes and load settings?")))
    {
      settings = fixed_settings;
      settings_modified = true;
    }
  }
  catch (std::exception const & e)
  {
    show_exception(e);
  }

  handle_settings_changed();
}

void main_controller::save_settings_to_file(std::string filename)
{
  if (!connected()) { return; }

  try
  {
    assert(connected());

    jrk::settings fixed_settings = settings;
    std::string warnings;
    fixed_settings.fix(&warnings);
    if (!warnings.empty())
    {
      if (window->confirm(warnings.append("\nAccept these changes and save settings?")))
      {
        settings = fixed_settings;
        settings_modified = true;
      }
      else
      {
        return;
      }
    }
    std::string settings_string = settings.to_string();
    write_string_to_file(filename, settings_string);
  }
  catch (std::exception const & e)
  {
    show_exception(e);
  }

  handle_settings_changed();
}

void main_controller::reload_variables()
{
  assert(connected());

  try
  {
    uint16_t flags = (1 << JRK_GET_VARIABLES_FLAG_CLEAR_ERROR_FLAGS_OCCURRED) |
      (1 << JRK_GET_VARIABLES_FLAG_CLEAR_CURRENT_CHOPPING_OCCURRENCE_COUNT);
    variables = device_handle.get_variables(flags);
    variables_update_failed = false;
  }
  catch (...)
  {
    variables_update_failed = true;
    throw;
  }

  // Update the running total of current chopping occurrences.
  // We store the total in a uint32_t but still let's not let it exceed INT_MAX
  // because there is no need to.
  {
    uint8_t new_counts = variables.get_current_chopping_occurrence_count();
    if (current_chopping_count < INT_MAX - new_counts)
    {
      current_chopping_count += new_counts;
    }
    else
    {
      current_chopping_count = INT_MAX;
    }
  }
}
