#include <pebble.h>
#include <stdio.h>
#include <stdlib.h>

// info fetched from API is sent to pebble via following keys  
// weather info
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

// stock info
#define KEY_SYMBOL 2
#define KEY_CURRENT 3
#define KEY_CHANGE 4

// face display of stock
#define KEY_FACE 5

// initialize mainwindow and layers  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
static TextLayer *s_stock_layer;

// alarm layer is to show message response according to the change of stock
static TextLayer *s_alarm_layer;

// initialize custom fonts
static GFont s_time_font;
static GFont s_weather_font;
static GFont s_stock_font;
static GFont s_alarm_font;

// background for time
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

// background for happy/sad face
static BitmapLayer *s_stock_background_layer;
static GBitmap *s_stock_background_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create GBitmap for happy face as default
  s_stock_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAPPY);
  s_stock_background_layer = bitmap_layer_create(GRect(100,5,42,42));
  bitmap_layer_set_background_color(s_stock_background_layer, GColorWhite);
  bitmap_layer_set_bitmap(s_stock_background_layer, s_stock_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stock_background_layer));
    
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(5, 56, 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

  // Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create temperature Layer
  s_weather_layer = text_layer_create(GRect(0, 115, 144, 25));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  
  // Create second custom font of size 20, apply it and add to Window
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

  // Create stock layer at top of watch
  // left part has two lines containing symbol, askValue and change
  s_stock_layer = text_layer_create(GRect(0,5,100,50));
  text_layer_set_background_color(s_stock_layer, GColorClear);
  text_layer_set_text_color(s_stock_layer, GColorWhite);
  text_layer_set_text_alignment(s_stock_layer, GTextAlignmentLeft);
  text_layer_set_text(s_stock_layer, "Loading.^.");
  
  // Create second custom font of size 16 for stock info
  s_stock_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_16));
  text_layer_set_font(s_stock_layer, s_stock_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_stock_layer));
  
  
  // Create alarm info layer (when stock goes too high/low)
  // The message box stays at the bottom right of the watchface
  s_alarm_layer = text_layer_create(GRect(0, 140, 144, 25));
  text_layer_set_background_color(s_alarm_layer, GColorClear);
  text_layer_set_text_color(s_alarm_layer, GColorWhite);
  text_layer_set_text_alignment(s_alarm_layer, GTextAlignmentRight);
  text_layer_set_text(s_alarm_layer, "...");
  
  // Create second custom font of 16, apply it and add to Window
  s_alarm_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_16));
  text_layer_set_font(s_alarm_layer, s_alarm_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_alarm_layer));
    
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
  
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
  
  // Destroy stock elements
  text_layer_destroy(s_stock_layer);
  fonts_unload_custom_font(s_stock_font);
  gbitmap_destroy(s_stock_background_bitmap);
  bitmap_layer_destroy(s_stock_background_layer);
  
  // Destroy alarm elements
  text_layer_destroy(s_alarm_layer);
  fonts_unload_custom_font(s_alarm_font);
  
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather/stock update every 20 minutes
  if(tick_time->tm_min % 20 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
  
}

static void face_change(Window *window, int flag){
  // function to update happy/sad face
  
  // first destroy current layer
  gbitmap_destroy(s_stock_background_bitmap);
  bitmap_layer_destroy(s_stock_background_layer);

  // recreate layer
  s_stock_background_layer = bitmap_layer_create(GRect(100,5,42,42));
  bitmap_layer_set_background_color(s_stock_background_layer, GColorWhite);
  
  if (flag == 1){
      // create new face layer/bitmap
      s_stock_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_SAD);
      text_layer_set_text(s_alarm_layer, "God bless");
  } 
  else if (flag == 0){
      // create new face layer/bitmap
      s_stock_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_HAPPY);
      text_layer_set_text(s_alarm_layer, "Keep up!");
  }
  
  // condition when value of stock doubles
  else if (flag == 2){
      // create new face layer/bitmap, use more delightful icon ":)"
      s_stock_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STOCKUP);    
      text_layer_set_text(s_alarm_layer, "$$$$$$$$$$$$$$$$");
  }
  
  // condition when value of stock shrinks to half, use more frustrated icon ":("
  else{
      // create new face layer/bitmap
    s_stock_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STOCKDOWN);
    text_layer_set_text(s_alarm_layer, "TAT, farewell");
  }

  // set new face display
  bitmap_layer_set_bitmap(s_stock_background_layer, s_stock_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_stock_background_layer));


}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  // Store weather info
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  // store stock info
  static char stock_symbol_buffer[8];
  static char stock_change_buffer[10];
  static char stock_current_buffer[8];
  static char stock_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // flag that decides face display
  int flag = 0;
  
  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    case KEY_SYMBOL:
      snprintf(stock_symbol_buffer, sizeof(stock_symbol_buffer), "%s", t->value->cstring);
      break;
    case KEY_CURRENT:
      snprintf(stock_current_buffer, sizeof(stock_current_buffer), "%s", t->value->cstring);
      break;
    case KEY_CHANGE:
      snprintf(stock_change_buffer, sizeof(stock_change_buffer), "%s", t->value->cstring);
      break;    
    case KEY_FACE:
      flag = (int)t->value->int32;
      // debug printout, 0,1,2,3 corresponds to definition in js file
      printf("flag received: %d\n", flag);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  // weather layer
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  // set current temperature, weather to textlayer
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
  
  // stock layer
  snprintf(stock_layer_buffer, sizeof(stock_layer_buffer), "%s %s\n%s", stock_symbol_buffer, stock_change_buffer, stock_current_buffer);
  // set current stock info to textlayer
  text_layer_set_text(s_stock_layer, stock_layer_buffer);
   
  // change face based on flag value
  face_change(s_main_window,flag);
  
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
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
