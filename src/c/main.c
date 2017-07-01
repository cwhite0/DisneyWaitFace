#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
typedef struct {
  int index;
  char name[30];
  char time[30];
  char id[40];
} WaitTime;

typedef struct {
  int index;
  char time[30];
} EntertainmentTime;

typedef struct {
  int index;
  char name[50];
  char type[25];
  char time[50];
} ItineraryItem;

typedef struct {
    void **data;
    int size;
    int count;
} vector;

// APP MESSAGE KEYS

enum {
  INDEX = 0x0,
  NAME = 0x1,
  WAIT_TIME = 0x2,
  GET_WAIT_TIMES = 0x3,
  CANCEL_MESSAGES = 0x4,
  ID = 0x5,
  LOCATION = 0x6,
  DESCRIPTION = 0x7,
  GET_ATTRACTION_INFO = 0x8,
  GET_ENTERTAINMENT = 0x9,
  ENTERTAINMENT_STATUS = 10,
  GET_SCHEDULE = 11,
  SCHEDULE_TIME = 12,
  TYPE = 13,
  TIME = 14,
  GET_ITINERARY = 15,
  ERROR_TITLE = 16,
  ERROR_DESC = 17,
   RLIST = 19
  
};



static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_weather_layer;
static  TextLayer *s_rlist_layer;



int minutes = 0;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[500];

  // Read tuples for data
  static char conditions_buffer[500];
  static char weather_layer_buffer[500];
  Tuple *temp_tuple = dict_find(iterator, NAME);
  Tuple *temp2 = dict_find(iterator, RLIST);

  // If all data is available, use it
  if(temp_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", temp_tuple->value->cstring);

    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
  if(temp2) {
  snprintf(conditions_buffer,sizeof(conditions_buffer), "%s", temp2->value->cstring);
    text_layer_set_text(s_rlist_layer,conditions_buffer);
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

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Get Disney Updates every 5
  
  if(tick_time->tm_min % 5 == 0) {
    text_layer_set_text(s_weather_layer, "Loading...");
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create GBitmap
 // s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND);

  // Create BitmapLayer to display the GBitmap
 // s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
  //bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  //layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(0, 0), bounds.size.w, 15));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorWhite);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Create GFont
  //s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_48));

  // Apply to TextLayer
  //text_layer_set_font(s_time_layer, s_time_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // Create temperature Layer
  s_weather_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(20, 15), bounds.size.w,bounds.size.h));

  // Style the text
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentRight);
  text_layer_set_text(s_weather_layer, "Loading...");

  s_rlist_layer = text_layer_create(GRect(0,PBL_IF_ROUND_ELSE(20,15),bounds.size.w,bounds.size.h));
  text_layer_set_text_color(s_rlist_layer,GColorWhite);
  text_layer_set_background_color(s_rlist_layer,GColorClear);
  text_layer_set_text_alignment(s_rlist_layer, GTextAlignmentRight);
  text_layer_set_text(s_rlist_layer, "loading...");
  layer_set_hidden((Layer *)s_rlist_layer,true);
  // Create secoynd custom font, apply it and add to Wipndow
  //s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_PERFECT_DOS_));
  //text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_rlist_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  //fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  //gbitmap_destroy(s_background_bitmap);

  // Destroy BitmapLayer
  //bitmap_layer_destroy(s_background_layer);

  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_rlist_layer);


  //fonts_unload_custom_font(s_weather_font);
}
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Subscribe to tap events
  //accel_tap_service_subscribe(accel_tap_handler);
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

 static void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // A tap event occured
   layer_set_hidden((Layer *) s_rlist_layer, !layer_get_hidden((Layer *) s_rlist_layer));
  
     layer_set_hidden((Layer *) s_weather_layer, !layer_get_hidden((Layer *) s_weather_layer));
   }


static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
  // Unsubscribe from tap events
accel_tap_service_unsubscribe();
}

int main(void) {
  init();
  
accel_tap_service_subscribe(accel_tap_handler);
  app_event_loop();
  deinit();
}