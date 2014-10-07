#include <pebble.h>

#define RADIUS 10

static Window *window;
static TextLayer *day_layer, *charge_layer;
static Layer *main_layer;
static char day_info[13];
char mCharge[5];
static GBitmap *charge_bitmap;
static BitmapLayer *bitmap_layer;
static GBitmap *phone_bitmap;
static BitmapLayer *phone_layer;
bool connectionStatus = false;

char* days_of_week[7] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Fri", "Sat"};
char* months_of_year[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

void update_proc(Layer *layer, GContext *ctx) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_context_set_fill_color(ctx, GColorWhite);
    int i = 0;
    time_t time_value = time(NULL);
    struct tm* mTime = localtime(&time_value);

    unsigned int hour = mTime->tm_hour;
    if(clock_is_24h_style()) {
      for(i = 0; i < 5; i++)
	graphics_draw_circle(ctx, GPoint(18+26*i, 37), RADIUS);
      for(i = 0; i < 5; i++)
      {
	if(hour % 2)
	  graphics_fill_circle(ctx, GPoint(122-26*i, 37), RADIUS);
	hour /= 2;
      }

    } else {
      if(hour == 0)
	hour = 12;
      else if(hour != 12)
	hour = hour % 12;
      for(i = 0; i < 4; i++)
      {
	if(hour % 2) {
	  graphics_fill_circle(ctx, GPoint(122-33*i, 37), RADIUS);
	} else {
	  graphics_draw_circle(ctx, GPoint(122-33*i, 37), RADIUS);
	}
	hour /= 2;
      }
    }

    unsigned int min = mTime->tm_min;
    for(i = 0; i < 6; i++)
    {
        if(min % 2) {
            graphics_fill_circle(ctx, GPoint(128-23*i, 84), RADIUS);
	} else {
            graphics_draw_circle(ctx, GPoint(128-23*i, 84), RADIUS);
	}
        min /= 2;
    }

    unsigned int sec = mTime->tm_sec;
    for(i = 0; i < 6; i++)
    {
        if(sec % 2) {
            graphics_fill_circle(ctx, GPoint(128-23*i, 131), RADIUS);
	} else {
            graphics_draw_circle(ctx, GPoint(128-23*i, 131), RADIUS);
	}
        sec /= 2;
    }

    if (0 == mTime->tm_min && 0 == mTime->tm_sec) {
      vibes_double_pulse();
    }

    int day_int = mTime->tm_mday;
    char* day_string = "01";
    day_string[1] = '0' + day_int%10;
    day_string[0] = '0' + day_int/10;
    memset(day_info, 0, sizeof(day_info));
    strcpy(day_info, days_of_week[mTime->tm_wday]);
    strcat(day_info, " ");
    strcat(day_info, day_string);
    strcat(day_info, " ");
    strcat(day_info, months_of_year[mTime->tm_mon]);
    text_layer_set_text(day_layer, day_info);
}

void handle_tick(struct tm *tick_time, TimeUnits units_changed)
{
    layer_mark_dirty(main_layer);
}

void battery_state_handler(BatteryChargeState charge)
{
    int chargeLevel = charge.charge_percent;
    memset(mCharge, 0, sizeof(mCharge));
    if(chargeLevel >= 100 || (charge.is_plugged && !charge.is_charging))
    {
        mCharge[0] = '1';
        mCharge[1] = '0';
        mCharge[2] = '0';
    }
    else if(chargeLevel <= 0)
    {
        mCharge[0] = '0';
    }
    else
    {
        mCharge[0] = '0' + chargeLevel/10%10;
        mCharge[1] = '0' + chargeLevel%10;
    }
    strcat(mCharge, "%");
    text_layer_set_text(charge_layer, mCharge);
    if(charge.is_charging)
    {
        if(!charge_bitmap)
        {
            charge_bitmap = gbitmap_create_with_resource(RESOURCE_ID_CHARGE_WHITE);
            bitmap_layer_set_bitmap(bitmap_layer, charge_bitmap);
        }
    }
    else
    {
        if(charge_bitmap)
        {
            bitmap_layer_set_bitmap(bitmap_layer, NULL);
            gbitmap_destroy(charge_bitmap);
            charge_bitmap = NULL;
        }
    }
}

void bluetooth_state_handler(bool connected)
{
    if(!connected && connectionStatus)
    {
        vibes_double_pulse();
    }
    connectionStatus = connected;
    if(connected)
    {
        if(!phone_bitmap)
        {
            phone_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PHONE_WHITE);
            bitmap_layer_set_bitmap(phone_layer, phone_bitmap);
        }
    }
    else
    {
        if(phone_bitmap)
        {
            bitmap_layer_set_bitmap(phone_layer, NULL);
            gbitmap_destroy(phone_bitmap);
            phone_bitmap = NULL;
        }
    }
}

static void window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    day_layer = text_layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, 20 } });
    main_layer = layer_create((GRect) { .origin = { 0, 0 }, .size = { bounds.size.w, bounds.size.h } });
    charge_layer = text_layer_create((GRect) { .origin = {0, 0 }, .size = { 30, 20} });
    bitmap_layer = bitmap_layer_create((GRect) { .origin = {25, 0}, .size={12, 20}});
    bitmap_layer_set_background_color(bitmap_layer, GColorClear);
    bitmap_layer_set_compositing_mode(bitmap_layer, GCompOpAssignInverted);
    
    phone_layer = bitmap_layer_create((GRect) { .origin = {bounds.size.w - 12, 0}, .size={12, 20}});
    bitmap_layer_set_background_color(phone_layer, GColorClear);
    bitmap_layer_set_compositing_mode(phone_layer, GCompOpAssignInverted);
    
    text_layer_set_text(day_layer, "");
    text_layer_set_background_color(day_layer, GColorBlack);
    text_layer_set_text_color(day_layer, GColorWhite);
    text_layer_set_background_color(charge_layer, GColorBlack);
    text_layer_set_text_color(charge_layer, GColorWhite);
    text_layer_set_text_alignment(charge_layer, GTextAlignmentLeft);
    text_layer_set_text(charge_layer, "");
    text_layer_set_text_alignment(day_layer, GTextAlignmentCenter);
    
    layer_set_update_proc(main_layer, update_proc);
    layer_add_child(window_layer, main_layer);
    layer_add_child(window_layer, text_layer_get_layer(day_layer));
    layer_add_child(window_layer, text_layer_get_layer(charge_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
    layer_add_child(window_layer, bitmap_layer_get_layer(phone_layer));
    
    tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
    battery_state_service_subscribe(battery_state_handler);
    bluetooth_connection_service_subscribe(bluetooth_state_handler);
    battery_state_handler(battery_state_service_peek());
    bluetooth_state_handler(bluetooth_connection_service_peek());
}

static void window_unload(Window *window) {
    text_layer_destroy(day_layer);
    text_layer_destroy(charge_layer);
    layer_destroy(main_layer);
    bitmap_layer_destroy(bitmap_layer);
    bitmap_layer_destroy(phone_layer);
    battery_state_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
    if(charge_bitmap)
    {
        gbitmap_destroy(charge_bitmap);
        charge_bitmap = NULL;
    }
    if(phone_bitmap)
    {
        gbitmap_destroy(phone_bitmap);
        phone_bitmap = NULL;
    }
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
    window_set_background_color(window, GColorBlack);
    window_set_fullscreen(window, true);
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

/* vim: sw=2 */
