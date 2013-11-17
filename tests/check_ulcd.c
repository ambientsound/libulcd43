#include <stdlib.h>
#include <check.h>
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


/**
 * Gfx test case
 */

START_TEST (test_gfx_contrast)
{
    param_t i;
    for (i = 0; i < 16; i++) {
        ck_assert(0 == ulcd_gfx_contrast(ulcd, i));
    }
}
END_TEST

START_TEST (test_display_on_off)
{
    ck_assert(0 == ulcd_display_off(ulcd));
    ck_assert(0 == ulcd_display_on(ulcd));
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
    ck_assert(0 == ulcd_touch_set_detect_region(ulcd, &p1, &p2));
}
END_TEST

START_TEST (test_touch_set)
{
    param_t status;
    ck_assert(0 == ulcd_touch_init(ulcd));
    ck_assert(0 == ulcd_touch_reset(ulcd));
    ck_assert(0 == ulcd_touch_disable(ulcd));
}
END_TEST

START_TEST (test_touch_get)
{
    param_t status;
    ulcd_touch_get(ulcd, TOUCH_GET_MODE_STATUS, &status);
    ck_assert(status == TOUCH_STATUS_NOTOUCH);
}
END_TEST

START_TEST (test_touch_get_event)
{
    struct touch_event_t ev;
    ev.point.x = 0;
    ev.point.y = 0;
    ulcd_touch_get_event(ulcd, &ev);
    ck_assert(ev.status == TOUCH_STATUS_NOTOUCH);
    ck_assert(ev.point.x == 0);
    ck_assert(ev.point.y == 0);
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
    suite_add_tcase(s, tc_util);

    /* Gfx test case */
    TCase *tc_gfx = tcase_create("gfx");
    tcase_add_unchecked_fixture(tc_gfx, setup, teardown);
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
