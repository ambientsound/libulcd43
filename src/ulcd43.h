#ifndef _ULCD43_H_
#define _ULCD43_H_

#define STRBUFSIZE 1024

/**
 * Errors
 */

#define ERROK 0
#define ERRNAK 1
#define ERRUNKNOWN 2

/*********
 * Types *
 *********/

typedef unsigned int color_t;
typedef unsigned int param_t;

/**
 * Connection object
 */
struct ulcd_t {
    int fd;
    char device[STRBUFSIZE];
    unsigned long baudrate;
    int error;
    char err[STRBUFSIZE];
};

struct point_t {
    unsigned int x;
    unsigned int y;
};

struct touch_event_t {
    param_t status;
    struct point_t point;
};

struct baudtable_t {
    unsigned int index;
    unsigned long baudrate;
};


/*************
 * Functions *
 *************/

/* util.c */
struct ulcd_t * ulcd_new(void);
void ulcd_free(struct ulcd_t *ulcd);
int ulcd_open_serial_device(struct ulcd_t *ulcd);
void ulcd_set_serial_parameters(struct ulcd_t *ulcd);
int ulcd_error(struct ulcd_t *ulcd, int error, const char *err, ...);

/* text.c */
int ulcd_move_cursor(struct ulcd_t *ulcd, param_t line, param_t column);

/* touch.c */
int ulcd_touch_set_detect_region(struct ulcd_t *ulcd, struct point_t *p1, struct point_t *p2);
int ulcd_touch_set(struct ulcd_t *ulcd, param_t type);
int ulcd_touch_get(struct ulcd_t *ulcd, param_t type, param_t *status);
int ulcd_touch_get_event(struct ulcd_t *ulcd, struct touch_event_t *ev);

/* display.c */
int ulcd_gfx_cls(struct ulcd_t *ulcd);
int ulcd_gfx_filled_circle(struct ulcd_t *ulcd, struct point_t *point, param_t radius, param_t color);
int ulcd_gfx_polygon(struct ulcd_t *ulcd, color_t color, param_t points, ...);
int ulcd_gfx_contrast(struct ulcd_t *ulcd, param_t contrast);
int ulcd_display_on(struct ulcd_t *ulcd);
int ulcd_display_off(struct ulcd_t *ulcd);

/* image.c */
int ulcd_image_bitblt(struct ulcd_t *ulcd, struct point_t *point, param_t width, param_t height, const char *buffer);


/***********************
 * Serial API commands *
 ***********************/


/*
#######################################
###  5.1: Text and String Commands  ###
#######################################
*/

#define MOVE_CURSOR 0xffe9
#define PUT_CH 0xfffe
#define PUT_STR 0x0018
#define CHAR_WIDTH 0x001e
#define CHAR_HEIGHT 0x001d
#define TEXT_FGCOLOUR 0xffe7
#define TEXT_BGCOLOUR 0xffe6
#define TXT_FONT_ID 0xffe5
#define TXT_WIDTH 0xffe4
#define TXT_HEIGHT 0xffe3
#define TXT_X_GAP 0xffe2
#define TXT_Y_GAP 0xffe1
#define TXT_BOLD 0xffde
#define TXT_INVERSE 0xffdc
#define TXT_ITALIC 0xffdd
#define TXT_OPACITY 0xffdf
#define TXT_UNDERLINE 0xffdb
#define TXT_ATTRIBUTES 0xffda

/* Text and string attributes */

#define TXT_ATTRIBUTE_BOLD (1 << 4)
#define TXT_ATTRIBUTE_ITALIC (1 << 5)
#define TXT_ATTRIBUTE_INVERSE (1 << 6)
#define TXT_ATTRIBUTE_UNDERLINED (1 << 7)

/*
################################
###  5.2: Graphics Commands  ###
################################
*/

#define CLEAR_SCREEN 0xffcd
#define CHANGE_COLOUR 0xffb4
#define CIRCLE 0xffc3
#define CIRCLE_FILLED 0xffc2
#define LINE 0xffc8
#define RECTANGLE 0xffc5
#define RECTANGLE_FILLED 0xffc4
#define POLYLINE 0x0015
#define POLYGON 0x0013
#define POLYGON_FILLED 0x0014
#define TRIANGLE 0xffbf
#define TRIANGLE_FILLED 0xffa9
#define ORBIT 0x0012
#define PUT_PIXEL 0xffc1
#define GET_PIXEL 0xffc0
#define MOVE_TO 0xffcc
#define LINE_TO 0xffca
#define CLIPPING 0xffa2
#define CLIP_WINDOW 0xffb5
#define SET_CLIP_REGION 0xffb3
#define ELLIPSE 0xffb2
#define ELLIPSE_FILLED 0xffb1
#define BUTTON 0x0011
#define PANEL 0xffaf
#define SLIDER 0xffae
#define SCREEN_COPY_PASTE 0xffad
#define BEVEL_SHADOW 0xff98
#define BEVEL_WIDTH 0xff99
#define BACKGROUND_COLOUR 0xffa4
#define OUTLINE_COLOUR 0xff9d
#define CONTRAST 0xff9c
#define FRAME_DELAY 0xff9f
#define LINE_PATTERN 0xff9b
#define SCREEN_MODE 0xff9e
#define TRANSPARENCY 0xffa0
#define TRANSPARENT_COLOUR 0xffa1
#define GFX_SET 0xffce
#define GFX_GET 0xffa6

#define BUTTON_STATE_DEPRESSED 0
#define BUTTON_STATE_RAISED 1
#define PANEL_STATE_RECESSED 0
#define PANEL_STATE_RAISED 1
#define SLIDER_MODE_INDENTED 0
#define SLIDER_MODE_RAISED 1
#define SLIDER_MODE_HIDDEN 2
#define CONTRAST_OFF 0
#define CONTRAST_MIN 1
#define CONTRAST_MAX 15
#define SCREEN_MODE_LANDSCAPE 0
#define SCREEN_MODE_LANDSCAPE_REVERSE 1
#define SCREEN_MODE_PORTRAIT 2
#define SCREEN_MODE_PORTRAIT_REVERSE 3
#define GFX_SET_OBJECT_COLOUR 18
#define GFX_SET_PAGE_DISPLAY 33
#define GFX_SET_PAGE_READ 34
#define GFX_SET_PAGE_WRITE 35
#define GFX_GET_X_MAX 0
#define GFX_GET_Y_MAX 1
#define GFX_GET_OBJECT_LEFT 2
#define GFX_GET_OBJECT_TOP 3
#define GFX_GET_OBJECT_RIGHT 4
#define GFX_GET_OBJECT_BOTTOM 5

/*
####################################################
###  5.4: Serial (UART) Communications Commands  ###
####################################################
*/

#define SET_BAUD_RATE 0x0026

#define BAUD_INDEX_MIN 0
#define BAUD_INDEX_MAX 19

/*
#############################
###  5.5: Timer Commands  ###
#############################
*/

#define SLEEP 0xff3b

/*
####################################
###  5.8: Touch Screen Commands  ###
####################################
*/

#define TOUCH_DETECT_REGION 0xff39
#define TOUCH_SET 0xff38
#define TOUCH_GET 0xff37

#define TOUCH_SET_MODE_INIT 0
#define TOUCH_SET_MODE_DISABLE 1
#define TOUCH_SET_MODE_RESET 2
#define TOUCH_GET_MODE_STATUS 0
#define TOUCH_GET_MODE_GET_X 1
#define TOUCH_GET_MODE_GET_Y 2
#define TOUCH_STATUS_INVALID 0
#define TOUCH_STATUS_NOTOUCH 0
#define TOUCH_STATUS_PRESS 1
#define TOUCH_STATUS_RELEASE 2
#define TOUCH_STATUS_MOVING 3

/*
####################################
###  5.9: Image Control Commands ###
####################################
*/

#define BLIT_COM_TO_DISPLAY 0x0023

/*
###############################
###  5.10: System Commands  ###
###############################
*/

#define GET_DISPLAY_MODEL 0x001a



/************************
 * Serial API responses *
 ************************/

#define ACK 0x06
#define NAK 0x15


#endif /* #ifndef _ULCD43_H_ */
