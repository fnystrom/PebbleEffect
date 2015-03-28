#include "pebble.h"

static void send_monthforecast_cmd(void);

Window *window;

TextLayer *current_power_layer;
TextLayer *estimate_power_layer;
TextLayer *momentan_layer;
TextLayer *text1_layer;
TextLayer *text2_layer;
TextLayer *text3_layer;
TextLayer *text4_layer;
TextLayer *connection_layer;
TextLayer *temperature_layer;
TextLayer *battery_layer;
TextLayer *upper_layer;
InverterLayer *inverter_layer;

TextLayer *text5_layer;
TextLayer *text6_layer;
TextLayer *so_far_layer;
TextLayer *estimate_month_layer;

static AppSync sync;
static uint8_t sync_buffer[128];
//static int flickMode;

enum EffectKey {
  //EFFECT_CURRENT_KEY = 0,
  EFFECT_ESTIMATE_KEY = 1,
	EFFECT_FORECAST_KEY = 2,
	EFFECT_BACKGROUND_KEY = 3,
	EFFECT_TEMPERATURE_KEY = 6,
	EFFECT_CURRENTPOWER_KEY = 7,
	EFFECT_FORECASTPOWER_KEY = 8,
	EFFECT_TEMPERATUREREQUEST_KEY = 9,
	EFFECT_TEMPERATUREMODE_KEY = 10,
	EFFECT_JUSTNU_KEY = 11,
	EFFECT_TOTALTIDAG_KEY = 12,
	EFFECT_FLICKMODE_KEY = 13,
  EFFECT_MONTHFORECASTPOWER_KEY = 14,
  EFFECT_SOFAR_KEY = 15,
  EFFECT_MONTHFORECAST_KEY = 16
};

#define TEXT_JUST_NU "Just nu:"
#define TEXT_HITTILLS "Idag:"
#define TEXT_PROGNOS "Prognos:"
#define TEXT_LADDAR "         "
#define TEXT_TEMPERATUR "Temp:"
#define MANADSPROGNOS "Månadsprognos"
#define HITTILLS "Hittills:"

#define DAY_VIEW 0
#define MONTH_VIEW 1
	
static void tap_handler(AccelAxisType axis, int32_t direction) {
  int flickMode = persist_read_int(EFFECT_FLICKMODE_KEY);
	  
  if (flickMode == DAY_VIEW) {
    flickMode = MONTH_VIEW;
	  
	  layer_set_hidden((Layer *)text1_layer, true);
	  layer_set_hidden((Layer *)text3_layer, true);
	  layer_set_hidden((Layer *)current_power_layer, true);
	  layer_set_hidden((Layer *)estimate_power_layer, true);
	  layer_set_hidden((Layer *)momentan_layer, true);

    layer_set_hidden((Layer *)text5_layer, false);
    layer_set_hidden((Layer *)text6_layer, false);
    layer_set_hidden((Layer *)so_far_layer, false);
    layer_set_hidden((Layer *)estimate_month_layer, false);
    
    // Skicka order om att hämta månadsprognos
    send_monthforecast_cmd();
  } else if (flickMode == MONTH_VIEW) {
    flickMode = DAY_VIEW;

    layer_set_hidden((Layer *)text1_layer, false);
    layer_set_hidden((Layer *)text3_layer, false);
    layer_set_hidden((Layer *)current_power_layer, false);
    layer_set_hidden((Layer *)estimate_power_layer, false);
	  layer_set_hidden((Layer *)momentan_layer, false);

    layer_set_hidden((Layer *)text5_layer, true);
    layer_set_hidden((Layer *)text6_layer, true);
    layer_set_hidden((Layer *)so_far_layer, true);
    layer_set_hidden((Layer *)estimate_month_layer, true);
  }

  persist_write_int(EFFECT_FLICKMODE_KEY, flickMode);
}

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

static void send_cmd(int cmd) {
  Tuplet value = TupletInteger(cmd, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void send_temperature_cmd(void) {
  send_cmd(EFFECT_TEMPERATUREREQUEST_KEY);
}

static void send_forecast_cmd(void) {
  send_cmd(EFFECT_FORECASTPOWER_KEY);
}

static void send_current_cmd(void) {
  send_cmd(EFFECT_CURRENTPOWER_KEY);
}

static void send_monthforecast_cmd(void) {
  send_cmd(EFFECT_MONTHFORECASTPOWER_KEY);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "tuple: %d, %s", (int)key, new_tuple->value->cstring);

  int flickMode;

  switch (key) {
  	case EFFECT_TOTALTIDAG_KEY:
      text_layer_set_text(current_power_layer, new_tuple->value->cstring);
      break;

	  case EFFECT_JUSTNU_KEY:
      text_layer_set_text(momentan_layer, new_tuple->value->cstring);
      break;
	  
	  case EFFECT_TEMPERATURE_KEY:
      text_layer_set_text(temperature_layer, new_tuple->value->cstring);
      break;

	  case EFFECT_FORECAST_KEY:
      text_layer_set_text(estimate_power_layer, new_tuple->value->cstring);
      break;
    
    case EFFECT_SOFAR_KEY:
      text_layer_set_text(so_far_layer, new_tuple->value->cstring);
      break;
    
    case EFFECT_MONTHFORECAST_KEY:
      text_layer_set_text(estimate_month_layer, new_tuple->value->cstring);
      break;
	  
	  case EFFECT_TEMPERATUREMODE_KEY:
      flickMode = persist_read_int(EFFECT_FLICKMODE_KEY);
      
	    if (flickMode == DAY_VIEW) {
        if (strcmp(new_tuple->value->cstring, "show") == 0) {
  		    persist_write_bool(EFFECT_TEMPERATUREMODE_KEY, true);
  		    layer_set_hidden((Layer *)text4_layer, false);
  		    layer_set_hidden((Layer *)temperature_layer, false);
        } else {
		      persist_write_bool(EFFECT_TEMPERATUREMODE_KEY, false);
		      layer_set_hidden((Layer *)text4_layer, true);
		      layer_set_hidden((Layer *)temperature_layer, true);
        } 
      }
      break;
	  
	  case EFFECT_BACKGROUND_KEY:
      if (strcmp(new_tuple->value->cstring, "black") == 0) {
          persist_write_bool(EFFECT_BACKGROUND_KEY, false);
      } else {
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
  static char time_text[] = "00:00";

  strftime(time_text, sizeof(time_text), "%R", tick_time);
  text_layer_set_text(upper_layer, time_text);

  static char second_text[] = "00";

  strftime(second_text, sizeof(second_text), "%S", tick_time);

  static char minute_text[] = "00";

  strftime(minute_text, sizeof(minute_text), "%M", tick_time);

  if(strcmp(second_text, "05") == 0){
    send_temperature_cmd();
  }
	   
  if(strcmp(second_text, "01") == 0 ||
     strcmp(second_text, "16") == 0 ||
  	 strcmp(second_text, "31") == 0 ||
	   strcmp(second_text, "46") == 0) {
     send_current_cmd();
  }
	
  if(strcmp(second_text, "00") == 0 && ((strcmp(minute_text, "30") == 0) ||
	                                    (strcmp(minute_text, "31") == 0) ||
	                                    (strcmp(minute_text, "32") == 0) ||
	                                    (strcmp(minute_text, "33") == 0) ||
	                                    (strcmp(minute_text, "34") == 0) ||
	                                    (strcmp(minute_text, "35") == 0) ||
	                                    (strcmp(minute_text, "40") == 0) ||
	                                    (strcmp(minute_text, "50") == 0) ||
  	                                  (strcmp(minute_text, "00") == 0) ||
	                                    (strcmp(minute_text, "10") == 0) ||
	                                    (strcmp(minute_text, "20") == 0))) {
    send_forecast_cmd();
  }
	
  handle_battery(battery_state_service_peek());
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(connection_layer, connected ? "ansluten" : "ej ansluten");
}

static TextLayer* createTextLayer(GRect rect, GFont font, GTextAlignment text_alignment, const char * text){
  Layer *window_layer = window_get_root_layer(window);
  
  TextLayer* layer = text_layer_create(rect);
  text_layer_set_text_color(layer, GColorWhite);
  text_layer_set_background_color(layer, GColorClear);
  text_layer_set_font(layer, font);
  text_layer_set_text_alignment(layer, text_alignment);
  layer_add_child(window_layer, text_layer_get_layer(layer));
  text_layer_set_text(layer, text);
  
  return layer;
}

static void window_load(Window *window) {
  int column = 65;
  int titleoffset = 3;
  int rowspace = 24;
  int bottomline = 119;
  int row0 = bottomline - 3 * rowspace;
  int row1 = bottomline - 2 * rowspace;
  int row2 = bottomline - rowspace;
  int row3 = bottomline;

  persist_write_int(EFFECT_FLICKMODE_KEY, DAY_VIEW);
	
  upper_layer = createTextLayer(GRect(0, 0, 144, 68), fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49), GTextAlignmentCenter, "00:00");
  text4_layer = createTextLayer(GRect(0, row0+titleoffset, column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentLeft, TEXT_TEMPERATUR);
  temperature_layer = createTextLayer(GRect(column, row0, 144-column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentRight, "");
  text3_layer = createTextLayer(GRect(0, row1+titleoffset, column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentLeft, TEXT_JUST_NU);
  momentan_layer = createTextLayer(GRect(column, row1, 144-column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentRight, "");
  text1_layer = createTextLayer(GRect(0, row2+titleoffset, column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentLeft, TEXT_HITTILLS);
  current_power_layer = createTextLayer(GRect(column, row2, 144-column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentRight, "");
  text2_layer = createTextLayer(GRect(0, row3+titleoffset, column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentLeft, TEXT_PROGNOS);
  estimate_power_layer = createTextLayer(GRect(column, row3, 144-column, 68), fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentRight, "");
  connection_layer = createTextLayer(GRect(0, 145, 72, 34), fonts_get_system_font(FONT_KEY_GOTHIC_18), GTextAlignmentLeft, "");
  battery_layer = createTextLayer(GRect(72, 145, 72, 34), fonts_get_system_font(FONT_KEY_GOTHIC_18), GTextAlignmentRight, "");
  inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));

  text5_layer = createTextLayer(GRect(0, row1, 144, 35), fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), GTextAlignmentLeft, MANADSPROGNOS);
  text6_layer = createTextLayer(GRect(0, row2+titleoffset, column, 35), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentLeft, HITTILLS);
  so_far_layer = createTextLayer(GRect(column-20, row2+titleoffset, 144-column+20, 35), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentRight, "9999kWh");
  estimate_month_layer = createTextLayer(GRect(column-20, row3+titleoffset, 144-column+20, 35), fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GTextAlignmentRight, "9999kWh");

  layer_set_hidden((Layer *)text5_layer, true);
  layer_set_hidden((Layer *)text6_layer, true);
  layer_set_hidden((Layer *)so_far_layer, true);
  layer_set_hidden((Layer *)estimate_month_layer, true);

  handle_bluetooth(bluetooth_connection_service_peek());

  battery_state_service_subscribe(&handle_battery);
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  bool inv = persist_read_bool(EFFECT_BACKGROUND_KEY);
  char *string = "black";
  if(inv){
	  string = "white";
  }
  	
  bool tempmode = persist_read_bool(EFFECT_TEMPERATUREMODE_KEY);
  char *s_tempmode = "hide";
  if(tempmode){
	s_tempmode = "show";
  }
	
  Tuplet initial_values[] = {
    TupletCString(EFFECT_TOTALTIDAG_KEY, TEXT_LADDAR),
    TupletCString(EFFECT_FORECAST_KEY, TEXT_LADDAR),
  	TupletCString(EFFECT_BACKGROUND_KEY, string),
	  TupletCString(EFFECT_TEMPERATURE_KEY, "     "),
	  TupletCString(EFFECT_TEMPERATUREMODE_KEY, s_tempmode),
	  TupletCString(EFFECT_JUSTNU_KEY, TEXT_LADDAR),
    TupletCString(EFFECT_SOFAR_KEY, TEXT_LADDAR),
    TupletCString(EFFECT_MONTHFORECAST_KEY, TEXT_LADDAR)
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
  text_layer_destroy(momentan_layer);
  text_layer_destroy(text1_layer);
  text_layer_destroy(text2_layer);
  text_layer_destroy(text3_layer);
  text_layer_destroy(text4_layer);
  text_layer_destroy(temperature_layer);
  text_layer_destroy(connection_layer);
  text_layer_destroy(battery_layer);

  text_layer_destroy(text5_layer);
  text_layer_destroy(text6_layer);
  text_layer_destroy(so_far_layer);
  text_layer_destroy(estimate_month_layer);
}

static void init(void) {
  window = window_create();
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  const int inbound_size = 128;
  const int outbound_size = 128;
  app_message_open(inbound_size, outbound_size);

  const bool animated = true;
  window_stack_push(window, animated);

  // Subscribe to the accelerometer tap service
  accel_tap_service_subscribe(tap_handler);
}

static void deinit(void) {
  window_destroy(window);

  accel_tap_service_unsubscribe();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
