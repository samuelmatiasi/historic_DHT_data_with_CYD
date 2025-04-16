#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <DHT.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define DHTPIN 27
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define ARC_SIZE 130
#define MAX_DATA_POINTS 60

// Touchscreen pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

uint32_t draw_buf[SCREEN_WIDTH * SCREEN_HEIGHT / 10];

static lv_obj_t *arc_temp;
static lv_obj_t *arc_humi;
static lv_obj_t *label_temp;
static lv_obj_t *label_humi;
static lv_obj_t *screen_arc;
static lv_obj_t *screen_chart;
static lv_chart_series_t *series_temp;
static lv_chart_series_t *series_humi;
static lv_obj_t *chart;
static lv_obj_t *chart_title; // Added title label for the chart

static int screen_state = 0;

float temp_data[MAX_DATA_POINTS] = {0};
float humi_data[MAX_DATA_POINTS] = {0};
int data_index = 0;

void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
}

void update_sensor_data() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  temp_data[data_index] = t;
  humi_data[data_index] = h;
  data_index = (data_index + 1) % MAX_DATA_POINTS;

  if (screen_state == 0) {
    lv_arc_set_value(arc_temp, t);
    lv_label_set_text_fmt(label_temp, "%.1f\xB0C", t);
    lv_arc_set_value(arc_humi, h);
    lv_label_set_text_fmt(label_humi, "%.1f%%", h);
  } else if (screen_state == 1 && chart) {
    for (int i = 0; i < MAX_DATA_POINTS; i++) {
      series_temp->y_points[i] = temp_data[(data_index + i) % MAX_DATA_POINTS];
    }
    lv_chart_refresh(chart);
  } else if (screen_state == 2 && chart) {
    for (int i = 0; i < MAX_DATA_POINTS; i++) {
      series_humi->y_points[i] = humi_data[(data_index + i) % MAX_DATA_POINTS];
    }
    lv_chart_refresh(chart);
  }
}

void create_sensor_arcs() {
  lv_obj_t *title;
  screen_arc = lv_obj_create(NULL);
  lv_scr_load(screen_arc);

  arc_temp = lv_arc_create(screen_arc);
  lv_obj_set_size(arc_temp, ARC_SIZE, ARC_SIZE);
  lv_arc_set_rotation(arc_temp, 135);
  lv_arc_set_bg_angles(arc_temp, 0, 270);
  lv_arc_set_range(arc_temp, 0, 50);
  lv_arc_set_value(arc_temp, 0);
  lv_obj_align(arc_temp, LV_ALIGN_LEFT_MID, 20, 10);
  lv_obj_set_style_arc_color(arc_temp, lv_palette_main(LV_PALETTE_RED), LV_PART_INDICATOR);

  label_temp = lv_label_create(screen_arc);
  lv_label_set_text(label_temp, "0\xB0C");
  lv_obj_set_style_text_font(label_temp, &lv_font_montserrat_28, 0);  // Bigger font
  lv_obj_align_to(label_temp, arc_temp, LV_ALIGN_CENTER, 0, 0);


  title = lv_label_create(screen_arc);
  lv_label_set_text(title, "Temperature");
  lv_obj_align_to(title, arc_temp, LV_ALIGN_OUT_TOP_MID, 0, -5);

  arc_humi = lv_arc_create(screen_arc);
  lv_obj_set_size(arc_humi, ARC_SIZE, ARC_SIZE);
  lv_arc_set_rotation(arc_humi, 135);
  lv_arc_set_bg_angles(arc_humi, 0, 270);
  lv_arc_set_range(arc_humi, 0, 100);
  lv_arc_set_value(arc_humi, 0);
  lv_obj_align(arc_humi, LV_ALIGN_RIGHT_MID, -20, 10);
  lv_obj_set_style_arc_color(arc_humi, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);

  label_humi = lv_label_create(screen_arc);
  lv_label_set_text(label_humi, "0%");
  lv_obj_set_style_text_font(label_humi, &lv_font_montserrat_28, 0);  // Bigger font
  lv_obj_align_to(label_humi, arc_humi, LV_ALIGN_CENTER, 0, 0);

  title = lv_label_create(screen_arc);
  lv_label_set_text(title, "Humidity");
  lv_obj_align_to(title, arc_humi, LV_ALIGN_OUT_TOP_MID, 0, -5);
}

void create_chart_screen() {
  screen_chart = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(screen_chart, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(screen_chart, LV_OPA_COVER, LV_PART_MAIN);

  chart_title = lv_label_create(screen_chart);
  lv_label_set_text(chart_title, "Chart Title");
  lv_obj_align(chart_title, LV_ALIGN_TOP_MID, 0, 10);

  chart = lv_chart_create(screen_chart);
  lv_obj_set_size(chart, 230, 180); // Increased chart size
  lv_obj_center(chart);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE);

  // Reduce number of displayed points (e.g., 30 instead of MAX_DATA_POINTS)
  const int display_points = 30;
  lv_chart_set_point_count(chart, display_points);
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);

  lv_obj_set_style_bg_opa(chart, LV_OPA_COVER, 0);
  lv_obj_set_style_bg_color(chart, lv_color_white(), 0);
  lv_obj_set_style_border_width(chart, 0, 0);
  lv_obj_set_style_pad_all(chart, 10, 0);

  lv_chart_set_div_line_count(chart, 5, 6);

  lv_obj_t *label_y = lv_label_create(screen_chart);
  lv_label_set_text(label_y, "Values");
  lv_obj_align_to(label_y, chart, LV_ALIGN_OUT_LEFT_MID, -10, 0);

  lv_obj_t *label_x = lv_label_create(screen_chart);
  lv_label_set_text(label_x, "Time");
  lv_obj_align_to(label_x, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  series_temp = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  series_humi = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);

  for (int i = 0; i < display_points; i++) {
    series_temp->y_points[i] = 0;
    series_humi->y_points[i] = 0;
  }

  lv_chart_refresh(chart);
}


void handle_screen_cycle() {
  screen_state = (screen_state + 1) % 3;

  if (screen_state == 0) {
    lv_scr_load(screen_arc);
  } else if (screen_state == 1) {
    lv_scr_load(screen_chart);
    lv_label_set_text(chart_title, "Temperature History");
    lv_chart_set_series_color(chart, series_temp, lv_palette_main(LV_PALETTE_RED));
    lv_chart_set_series_color(chart, series_humi, lv_color_white());
  } else {
    lv_scr_load(screen_chart);
    lv_label_set_text(chart_title, "Humidity History");
    lv_chart_set_series_color(chart, series_temp, lv_color_white());
    lv_chart_set_series_color(chart, series_humi, lv_palette_main(LV_PALETTE_BLUE));
  }

  lv_chart_refresh(chart);
}

void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  static bool pressed = false;
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();
    int x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    int y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
    if (!pressed) {
      handle_screen_cycle();
      pressed = true;
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
    pressed = false;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("LVGL Sensor Arcs");

  lv_init();
  lv_log_register_print_cb(log_print);
  dht.begin();

  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  touchscreen.setRotation(2);

  lv_display_t * disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);

  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, touchscreen_read);

  create_sensor_arcs();
  create_chart_screen();
  lv_scr_load(screen_arc);
}

unsigned long last_update = 0;

void loop() {
  lv_task_handler();
  lv_tick_inc(5);
  delay(5);

  if (millis() - last_update > 2000) {
    update_sensor_data();
    last_update = millis();
  }
}


