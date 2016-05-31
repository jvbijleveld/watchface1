#include <pebble.h>
static Window *s_main_window;
static TextLayer *s_time_layer;
static int s_battery_level;
static Layer *s_battery_layer;
static TextLayer *s_battery_percentage;
static TextLayer *s_date_layer;

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
  static GFont s_time_font;
  static GFont s_date_font;
  
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BERLIN_SANS_FB_48));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_BERLIN_SANS_FB_20));
  
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_time_layer = text_layer_create(GRect(4, 20, (bounds.size.w-8), 50));
  s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  s_battery_percentage = text_layer_create(GRect(bounds.size.w-28, 4, 20, 25));
  s_date_layer = text_layer_create(GRect(4, 75, (bounds.size.w-8), 35));
  
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  //battery percentage
  text_layer_set_background_color(s_battery_percentage, GColorBlack);
  text_layer_set_text_color(s_battery_percentage, GColorWhite);
  layer_add_child(window_layer, text_layer_get_layer(s_battery_percentage));
  
  //time layer
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  //date layer
  text_layer_set_background_color(s_date_layer, GColorBlack);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window){
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  layer_destroy(s_battery_layer);
  text_layer_destroy(s_battery_percentage);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void battery_callback(BatteryChargeState state) {
  s_battery_level = state.charge_percent;
  layer_mark_dirty(s_battery_layer);
  
  static char s_buffer[8];
  snprintf(s_buffer,sizeof(s_buffer), "%d", s_battery_level);
   
  //text_layer_set_text(s_battery_percentage, s_buffer);
}

static void init(){
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_callback);

  window_stack_push(s_main_window, true);
  battery_callback(battery_state_service_peek());
  update_time();
}

static void deinit(){
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}