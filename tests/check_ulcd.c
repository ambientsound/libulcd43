#include <stdlib.h>
#include <check.h>
#include "../src/util.h"
#include "../src/ulcd43.h"

struct ulcd_t *ulcd;

void
setup(void)
{
    ulcd = ulcd_new();
    ulcd_set_baud_rate(ulcd, 115200);
    strcpy(ulcd->device, "/dev/ttyAMA0");

    if (ulcd_open_serial_device(ulcd)) {
        ck_abort_msg("Could not open serial port.");
    }
    ulcd_set_serial_parameters(ulcd);
}

void
teardown(void)
{
    ulcd_free(ulcd);
}


/**
 * Util test case
 */

START_TEST (test_error)
{
    ulcd_error(ulcd, 123, "H%dll%d Wo%sd!", 3, 0, "rl");
    ck_assert_int_eq(123, ulcd->error);
    ck_assert_str_eq(ulcd->err, "H3ll0 World!");
}
END_TEST

START_TEST (test_make_polygon)
{
    struct polygon_t *poly;
    struct point_t p1, p2, p3;
    struct point_t *t1, *t2, *t3;

    p1.x = 100; p1.y = 100;
    p2.x = 200; p2.y = 250;
    p3.x = 150; p3.y = 50;

    poly = ulcd_make_polygon(3, &p1, &p2, &p3);

    t1 = poly->points[0];
    t2 = poly->points[1];
    t3 = poly->points[2];

    ck_assert_int_eq(3, poly->num);
    ck_assert_int_eq(t1->x, p1.x);
    ck_assert_int_eq(t1->y, p1.y);
    ck_assert_int_eq(t2->x, p2.x);
    ck_assert_int_eq(t2->y, p2.y);
    ck_assert_int_eq(t3->x, p3.x);
    ck_assert_int_eq(t3->y, p3.y);

    ulcd_free_polygon(poly);
}
END_TEST

START_TEST (test_pack_polygon)
{
    /*               num         p1.x        p2.x        p3.x        p1.y        p2.y        p3.y           */
    char chk[14] = { 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03, 0x00, 0x01, 0x00, 0x02, 0x00, 0x03 };
    char buffer[14];
    struct polygon_t *poly;
    struct point_t p1, p2, p3;

    p1.x = 1; p1.y = 1;
    p2.x = 2; p2.y = 2;
    p3.x = 3; p3.y = 3;

    poly = ulcd_make_polygon(3, &p1, &p2, &p3);

    ck_assert_int_eq(14, pack_polygon(buffer, poly));
    ck_assert_int_eq(0, memcmp(buffer, chk, 14));

    ulcd_free_polygon(poly);
}
END_TEST


/**
 * Gfx test case
 */

START_TEST (test_gfx_cls)
{
    ck_assert_int_eq(0, ulcd_gfx_cls(ulcd));
}
END_TEST

START_TEST (test_gfx_circle)
{
    struct point_t p1;
    p1.x = 100; p1.y = 100;
    ck_assert_int_eq(0, ulcd_gfx_circle(ulcd, &p1, 50, 0xffff));
}
END_TEST

START_TEST (test_gfx_filled_circle)
{
    struct point_t p1;
    p1.x = 100; p1.y = 100;
    ck_assert_int_eq(0, ulcd_gfx_filled_circle(ulcd, &p1, 50, 0xffff));
}
END_TEST

START_TEST (test_gfx_rectangle)
{
    struct point_t p1, p2;
    p1.x = 100; p1.y = 100;
    p2.x = 200; p2.y = 250;
    ck_assert_int_eq(0, ulcd_gfx_rectangle(ulcd, &p1, &p2, 0xffff));
}
END_TEST

START_TEST (test_gfx_filled_rectangle)
{
    struct point_t p1, p2;
    p1.x = 100; p1.y = 100;
    p2.x = 200; p2.y = 250;
    ck_assert_int_eq(0, ulcd_gfx_filled_rectangle(ulcd, &p1, &p2, 0xffff));
}
END_TEST

START_TEST (test_gfx_polygon)
{
    struct polygon_t *poly;
    struct point_t p1, p2, p3;

    p1.x = 100; p1.y = 100;
    p2.x = 200; p2.y = 250;
    p3.x = 150; p3.y = 50;

    poly = ulcd_make_polygon(3, &p1, &p2, &p3);

    ck_assert_int_eq(0, ulcd_gfx_polygon(ulcd, poly, 0xffff));

    ulcd_free_polygon(poly);
}
END_TEST

START_TEST (test_gfx_filled_polygon)
{
    struct polygon_t *poly;
    struct point_t p1, p2, p3;

    p1.x = 100; p1.y = 100;
    p2.x = 200; p2.y = 250;
    p3.x = 150; p3.y = 50;

    poly = ulcd_make_polygon(3, &p1, &p2, &p3);

    ck_assert_int_eq(0, ulcd_gfx_filled_polygon(ulcd, poly, 0xffff));

    ulcd_free_polygon(poly);
}
END_TEST

START_TEST (test_gfx_contrast)
{
    param_t i;
    for (i = 0; i < 16; i++) {
        ck_assert_int_eq(0, ulcd_gfx_contrast(ulcd, i));
    }
}
END_TEST

START_TEST (test_display_on_off)
{
    ck_assert_int_eq(0, ulcd_display_off(ulcd));
    ck_assert_int_eq(0, ulcd_display_on(ulcd));
}
END_TEST


/**
 * Serial test case
 */

START_TEST (test_set_baud_rate)
{
    ck_assert_int_eq(0, ulcd_set_baud_rate(ulcd, ulcd->baud_rate));
    ck_assert_int_eq(ERRBAUDRATE, ulcd_set_baud_rate(ulcd, 1337));
}
END_TEST


/**
 * Touch test case
 */

START_TEST (test_touch_set_detect_region)
{
    struct point_t p1 = { 0, 0 };
    struct point_t p2 = { 479, 271 };
    ck_assert_int_eq(0, ulcd_touch_set_detect_region(ulcd, &p1, &p2));
}
END_TEST

START_TEST (test_touch_set)
{
    param_t status;
    ck_assert_int_eq(0, ulcd_touch_init(ulcd));
    ck_assert_int_eq(0, ulcd_touch_reset(ulcd));
    ck_assert_int_eq(0, ulcd_touch_disable(ulcd));
}
END_TEST

START_TEST (test_touch_get)
{
    param_t status;
    ulcd_touch_get(ulcd, TOUCH_GET_MODE_STATUS, &status);
    ck_assert_int_eq(status, TOUCH_STATUS_NOTOUCH);
}
END_TEST

START_TEST (test_touch_get_event)
{
    struct touch_event_t ev;
    ev.point.x = 0;
    ev.point.y = 0;
    ulcd_touch_get_event(ulcd, &ev);
    ck_assert_int_eq(ev.status, TOUCH_STATUS_NOTOUCH);
    ck_assert_int_eq(ev.point.x, 0);
    ck_assert_int_eq(ev.point.y, 0);
}
END_TEST


/**
 * Text test case
 */

START_TEST (test_move_cursor)
{
    ck_assert_int_eq(0, ulcd_move_cursor(ulcd, 0, 0));
}
END_TEST

START_TEST (test_txt_putstr)
{
    const char *str = "All your base are belong to us";
    param_t slen;
    ck_assert_int_eq(0, ulcd_txt_putstr(ulcd, str, &slen));
    ck_assert_int_eq(slen, strlen(str));
}
END_TEST

START_TEST (test_txt_charwidth)
{
    param_t p;
    ck_assert_int_eq(0, ulcd_txt_charwidth(ulcd, 'a', &p));
    ck_assert_int_eq(7, p);
}
END_TEST

START_TEST (test_txt_charheight)
{
    param_t p;
    ck_assert_int_eq(0, ulcd_txt_charheight(ulcd, 'e', &p));
    ck_assert_int_eq(8, p);
}
END_TEST

/**
 * Image Control test case
 */

/*
START_TEST (test_image_bitblt)
{
    #include "image.c"
    struct point_t p;
    p.x = 0;
    p.y = 0;
    ulcd_image_bitblt(ulcd, &p, 480, 272, image);
}
END_TEST
*/

Suite *
ulcd_suite(void)
{
    Suite *s = suite_create("uLCD43");

    /* Util test case */
    TCase *tc_util = tcase_create("util");
    tcase_add_unchecked_fixture(tc_util, setup, teardown);
    tcase_add_test(tc_util, test_error);
    tcase_add_test(tc_util, test_make_polygon);
    tcase_add_test(tc_util, test_pack_polygon);
    suite_add_tcase(s, tc_util);

    /* Gfx test case */
    TCase *tc_gfx = tcase_create("gfx");
    tcase_add_unchecked_fixture(tc_gfx, setup, teardown);
    tcase_add_test(tc_gfx, test_gfx_cls);
    tcase_add_test(tc_gfx, test_gfx_circle);
    tcase_add_test(tc_gfx, test_gfx_filled_circle);
    tcase_add_test(tc_gfx, test_gfx_rectangle);
    tcase_add_test(tc_gfx, test_gfx_filled_rectangle);
    tcase_add_test(tc_gfx, test_gfx_polygon);
    tcase_add_test(tc_gfx, test_gfx_filled_polygon);
    tcase_add_test(tc_gfx, test_gfx_contrast);
    tcase_add_test(tc_gfx, test_display_on_off);
    suite_add_tcase(s, tc_gfx);

    /* Serial test case */
    TCase *tc_serial = tcase_create("serial");
    tcase_add_unchecked_fixture(tc_serial, setup, teardown);
    tcase_add_test(tc_serial, test_set_baud_rate);
    suite_add_tcase(s, tc_serial);

    /* Touch test case */
    TCase *tc_touch = tcase_create("touch");
    tcase_add_unchecked_fixture(tc_touch, setup, teardown);
    tcase_add_test(tc_touch, test_touch_set_detect_region);
    tcase_add_test(tc_touch, test_touch_set);
    tcase_add_test(tc_touch, test_touch_get);
    tcase_add_test(tc_touch, test_touch_get_event);
    suite_add_tcase(s, tc_touch);

    /* Text test case */
    TCase *tc_text = tcase_create("text");
    tcase_add_unchecked_fixture(tc_text, setup, teardown);
    tcase_add_test(tc_text, test_move_cursor);
    tcase_add_test(tc_text, test_txt_putstr);
    tcase_add_test(tc_text, test_txt_charwidth);
    tcase_add_test(tc_text, test_txt_charheight);
    suite_add_tcase(s, tc_text);

    /* Image test case */
    /*
    TCase *tc_image = tcase_create("Image");
    tcase_add_unchecked_fixture(tc_image, setup, teardown);
    tcase_add_test(tc_image, test_image_bitblt);
    tcase_set_timeout(tc_image, 60);
    suite_add_tcase(s, tc_image);
    */

    return s;
}

int
main(void)
{
    int number_failed;
    Suite *s = ulcd_suite();
    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
