#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0xC4, 0xA7, 0xFB, 0x23, 0x4D, 0x4B, 0x4E, 0x25, 0xBD, 0x7E, 0x91, 0xC1, 0x54, 0x66, 0xCA, 0x3A }

//UUID, app name, author, minor version, major version, icon
PBL_APP_INFO(MY_UUID, "Pebble Digital Frame", "Chris Miller", 1, 0, DEFAULT_MENU_ICON, APP_INFO_WATCH_FACE);


//  APP_LOG(APP_LOG_LEVEL_INFO, "Message");

#define HOUR_BUF_LEN 3
#define MIN_BUF_LEN 3
#define AMPM_BUF_LEN 3
#define DATE_BUF_LEN 12

#define HOUR_24_FORMAT "%H"
#define HOUR_12_FORMAT "%I"
#define DATE_FORMAT "%a, %b %d"
#define MINUTE_FORMAT "%M"
#define AM_PM_FORMAT "%p"
#define HOUR_MIN_FORMAT "%s:%s"

#define DATE_FONT FONT_KEY_ROBOTO_CONDENSED_21
#define TIME_FONT FONT_KEY_BITHAM_42_BOLD
#define AMPM_FONT FONT_KEY_GOTHIC_18


Window window;

Layer     timeLayer;
Layer     lineLayer;

TextLayer hrMinLayer;
TextLayer ampmLayer;
TextLayer dateLayer;

//static buffer for string_format_time
static char hourBuffer[HOUR_BUF_LEN];
static char minBuffer[MIN_BUF_LEN];
static char ampmBuffer[AMPM_BUF_LEN];
static char dateBuffer[DATE_BUF_LEN];

static char is24HourTime = false;

void update_hour_minutes(PblTm *t) {
  static char buf[5];
  string_format_time(hourBuffer, sizeof(hourBuffer), is24HourTime ? HOUR_24_FORMAT : HOUR_12_FORMAT, t);
  string_format_time(minBuffer, sizeof(minBuffer), MINUTE_FORMAT, t);
 
  if (!is24HourTime && (hourBuffer[0] == '0')) {
    memmove(hourBuffer, &hourBuffer[1], sizeof(hourBuffer) - 1);
  }

  snprintf(buf, sizeof buf, HOUR_MIN_FORMAT, hourBuffer, minBuffer);
  text_layer_set_text(&hrMinLayer, buf);
}

void update_am_pm(PblTm *t) {
  if (is24HourTime) strcpy(ampmBuffer, "  ");
  else string_format_time(ampmBuffer, sizeof(ampmBuffer), AM_PM_FORMAT, t);
  text_layer_set_text(&ampmLayer, (ampmBuffer));
}

void update_date(PblTm *t) {
  string_format_time(dateBuffer, sizeof(dateBuffer), DATE_FORMAT, t);
  text_layer_set_text(&dateLayer, dateBuffer);
}

void line_layer_callback(Layer *l, GContext *ctx) {
  (void)l; //not used
  graphics_context_set_stroke_color(ctx, GColorWhite);
  graphics_draw_line(ctx, GPoint(0, 35), GPoint(144, 35));
  graphics_draw_line(ctx, GPoint(0, 36), GPoint(144, 36));
}

void line_layers_init() {
  layer_init(&lineLayer, window.layer.frame);
  lineLayer.update_proc = &line_layer_callback;
  layer_add_child(&window.layer, &lineLayer);
}

void create_text_layer(TextLayer *l, GRect rect, GColor color, GFont font, GTextAlignment align) {
  text_layer_init(l, rect);
  text_layer_set_background_color(l, GColorClear);
  text_layer_set_text_color(l, color);
  text_layer_set_text_alignment(l, align);
  text_layer_set_font(l, fonts_get_system_font(font));
}

void date_layer_init(PblTm *t) {
  create_text_layer(&dateLayer, GRect(0, 2, 144, 34), GColorWhite, DATE_FONT, GTextAlignmentCenter);
  layer_add_child(&window.layer, &dateLayer.layer);
  update_date(t);
}

void time_layer_init(PblTm *t) {
  //hour:minute
  create_text_layer(&hrMinLayer, GRect(is24HourTime ? 15 : 6, 46, 112, 50), GColorWhite, TIME_FONT, GTextAlignmentCenter);
  //am/pm
  create_text_layer(&ampmLayer, GRect(106, 66, 34, 54), GColorWhite, AMPM_FONT, GTextAlignmentCenter);

  update_hour_minutes(t);
  update_am_pm(t);

  layer_add_child(&window.layer, &hrMinLayer.layer);
  layer_add_child(&window.layer, &ampmLayer.layer);
}

void handle_init(AppContextRef ctx) {
  (void) ctx; // Dont need this
  
  window_init(&window, "Digital Frame");
  window_stack_push(&window, true /* Animated */);
  window_set_background_color(&window, GColorBlack);

  PblTm time; //time struct
  get_time(&time); //update stuct with current time
  is24HourTime = clock_is_24h_style();

  line_layers_init();
  date_layer_init(&time);
  time_layer_init(&time);
}

//@todo dealloc
void handle_deinit(AppContextRef ctx) {
  (void) ctx; // Dont need this
  
}

void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void) ctx; // Dont need this
  PblTm time;
  get_time(&time);
  update_hour_minutes(t->tick_time);
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_tick,
      .tick_units = MINUTE_UNIT
    }
  };
  app_event_loop(params, &handlers);
}
