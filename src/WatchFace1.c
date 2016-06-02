#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;
static TextLayer *s_time_layer;
static int s_battery_level;
static Layer *s_battery_layer;
static TextLayer *s_battery_percentage;
static TextLayer *s_date_layer;
static TextLayer *s_weather_layer;
static BitmapLayer *s_bt_icon_layer;
static GBitmap *s_bt_icon_bitmap;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  //static char conditions_buffer[32];
  //static char weather_layer_buffer[32];

  APP_LOG(APP_LOG_LEVEL_DEBUG, "received Callback");
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dc", (int)temp_tuple->value->int32);
    //snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    //snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_weather_layer, temperature_buffer);
  }
}

static void bluetooth_callback(bool connected) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Got BT callback");
  
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "BT not connected");
    vibes_double_pulse();
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  static char s_buffer[8];
  static char d_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  strftime(d_buffer, sizeof(d_buffer), "%a %d", tick_time); 
  
  text_layer_set_text(s_time_layer, s_buffer);
  text_layer_set_text(s_date_layer, d_buffer);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  int size;
  GRect bounds = layer_get_bounds(layer);
  
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  
  if(s_battery_level > 75){
    if(s_battery_level == 100){
      size = bounds.size.w;
    }else{
      size = (int)(float)(((float)(s_battery_level % 25) / 25.0F) * (bounds.size.w));
    }    
    graphics_fill_rect(ctx, GRect((bounds.size.w - size + 2), 1, (size-4), 2), 0, GCornerNone);
    
    graphics_fill_rect(ctx, GRect((bounds.size.w-4), 1, 2, bounds.size.h-8), 0, GCornerNone);
  }
  
  if(s_battery_level >= 50){
    if(s_battery_level == 50){
      size = 0;
    }else{
      size = (int)(float)(((float)(s_battery_level % 25) / 25.0F) * (bounds.size.h));
    }
    
    graphics_fill_rect(ctx, GRect((bounds.size.w-4), ((bounds.size.h - size + 2)), 2, size-4), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(1, (bounds.size.h-4), (bounds.size.w-4), 2), 0, GCornerNone);
  }
  
  if(s_battery_level > 25){
    size = (int)(float)(((float)(s_battery_level % 25) / 25.0F) * (bounds.size.w));
    graphics_fill_rect(ctx, GRect(2, bounds.size.h-4, size-4, 2), 0, GCornerNone);
    
    graphics_fill_rect(ctx, GRect(1, 1, 2, (bounds.size.h-3)), 0, GCornerNone);  
  }else{
    size = (int)(float)(((float)s_battery_level / 25.0F) * (bounds.size.h));
    graphics_fill_rect(ctx, GRect(1, 1, 2, (size-4)), 0, GCornerNone);
  }
   
}

static void main_window_load(Window *window) {
  static GFont s_h1_font;
  static GFont s_h2_font;
  
  s_h1_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BERLIN_SANS_FB_48));
  s_h2_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BERLIN_SANS_FB_20));
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_layer = text_layer_create(GRect(4, 20, (bounds.size.w-8), 50));
  s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  s_battery_percentage = text_layer_create(GRect(bounds.size.w-28, 4, 20, 25));
  s_date_layer = text_layer_create(GRect(4, 75, (bounds.size.w-8), 35));
  s_weather_layer = text_layer_create(GRect(4, 115, (bounds.size.w-8), 35));
  s_bt_icon_layer = bitmap_layer_create(GRect(5, 5, 12, 16));
  
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  //battery percentage
  text_layer_set_background_color(s_battery_percentage, GColorBlack);
  text_layer_set_text_color(s_battery_percentage, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_battery_percentage));
  
  //time layer
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_h1_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  //date layer
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_h2_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  
  //temp layer
  text_layer_set_background_color(s_weather_layer, GColorBlack);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_font(s_weather_layer, s_h2_font);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "..loading..");
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  
  //connection layer
  s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PHONE_CONNECTED);
  bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_icon_layer));
  
  bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_battery_percentage);
  layer_destroy(s_battery_layer);
  gbitmap_destroy(s_bt_icon_bitmap);
  bitmap_layer_destroy(s_bt_icon_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer);
  
  //static char s_buffer[8];
  //snprintf(s_buffer,sizeof(s_buffer), "%d", s_battery_level);
   
  //text_layer_set_text(s_battery_percentage, s_buffer);
}

static void init(){
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init::main_window_created");
  
  window_stack_push(s_main_window, true);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init::updateTime");
  update_time();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init::setBatteryState");
  battery_callback(battery_state_service_peek());
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init::Register Timer Ticker");
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init::Register Battery event");
  battery_state_service_subscribe(battery_callback);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init::register AppMessage events");
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init::register BT event");
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
}

static void deinit(){
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}