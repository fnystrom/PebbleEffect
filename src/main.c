#include "pebble.h"

static Window *window;

static TextLayer *temperature_layer;
static TextLayer *city_layer;
static TextLayer *apa_layer;
static TextLayer *text1_layer;
static TextLayer *text2_layer;
static TextLayer *text3_layer;

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

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  text1_layer = text_layer_create(GRect(0, 20, 144, 68));
  text_layer_set_text_color(text1_layer, GColorWhite);
  text_layer_set_background_color(text1_layer, GColorClear);
  text_layer_set_font(text1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(text1_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text1_layer));
  text_layer_set_text(text1_layer, "Total Effect:");
	
  temperature_layer = text_layer_create(GRect(0, 50, 144, 68));
  text_layer_set_text_color(temperature_layer, GColorWhite);
  text_layer_set_background_color(temperature_layer, GColorClear);
  text_layer_set_font(temperature_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(temperature_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(temperature_layer));

  text2_layer = text_layer_create(GRect(0, 80, 144, 68));
  text_layer_set_text_color(text2_layer, GColorWhite);
  text_layer_set_background_color(text2_layer, GColorClear);
  text_layer_set_font(text2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(text2_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text2_layer));
  text_layer_set_text(text2_layer, "Estimate:");
	
  apa_layer = text_layer_create(GRect(0, 110, 144, 68));
  text_layer_set_text_color(apa_layer, GColorWhite);
  text_layer_set_background_color(apa_layer, GColorClear);
  text_layer_set_font(apa_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(apa_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(apa_layer));

/*  text3_layer = text_layer_create(GRect(0, 125, 144, 68));
  text_layer_set_text_color(text3_layer, GColorWhite);
  text_layer_set_background_color(text3_layer, GColorClear);
  text_layer_set_font(text3_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(text3_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text3_layer));
  text_layer_set_text(text3_layer, "eliq");
  */
	
	/*
  city_layer = text_layer_create(GRect(0, 125, 144, 68));
  text_layer_set_text_color(city_layer, GColorWhite);
  text_layer_set_background_color(city_layer, GColorClear);
  text_layer_set_font(city_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(city_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(city_layer));
*/

	Tuplet initial_values[] = {
    TupletCString(EFFECT_CURRENT_KEY, "100"),
    /*TupletCString(EFFECT_ESTIMATE_KEY, "200"),*/
    TupletCString(EFFECT_APA_KEY, "300")
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

  send_cmd();
}


static void window_unload(Window *window) {
  app_sync_deinit(&sync);

  text_layer_destroy(city_layer);
  text_layer_destroy(temperature_layer);
  text_layer_destroy(apa_layer);
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
