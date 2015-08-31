#include <tizen.h>
#include "wsi.h"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *web;
	Evas_Object *btn_back;
	Evas_Object *btn_fwd;
	Evas_Object *url_entry;
	Evas_Object *bx;
	Evas_Object *hoversel;
	Eina_List *sub_wins;
	Eina_Bool js_hooks : 1;
} appdata_s;

static void
_btn_back_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *web = data;

	elm_web_back(web);
}

static void
_btn_fwd_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *web = data;

	elm_web_forward(web);
}

static void
_btn_reload_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *web = data;

	elm_web_reload(web);
}

static void
_url_entry_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
	Evas_Object *web = data;
	const char *url = elm_object_text_get(obj);

	elm_web_uri_set(web, url);
}

static void
_title_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
	char buf[512];
	snprintf(buf, sizeof(buf), "Web - %s", (const char *)event_info);
	elm_win_title_set(data, buf);
}

static void
_url_changed_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
	appdata_s *wt = data;

	elm_object_text_set(wt->url_entry, event_info);

	elm_object_disabled_set(wt->btn_back, !elm_web_back_possible_get(wt->web));
	elm_object_disabled_set(wt->btn_fwd, !elm_web_forward_possible_get(wt->web));
}

static void
_new_win_del_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
	appdata_s *wt = data;
	wt->sub_wins = eina_list_remove(wt->sub_wins, obj);
}

static void
_web_win_close_request_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	evas_object_del(data);
}

static void
_main_web_del_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	appdata_s *wt = data;
	Evas_Object *sub_win;

	EINA_LIST_FREE(wt->sub_wins, sub_win)
	evas_object_del(sub_win);

	free(wt);
}


static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}

static void
create_base_gui(appdata_s *ad)
{
	/* Window */
	Evas_Object *win, *bx, *bx2, *bt, *web, *url;
	elm_need_web();


	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	win = ad->win;
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(ad->win, EEXT_CALLBACK_BACK, win_back_cb, ad);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	bx = elm_box_add(win);
	evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, bx);
	evas_object_show(bx);

	bx2 = elm_box_add(win);
	elm_box_horizontal_set(bx2, EINA_TRUE);
	evas_object_size_hint_weight_set(bx2, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(bx2, EVAS_HINT_FILL, 0.0);
	elm_box_pack_end(bx, bx2);
	evas_object_show(bx2);

	web = elm_web_add(win);
	evas_object_size_hint_weight_set(web, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(web, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(bx, web);
	evas_object_show(web);

	bt = elm_button_add(win);
	elm_object_text_set(bt, "<");
	elm_box_pack_end(bx2, bt);
	evas_object_show(bt);

	evas_object_smart_callback_add(bt, "clicked", _btn_back_cb, web);
	ad->btn_back = bt;

	bt = elm_button_add(win);
	elm_object_text_set(bt, "R");
	elm_box_pack_end(bx2, bt);
	evas_object_show(bt);

	evas_object_smart_callback_add(bt, "clicked", _btn_reload_cb, web);

	bt = elm_button_add(win);
	elm_object_text_set(bt, ">");
	elm_box_pack_end(bx2, bt);
	evas_object_show(bt);

	evas_object_smart_callback_add(bt, "clicked", _btn_fwd_cb, web);
	ad->btn_fwd = bt;

	url = elm_entry_add(win);
	elm_entry_single_line_set(url, EINA_TRUE);
	elm_entry_scrollable_set(url, EINA_TRUE);
	evas_object_size_hint_weight_set(url, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(url, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(bx2, url);
	evas_object_show(url);

	evas_object_smart_callback_add(url, "activated", _url_entry_changed_cb, web);
	ad->url_entry = url;
	evas_object_smart_callback_add(web, "title,changed", _title_changed_cb, win);
	evas_object_smart_callback_add(web, "url,changed", _url_changed_cb, ad);
	evas_object_event_callback_add(web, EVAS_CALLBACK_DEL, _main_web_del_cb, ad);
	ad->web = web;
	elm_web_uri_set(web, "http://www.iotivity.org");
	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
}

static void
app_pause(void *data)
{
}

static void
app_resume(void *data)
{
}

static void
app_terminate(void *data)
{
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
