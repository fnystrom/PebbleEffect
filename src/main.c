#include "pebble.h"

static Window *window;

static TextLayer *temperature_layer;
static TextLayer *city_layer;
static TextLayer *apa_layer;
static TextLayer *text1_layer;
static TextLayer *text2_layer;
static TextLayer *connection_layer;
static TextLayer *upper_layer;

static AppSync sync;
static uint8_t sync_buffer[64];

enum EffectKey {
    EFFECT_CURRENT_KEY = 0,
    EFFECT_ESTIMATE_KEY = 1,
	EFFECT_APA_KEY = 2
};
		
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
	case EFFECT_CURRENT_KEY:
      text_layer_set_text(temperature_layer, new_tuple->value->cstring);
      break;

	case EFFECT_ESTIMATE_KEY:
      text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
	  
	  case EFFECT_APA_KEY:
      text_layer_set_text(apa_layer, new_tuple->value->cstring);
      break;
  }
}

// Called once per second
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00:00"; // Needs to be static because it's used by the system later.

  strftime(time_text, sizeof(time_text), "%T", tick_time);
  text_layer_set_text(upper_layer, time_text);
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

static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
  static char time_text[] = "00:00"; // Needs to be static because it's used by the system later.

  strftime(time_text, sizeof(time_text), "%R", tick_time);
  text_layer_set_text(upper_layer, time_text);
	
  send_cmd();
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
	
  temperature_layer = text_layer_create(GRect(0, 70, 144, 68));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));

  text2_layer = text_layer_create(GRect(0, 95, 144, 68));
  text_layer_set_text_color(text2_layer, GColorWhite);
  text_layer_set_background_color(text2_layer, GColorClear);
  text_layer_set_font(text2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text2_layer));
  text_layer_set_text(text2_layer, "Prognos:");
	
  apa_layer = text_layer_create(GRect(0, 115, 144, 68));
  text_layer_set_text_color(apa_layer, GColorWhite);
  text_layer_set_background_color(apa_layer, GColorClear);
  text_layer_set_font(apa_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(apa_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(apa_layer));

  connection_layer = text_layer_create(GRect(0, 145, 144, 34));
  text_layer_set_text_color(connection_layer, GColorWhite);
  text_layer_set_background_color(connection_layer, GColorClear);
  text_layer_set_font(connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(connection_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(connection_layer));
  handle_bluetooth(bluetooth_connection_service_peek());

  bluetooth_connection_service_subscribe(&handle_bluetooth);

  Tuplet initial_values[] = {
    TupletCString(EFFECT_CURRENT_KEY, "laddar..."),
    TupletCString(EFFECT_APA_KEY, "laddar...")
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

//  send_cmd();

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  //handle_second_tick(current_time, SECOND_UNIT);

  //tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

  handle_minute_tick(current_time, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}


static void window_unload(Window *window) {
  app_sync_deinit(&sync);
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  text_layer_destroy(upper_layer);
  text_layer_destroy(city_layer);
  text_layer_destroy(temperature_layer);
  text_layer_destroy(apa_layer);
  text_layer_destroy(text1_layer);
  text_layer_destroy(text2_layer);
  text_layer_destroy(connection_layer);
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
