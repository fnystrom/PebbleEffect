#include "pebble.h"
	
Window *window;

TextLayer *current_power_layer;
TextLayer *estimate_power_layer;
TextLayer *text1_layer;
TextLayer *text2_layer;
TextLayer *connection_layer;
TextLayer *battery_layer;
TextLayer *upper_layer;
InverterLayer *inverter_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

enum EffectKey {
    EFFECT_CURRENT_KEY = 0,
    EFFECT_ESTIMATE_KEY = 1,
	EFFECT_FORECAST_KEY = 2,
	EFFECT_BACKGROUND_KEY = 3,
	EFFECT_MODE_KEY = 5
};
		
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

void update_configuration(void)
{
    static bool old_inv = 0;    /* default to not inverted */

    if (persist_exists(EFFECT_BACKGROUND_KEY))
    {
        bool inv = persist_read_bool(EFFECT_BACKGROUND_KEY);

        if (inv != old_inv)
        {
            Layer *inv_layer = inverter_layer_get_layer(inverter_layer);

            if (inv)
            {
                layer_add_child(window_get_root_layer(window), inv_layer);
            }
            else
            {
                layer_remove_from_parent(inv_layer);
            }

            old_inv = inv;
        }
    }
}

static void send_cmd(void) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "tuple: %d, %s", (int)key, new_tuple->value->cstring);

  switch (key) {
	case EFFECT_CURRENT_KEY:
      text_layer_set_text(current_power_layer, new_tuple->value->cstring);
      break;

	case EFFECT_FORECAST_KEY:
      text_layer_set_text(estimate_power_layer, new_tuple->value->cstring);
      break;
	  
	case EFFECT_MODE_KEY:
	  if (strcmp(new_tuple->value->cstring, "forecast") == 0)
      {
		  if (!persist_read_bool(EFFECT_MODE_KEY)){send_cmd();}
          persist_write_bool(EFFECT_MODE_KEY, true);
		  text_layer_set_text(text1_layer, "Effekt idag:");
		  text_layer_set_text(text2_layer, "Prognos:");
      }
      else
      {
          if (persist_read_bool(EFFECT_MODE_KEY)){send_cmd();}
		  persist_write_bool(EFFECT_MODE_KEY, false);
		  text_layer_set_text(text1_layer, "Just nu:");
		  text_layer_set_text(text2_layer, "Diff:");
      }
      break;
	  
	case EFFECT_BACKGROUND_KEY:
      if (strcmp(new_tuple->value->cstring, "black") == 0)
      {
          persist_write_bool(EFFECT_BACKGROUND_KEY, false);
      }
      else
      {
          persist_write_bool(EFFECT_BACKGROUND_KEY, true);
      }
	  update_configuration();
      break;
	  
  	default:
	  //APP_LOG(APP_LOG_LEVEL_DEBUG, "default");
	  break;	
  }
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% laddat";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "laddar");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  bool forecast = persist_read_bool(EFFECT_MODE_KEY);
  
  static char time_text[] = "00:00";

  strftime(time_text, sizeof(time_text), "%R", tick_time);
  text_layer_set_text(upper_layer, time_text);

  static char second_text[] = "00";

  strftime(second_text, sizeof(second_text), "%S", tick_time);

  static char minute_text[] = "00";

  strftime(minute_text, sizeof(minute_text), "%M", tick_time);

  if(!forecast){
    if(strcmp(second_text, "00") == 0 ||
	   strcmp(second_text, "10") == 0 ||
	   strcmp(second_text, "20") == 0 ||
	   strcmp(second_text, "30") == 0 ||
	   strcmp(second_text, "40") == 0 ||
	   strcmp(second_text, "50") == 0){
        send_cmd();   
	  }		
  } else if(forecast && strcmp(second_text, "00") == 0){
    if ((strcmp(minute_text, "30") == 0) ||
	    (strcmp(minute_text, "31") == 0) ||
	    (strcmp(minute_text, "32") == 0) ||
	    (strcmp(minute_text, "33") == 0) ||
	    (strcmp(minute_text, "34") == 0) ||
	    (strcmp(minute_text, "35") == 0) ||
	    (strcmp(minute_text, "40") == 0) ||
	    (strcmp(minute_text, "50") == 0) ||
  	    (strcmp(minute_text, "00") == 0) ||
	    (strcmp(minute_text, "10") == 0) ||
	    (strcmp(minute_text, "20") == 0))
    {
      send_cmd();
    }
  }
	
  handle_battery(battery_state_service_peek());
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(connection_layer, connected ? "ansluten" : "ej ansluten");
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  upper_layer = text_layer_create(GRect(0, 0, 144, 68));
  text_layer_set_text_color(upper_layer, GColorWhite);
  text_layer_set_background_color(upper_layer, GColorClear);
  text_layer_set_font(upper_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(upper_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(upper_layer));
  text_layer_set_text(upper_layer, "00:00");
	
  text1_layer = text_layer_create(GRect(0, 50, 144, 68));
  text_layer_set_text_color(text1_layer, GColorWhite);
  text_layer_set_background_color(text1_layer, GColorClear);
  text_layer_set_font(text1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text1_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text1_layer));
  text_layer_set_text(text1_layer, "Effekt idag:");
	
  current_power_layer = text_layer_create(GRect(0, 70, 144, 68));
  text_layer_set_text_color(current_power_layer, GColorWhite);
  text_layer_set_background_color(current_power_layer, GColorClear);
  text_layer_set_font(current_power_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(current_power_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(current_power_layer));

  text2_layer = text_layer_create(GRect(0, 95, 144, 68));
  text_layer_set_text_color(text2_layer, GColorWhite);
  text_layer_set_background_color(text2_layer, GColorClear);
  text_layer_set_font(text2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text2_layer));
  text_layer_set_text(text2_layer, "Prognos:");
	
  estimate_power_layer = text_layer_create(GRect(0, 115, 144, 68));
  text_layer_set_text_color(estimate_power_layer, GColorWhite);
  text_layer_set_background_color(estimate_power_layer, GColorClear);
  text_layer_set_font(estimate_power_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(estimate_power_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(estimate_power_layer));

  connection_layer = text_layer_create(GRect(0, 145, 72, 34));
  text_layer_set_text_color(connection_layer, GColorWhite);
  text_layer_set_background_color(connection_layer, GColorClear);
  text_layer_set_font(connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(connection_layer, GTextAlignmentLeft);
  layer_add_child(window_layer, text_layer_get_layer(connection_layer));

  battery_layer = text_layer_create(GRect(72, 145, 72, 34));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));

  inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  
  handle_bluetooth(bluetooth_connection_service_peek());

  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  bool inv = persist_read_bool(EFFECT_BACKGROUND_KEY);
  char *string = "black";
  if(inv){
	  string = "white";
  }
  
  bool forecast = persist_read_bool(EFFECT_MODE_KEY);
  char *s_forecast = "now";
  if(forecast){
	s_forecast = "forecast";
  }
	
  Tuplet initial_values[] = {
    TupletCString(EFFECT_CURRENT_KEY, "laddar..."),
    TupletCString(EFFECT_FORECAST_KEY, "laddar..."),
	TupletCString(EFFECT_BACKGROUND_KEY, string),
	TupletCString(EFFECT_MODE_KEY, s_forecast)
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT);
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

  update_configuration();
}

static void window_unload(Window *window) {
  app_sync_deinit(&sync);
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();
  inverter_layer_destroy(inverter_layer);

  text_layer_destroy(upper_layer);
  text_layer_destroy(current_power_layer);
  text_layer_destroy(estimate_power_layer);
  text_layer_destroy(text1_layer);
  text_layer_destroy(text2_layer);
  text_layer_destroy(connection_layer);
  text_layer_destroy(battery_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
