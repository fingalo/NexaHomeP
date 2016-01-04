#include <pebble.h>
/*
	#undef APP_LOG

	#define APP_LOG(...)
*/
	
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
	// Persistent data
#define S_SENSOR_FLAG_PKEY 1
#define S_EVENT_FLAG_PKEY 2
#define S_SHOWEVENT_FLAG_PKEY 3
#define S_SHOWTIMESTAMP_FLAG_PKEY 4
#define S_SENSOR_FORMAT_FLAG_PKEY 5
	
	// Device methods
#define TELLSTICK_TURNON	(1)
#define TELLSTICK_TURNOFF	(2)
#define TELLSTICK_BELL		(4)
#define TELLSTICK_TOGGLE	(8)
#define TELLSTICK_DIM			(16)
#define TELLSTICK_LEARN		(32)
#define TELLSTICK_EXECUTE	(64)
#define TELLSTICK_UP			(128)
#define TELLSTICK_DOWN		(256)
#define TELLSTICK_STOP		(512)

enum {
  AKEY_MODULE,
  AKEY_ACTION,
  AKEY_NAME,
  AKEY_ID,
  AKEY_STATE,
  AKEY_TEMP,
  AKEY_HUM,
  AKEY_METHODS,
  AKEY_DIMVALUE,
  AKEY_TYPE,
  AKEY_TIMESTAMP,
};

static Window *window;
static TextLayer *textLayer;
static MenuLayer *menuLayer;
AppTimer *timer;
static GBitmap *TelldusOn, *TelldusOff, *NexaHome;
static GBitmap *TelldusOnI, *TelldusOffI, *NexaHomeI;
static GBitmap  *dim_icons[5];
static GBitmap  *dim_icons_i[5];
int num_menu_icons = 0;

#define MAX_DEVICE_LIST_ITEMS (30)
#define MAX_SENSOR_LIST_ITEMS (30)
#define MAX_STATUS_LIST_ITEMS (20)
#define MAX_EVENT_LIST_ITEMS (20)
#define MAX_DEVICE_NAME_LENGTH (16)
#define MAX_SENSOR_NAME_LENGTH (16)
#define MAX_STATUS_NAME_LENGTH (16)
#define MAX_EVENT_NAME_LENGTH (16)
#define MAX_EVENT_TIME_LENGTH (20)
#define MAX_TEMP_LENGTH (6)
#define MAX_HUM_LENGTH (4)
#define SHOW_EVENT (2)
#define SHOW_TIMESTAMP (1)

enum  { MENU_SECTION_STATUS,MENU_SECTION_ENVIRONMENT,  MENU_SECTION_EVENT, MENU_SECTION_DEVICE, MENU_SECTION_NUMBER};
	
typedef struct {
	int id;
	char name[MAX_DEVICE_NAME_LENGTH+1];
	int state;
	int methods;
	int dimvalue;
	char eventd[MAX_DEVICE_NAME_LENGTH+1];
	char timestamp[MAX_DEVICE_NAME_LENGTH+1];
} Device;

typedef struct {
	int id;
	char name[MAX_STATUS_NAME_LENGTH+1];
} Status;

typedef struct {
	int id;
	char name[MAX_SENSOR_NAME_LENGTH+1];
	char temp[MAX_TEMP_LENGTH+1];
	char hum[MAX_HUM_LENGTH+1];
	char timestamp[MAX_DEVICE_NAME_LENGTH+1];
} Sensor;

typedef struct {
	char name[MAX_EVENT_NAME_LENGTH+1];
	char time[MAX_EVENT_TIME_LENGTH+1];
} Event;

static Device s_device_list_items[MAX_DEVICE_LIST_ITEMS];
static Status s_status_list_items[MAX_STATUS_LIST_ITEMS];
static Sensor s_sensor_list_items[MAX_SENSOR_LIST_ITEMS];
static Event s_event_list_items[MAX_EVENT_LIST_ITEMS];
static int s_device_count = 0;
static int s_status_current = 0;
static int s_status_count = 0;
static int s_status_show_count = 0;
static int s_sensor_count = 0;
static bool s_sensor_flag = false;
static bool s_sensor_format_flag = false;
static int s_sensor_show_count = 0;

static int s_event_count = 0;
static bool s_event_flag = false;
static int s_event_show_count = 0;
static int s_stamp_flag = 0;
static bool s_stampenv_flag = false;

void timer_callback(void *data) {
			s_stampenv_flag = 0;//! s_stampenv_flag;
			layer_mark_dirty(menu_layer_get_layer(menuLayer));
			timer = NULL;
   //Register next execution
}

void out_sent_handler(DictionaryIterator *sent, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "outgoing message was delivered");
}

void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "outgoing message failed");
}

void setEvent(char *name, char  *time) {
	if (s_event_count >= MAX_EVENT_LIST_ITEMS) {
		return;
	}
 	strncpy(s_event_list_items[s_event_count].name, name, MAX_EVENT_NAME_LENGTH);
 	strncpy(s_event_list_items[s_event_count].time, time, MAX_EVENT_TIME_LENGTH);
	s_event_count++;

	if (s_event_flag == true){
		s_event_show_count  = s_event_count;
	} else {
		s_event_show_count = 1;
	}	
}

void setSensor(int id, char *name, char *temp, char *hum, char* timestamp) {
	if (s_sensor_count >= MAX_SENSOR_LIST_ITEMS) {
		return;
	}
	s_sensor_list_items[s_sensor_count].id = id;
 	strncpy(s_sensor_list_items[s_sensor_count].name, name, MAX_SENSOR_NAME_LENGTH);
 	strncpy(s_sensor_list_items[s_sensor_count].temp, temp, MAX_TEMP_LENGTH);
 	strncpy(s_sensor_list_items[s_sensor_count].hum, hum, MAX_HUM_LENGTH);
 	strncpy(s_sensor_list_items[s_sensor_count].timestamp, timestamp, MAX_DEVICE_NAME_LENGTH);
	s_sensor_count++;
	
	if (s_sensor_flag == true){
		s_sensor_show_count  = s_sensor_count;
	} else {
		s_sensor_show_count = 1;
	}	
}

void setDevice(int id, char *name, int state, int methods, int dimvalue, char *eventd, char* timestamp) {
	if (s_device_count >= MAX_DEVICE_LIST_ITEMS) {
		return;
	}
	s_device_list_items[s_device_count].id = id;
	s_device_list_items[s_device_count].state = state;
	s_device_list_items[s_device_count].methods = methods;
	s_device_list_items[s_device_count].dimvalue = dimvalue;
 	strncpy(s_device_list_items[s_device_count].name, name, MAX_DEVICE_NAME_LENGTH);
 	strncpy(s_device_list_items[s_device_count].eventd, eventd, MAX_DEVICE_NAME_LENGTH);
 	strncpy(s_device_list_items[s_device_count].timestamp, timestamp, MAX_DEVICE_NAME_LENGTH);
	s_device_count++;
}

void setStatus(int id, char *name) {
	if (s_status_count >= MAX_DEVICE_LIST_ITEMS) {
		return;
	}
	s_status_list_items[s_status_count].id = id;
 	strncpy(s_status_list_items[s_status_count].name, name, MAX_DEVICE_NAME_LENGTH);
	s_status_count++;
	if(s_status_count > 0) s_status_show_count = 1;
}

void in_received_handler(DictionaryIterator *received, void *context) {
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message received");
	// Check for fields you expect to receive
	Tuple *moduleTuple = dict_find(received, AKEY_MODULE);
	if (!moduleTuple ) {
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Malformed message");
		return;
	}
	
	if (strcmp(moduleTuple->value->cstring, "done") == 0) {
//	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message DONE received");
		
		layer_set_hidden(text_layer_get_layer(textLayer), true);
		layer_set_hidden(menu_layer_get_layer(menuLayer), false);
    MenuIndex index;
    index.section = 0;
    index.row = 0;
    menu_layer_set_selected_index( menuLayer, index, MenuRowAlignNone, false );  	
		menu_layer_reload_data(menuLayer);
		
	} else if (strcmp(moduleTuple->value->cstring, "clear") == 0) {
		
		s_device_count = 0;
		s_sensor_count = 0;
		text_layer_set_text(textLayer, "Logged in...");
		layer_set_hidden(text_layer_get_layer(textLayer), false);
		layer_set_hidden(menu_layer_get_layer(menuLayer), true);
		
	}  else if (strcmp(moduleTuple->value->cstring, "event") == 0) {
		
		Tuple *nameTuple = dict_find(received, AKEY_NAME);
		Tuple *tempTuple = dict_find(received, AKEY_TEMP);

		if (!tempTuple || !nameTuple ){
			return;
		} 
		setEvent(nameTuple->value->cstring,tempTuple->value->cstring );

	}  else if (strcmp(moduleTuple->value->cstring, "status") == 0) {
		
		Tuple *idTuple = dict_find(received, AKEY_ID);
		Tuple *nameTuple = dict_find(received, AKEY_NAME);
		Tuple *stateTuple = dict_find(received, AKEY_STATE);

		if (!idTuple || !nameTuple || !stateTuple){
			return;
		} 
		setStatus(idTuple->value->int8, nameTuple->value->cstring);
		s_status_current = stateTuple->value->int8;
		
	}  else if (strcmp(moduleTuple->value->cstring, "device") == 0) {
		
		Tuple *idTuple = dict_find(received, AKEY_ID);
		Tuple *nameTuple = dict_find(received, AKEY_NAME);
		Tuple *stateTuple = dict_find(received, AKEY_STATE);
		Tuple *methodsTuple = dict_find(received, AKEY_METHODS);
		Tuple *dimvalueTuple = dict_find(received, AKEY_DIMVALUE);
		Tuple *tempTuple = dict_find(received, AKEY_TEMP);
		Tuple *timestampTuple = dict_find(received, AKEY_TIMESTAMP);

		if (!idTuple || !nameTuple || !stateTuple || !methodsTuple || !dimvalueTuple || !tempTuple || !timestampTuple ) {
			return;
		} 
    for(int i=0; i<s_device_count; i++) { 
      if(s_device_list_items[i].id == idTuple->value->int8) {  
				s_device_list_items[i].state = stateTuple->value->int8;
				s_device_list_items[i].dimvalue = dimvalueTuple->value->int16;
        layer_mark_dirty(menu_layer_get_layer(menuLayer));
        return;
        }
    }
		setDevice(idTuple->value->int8, 
							nameTuple->value->cstring, 
							stateTuple->value->int8, 
							methodsTuple->value->int16, 
							dimvalueTuple->value->int16,
							tempTuple->value->cstring,
							timestampTuple->value->cstring
						 );
		
	} else if (strcmp(moduleTuple->value->cstring, "sensor") == 0) {
		
		Tuple *idTuple = dict_find(received, AKEY_ID);
		Tuple *nameTuple = dict_find(received, AKEY_NAME);
		Tuple *tempTuple = dict_find(received, AKEY_TEMP);
		Tuple *timestampTuple = dict_find(received, AKEY_TIMESTAMP);
		Tuple *humidityTuple = dict_find(received, AKEY_HUM);

		if (!idTuple || !nameTuple || !tempTuple || !tempTuple || !timestampTuple || !humidityTuple) {
			return;
		} 
    for(int i=0; i<s_sensor_count; i++) { 
      if(s_sensor_list_items[i].id == idTuple->value->int8) {  
        return;
      }
    }
	  setSensor(idTuple->value->int8, 
							nameTuple->value->cstring, 
							tempTuple->value->cstring,
							humidityTuple->value->cstring,
							timestampTuple->value->cstring);
		//menu_layer_reload_data(menuLayer);
	}

}

void in_dropped_handler(AppMessageResult reason, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incoming message dropped");
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	
#if defined(PBL_ROUND)
	if( !menu_layer_is_index_selected(menuLayer, cell_index))
		return 44;
	else
#endif
	return PBL_IF_ROUND_ELSE(60, 44);
}

static uint16_t menu_get_num_sections_callback(MenuLayer *menu_layer, void *data) {
  return MENU_SECTION_NUMBER;
}

// Each section has a number of items;  we use a callback to specify this
// You can also dynamically add and remove items using this
static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {

	switch (section_index) {

		case MENU_SECTION_STATUS:
      return s_status_show_count;
 
		case MENU_SECTION_ENVIRONMENT:
      return s_sensor_show_count;

		case MENU_SECTION_DEVICE:
      return s_device_count;
		
		case MENU_SECTION_EVENT:
      return s_event_show_count;

		default:
      return 0;
  }
}

static void draw_row_callback(GContext* ctx, Layer *cell_layer, MenuIndex *cell_index, void *data) {
  GBitmap *Img_status;
	
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	
	if (cell_index->section == MENU_SECTION_ENVIRONMENT){
		
    Sensor* sensor = &s_sensor_list_items[cell_index->row];
    char buff1[30] = "";
    char p1 = ' ';
    if (sensor->hum[0] != '\0') {
      p1 = '%';
    }  

		if (strlen(sensor->temp) > 2) {
			if (s_sensor_format_flag) {
				snprintf (buff1,30,"        %s%c%cC   %s%c",sensor->temp,0xc2,0xb0,sensor->hum,p1);
			} 
			else {
				snprintf (buff1,30,"%s%c%cC   %s%c",sensor->temp,0xc2,0xb0,sensor->hum,p1);
			}
		}
		else {			
			p1 = '%';
			if (s_sensor_format_flag) 
				snprintf (buff1,30,"        %s%c",sensor->temp,p1);
			else
				snprintf (buff1,30,"%s%c",sensor->temp,p1);
		}

		char buff2[30] = "";
		if (s_stampenv_flag)		
			snprintf (buff2,19,"%s",sensor->timestamp);
		else
			snprintf (buff2,19,"%s",sensor->name);

		if (!s_sensor_format_flag) {
			if (menu_cell_layer_is_highlighted( cell_layer)) {
				graphics_context_set_text_color(ctx, GColorWhite);
				graphics_context_set_stroke_color(ctx, GColorWhite);
			}
			else {
				graphics_context_set_text_color(ctx, GColorBlack);				
				graphics_context_set_stroke_color(ctx, GColorBlack);
			}

			#if defined(PBL_ROUND)
			if ( menu_layer_is_index_selected(menuLayer, cell_index) ){ 
			#endif
				menu_cell_basic_draw(ctx, cell_layer, buff2,  " " , NULL);
				GRect bounds = layer_get_bounds(cell_layer);
				graphics_draw_text(ctx, buff1, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect( bounds.origin.x, bounds.origin.y + bounds.size.h/2-5, bounds.size.w, bounds.size.h/2), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
			#if defined(PBL_ROUND)
			} else {
				menu_cell_basic_draw(ctx, cell_layer, buff2, NULL,  NULL);			
			}
			#endif			
			}
		else {
			menu_cell_basic_draw(ctx, cell_layer, buff1, buff2,  NULL);
		}
		
	} else 	if (cell_index->section == MENU_SECTION_DEVICE){
		
    Device* device;		
    	device = &s_device_list_items[cell_index->row];

    if (device->methods & TELLSTICK_DIM) {
     if (device->state == TELLSTICK_TURNOFF) {
				if (menu_cell_layer_is_highlighted( cell_layer)) {
					Img_status = dim_icons_i[0]; 
				} else {
					Img_status = dim_icons[0]; 				
				}
      } 
      else if (device->state == TELLSTICK_TURNON) {
				if (menu_cell_layer_is_highlighted( cell_layer)) {
	        Img_status = dim_icons_i[(device->dimvalue + 24)/25];    
				} else {
				 	Img_status = dim_icons[(device->dimvalue + 24)/25];    
				}
      } else {
				if (menu_cell_layer_is_highlighted( cell_layer)) {
	        Img_status = dim_icons_i[(device->dimvalue + 24)/25];    
				} else {
				 	Img_status = dim_icons[(device->dimvalue + 24)/25];    
				}
      }			
    }
    else {			
				if (menu_cell_layer_is_highlighted( cell_layer)) {
					Img_status = TelldusOnI; 
				} else {
					Img_status = TelldusOn; 
				}
      if (device->state == TELLSTICK_TURNOFF) {
 				if (menu_cell_layer_is_highlighted( cell_layer)) {
					Img_status = TelldusOffI; 
				} else {
					Img_status = TelldusOff; 
				}
			}
    }  

#if defined(PBL_COLOR)
	 if (device->state != TELLSTICK_TURNOFF) graphics_context_set_text_color(ctx, GColorYellow);				
#endif
#if defined(PBL_ROUND)
		if ( menu_layer_is_index_selected(menuLayer, cell_index) ) { 
#endif			
			if (s_stamp_flag == SHOW_EVENT && device->eventd[0] != '\0') 
					menu_cell_basic_draw(ctx, cell_layer,  device->name, device->eventd, Img_status);
			else if (s_stamp_flag == SHOW_TIMESTAMP) 
					menu_cell_basic_draw(ctx, cell_layer,  device->name, device->timestamp, Img_status);
			else 
				menu_cell_basic_draw(ctx, cell_layer,  device->name, NULL, Img_status);		
#if defined(PBL_ROUND)
		} else {
    	menu_cell_basic_draw(ctx, cell_layer,  device->name, NULL, NULL);					
		}	
#endif			
			
			
	} else 	if (cell_index->section == MENU_SECTION_EVENT){
		
    Event* event;		
    event = &s_event_list_items[cell_index->row];

		#if defined(PBL_ROUND)
		if ( menu_layer_is_index_selected(menuLayer, cell_index))
		#endif
    	menu_cell_basic_draw(ctx, cell_layer, event->name, event->time, NULL);
		#if defined(PBL_ROUND)
		else
    	menu_cell_basic_draw(ctx, cell_layer, event->name, NULL, NULL);
		#endif			

	} else {
		Status* status;		
		if (s_status_show_count == 1) {
			status = &s_status_list_items[s_status_current];
#if defined(PBL_ROUND)
		if ( menu_layer_is_index_selected(menuLayer, cell_index) ){ 
#endif			
			if (menu_cell_layer_is_highlighted( cell_layer)) {
	      Img_status = NexaHomeI; 
			} else {
				Img_status = NexaHome; 
			}
#if defined(PBL_ROUND)
		}  else {
			Img_status = NULL;
		}
#endif			
	
		} else {
			status = &s_status_list_items[cell_index->row];
			if (cell_index->row == s_status_current)
			if (menu_cell_layer_is_highlighted( cell_layer)) {
	      Img_status = TelldusOnI; 
			} else {
				Img_status = TelldusOn; 
			}
			else
 				if (menu_cell_layer_is_highlighted( cell_layer)) {
					Img_status = TelldusOffI; 
				} else {
					Img_status = TelldusOff; 
				}
		}
		menu_cell_basic_draw(ctx, cell_layer, status->name, NULL, Img_status);
	}
}

void set_sec_text(GContext* ctx, char * text, const Layer *cell_layer) {
	GRect bounds = layer_get_bounds(cell_layer);
	graphics_draw_text(ctx, text, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD), GRect(  bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h), GTextOverflowModeTrailingEllipsis, PBL_IF_ROUND_ELSE(GTextAlignmentCenter,GTextAlignmentLeft), NULL);
}

static void menu_draw_header_callback(GContext* ctx, const Layer *cell_layer, uint16_t section_index, void *data) {

	switch (section_index) {

		case MENU_SECTION_STATUS:
			set_sec_text(ctx, "Status", cell_layer);
      break;
 
		case MENU_SECTION_ENVIRONMENT:	
			if (s_sensor_show_count == 0)
				break;
			if (s_sensor_flag == false)
				set_sec_text(ctx, "Environment >>>>", cell_layer);
			else
				set_sec_text(ctx, "Status", cell_layer);
      break;	
 
		case MENU_SECTION_DEVICE:	
			if (s_device_count == 0)
				break;
			set_sec_text(ctx, "Devices", cell_layer);
      break;

		case MENU_SECTION_EVENT:	
			if (s_event_show_count == 0)
				break;
			if (s_event_flag == false)
				set_sec_text(ctx, "Events Queue >>>>", cell_layer);
			else
				set_sec_text(ctx, "Events Queue", cell_layer);
      break;	
  }
}

static int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
  // This is a define provided in pebble.h that you may use for the default height
	switch (section_index) {
		
		case MENU_SECTION_DEVICE:	
			if (s_device_count == 0)
				return 0;
			break;
		case MENU_SECTION_ENVIRONMENT:	
			if (s_sensor_show_count == 0)
				return 0;
			break;	
		case MENU_SECTION_EVENT:	
			if (s_event_show_count == 0)
				return 0;
			break;	
	}
	return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
  if(cell_index->section == MENU_SECTION_ENVIRONMENT) {
		if (s_sensor_flag == false){
			s_sensor_flag = true;
			s_sensor_show_count  = s_sensor_count;
		} else {
			s_sensor_flag = false;
			s_sensor_show_count = 1;
		}
		MenuIndex index;
		index.section = MENU_SECTION_ENVIRONMENT;
		index.row =  0;
		menu_layer_set_selected_index( menuLayer, index, MenuRowAlignCenter , false );  	
		menu_layer_reload_data(menuLayer);
   
	} else if (cell_index->section == MENU_SECTION_EVENT) {

		if (s_event_flag == false){
			s_event_flag = true;
			s_event_show_count  = s_event_count;
		} else {
			s_event_flag = false;
			s_event_show_count = 1;
		}
		MenuIndex index;
		index.section = MENU_SECTION_EVENT;
		index.row =  0;
		menu_layer_set_selected_index( menuLayer, index, MenuRowAlignCenter , false );  	
		menu_layer_reload_data(menuLayer);

	} else if (cell_index->section == MENU_SECTION_STATUS) {

		if (s_status_show_count == 1) {
			s_status_show_count = s_status_count;
			MenuIndex index;
			index.section = MENU_SECTION_STATUS;
			index.row =  s_status_current;
			menu_layer_set_selected_index( menuLayer, index, MenuRowAlignCenter , false );  	
			menu_layer_reload_data(menuLayer);

		}	else {

			Status* status;
			s_status_show_count = 1;
			s_status_current = cell_index->row;

			status = &s_status_list_items[cell_index->row];
			DictionaryIterator *iter;
			app_message_outbox_begin(&iter);
			Tuplet module = TupletCString(AKEY_MODULE, "status");
			dict_write_tuplet(iter, &module);
			Tuplet id = TupletInteger(AKEY_ID, status->id);
			dict_write_tuplet(iter, &id);
			app_message_outbox_send();
			MenuIndex index;
			index.section = 0;
			index.row =  0;
			menu_layer_set_selected_index( menuLayer, index, MenuRowAlignCenter , false );  	
			menu_layer_reload_data(menuLayer);
		}
		
	} else if (cell_index->section == MENU_SECTION_DEVICE) {
		
		Device* device;
		device = &s_device_list_items[cell_index->row];
		DictionaryIterator *iter;
		app_message_outbox_begin(&iter);
		Tuplet module = TupletCString(AKEY_MODULE, "device");
		dict_write_tuplet(iter, &module);
		Tuplet id = TupletInteger(AKEY_ID, device->id);
		dict_write_tuplet(iter, &id);
		Tuplet methods = TupletInteger(AKEY_METHODS, 3);
		dict_write_tuplet(iter, &methods);
		app_message_outbox_send();
	}
}

static void select_long_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
	if(cell_index->section == MENU_SECTION_DEVICE)  {
		
		Device* device = &s_device_list_items[cell_index->row];
		if (device->methods & TELLSTICK_DIM) {
			DictionaryIterator *iter;
			app_message_outbox_begin(&iter);
			Tuplet module = TupletCString(AKEY_MODULE, "device");
			dict_write_tuplet(iter, &module);
			Tuplet id = TupletInteger(AKEY_ID, device->id);
			dict_write_tuplet(iter, &id);

			int c;

			if (device->dimvalue == 100)
				c = 0;
			else {
				c = device->dimvalue / 25;
				c = c * 25 + 25;
				if (c > 100) c = 0;
			}

			device->dimvalue = c; 

			layer_mark_dirty(menu_layer_get_layer(menuLayer));
			Tuplet dimvalue = TupletInteger(AKEY_DIMVALUE, device->dimvalue);
			dict_write_tuplet(iter, &dimvalue);
			Tuplet methods = TupletInteger(AKEY_METHODS, TELLSTICK_DIM);
			dict_write_tuplet(iter, &methods);
			app_message_outbox_send();
		} else {		
			s_stamp_flag++;
			if (s_stamp_flag > 2) s_stamp_flag = 0;
			layer_mark_dirty(menu_layer_get_layer(menuLayer));
		}

	}
	else if(cell_index->section == MENU_SECTION_ENVIRONMENT) {		
		if (!s_stampenv_flag) 
			timer = app_timer_register(5000, timer_callback, NULL);
		else
			s_sensor_format_flag = !s_sensor_format_flag;
		s_stampenv_flag = true; //!s_stampenv_flag;
		layer_mark_dirty(menu_layer_get_layer(menuLayer));
	}
}

static void window_load(Window *window) {
	Layer *windowLayer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(windowLayer);
	GRect frame = layer_get_frame(windowLayer);
	TelldusOn = gbitmap_create_with_resource(RESOURCE_ID_IMG_ON);
	TelldusOff = gbitmap_create_with_resource(RESOURCE_ID_IMG_OFF2);
	TelldusOnI = gbitmap_create_with_resource(RESOURCE_ID_IMG_ONI);
	TelldusOffI = gbitmap_create_with_resource(RESOURCE_ID_IMG_OFF2I);
	NexaHome = gbitmap_create_with_resource(RESOURCE_ID_IMG_NEXA2);
	NexaHomeI = gbitmap_create_with_resource(RESOURCE_ID_IMG_NEXA2I);
    
	num_menu_icons = 0;
	dim_icons_i[num_menu_icons] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM0II);
	dim_icons[num_menu_icons++] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM0I);
  dim_icons_i[num_menu_icons] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM25II);
  dim_icons[num_menu_icons++] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM25I);
  dim_icons_i[num_menu_icons] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM50II);
  dim_icons[num_menu_icons++] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM50I);
  dim_icons_i[num_menu_icons] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM75II);
  dim_icons[num_menu_icons++] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM75I);
  dim_icons[num_menu_icons] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM100I);
  dim_icons_i[num_menu_icons] = gbitmap_create_with_resource(RESOURCE_ID_IMG_DIM100II);

  
	textLayer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 40 } });
	text_layer_set_background_color(textLayer, PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite)) ;
	text_layer_set_text_color(textLayer, PBL_IF_COLOR_ELSE(GColorWhite, GColorBlack)) ;
	text_layer_set_text(textLayer, "Loading...");
	text_layer_set_text_alignment(textLayer, GTextAlignmentCenter);
	text_layer_set_overflow_mode(textLayer, GTextOverflowModeWordWrap);
	layer_add_child(windowLayer, text_layer_get_layer(textLayer));
  
	menuLayer = menu_layer_create(frame);
	menu_layer_set_highlight_colors(menuLayer, GColorDukeBlue, GColorWhite);
	menu_layer_set_callbacks(menuLayer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections_callback,
    .get_header_height = menu_get_header_height_callback,
    .draw_header = (MenuLayerDrawHeaderCallback) menu_draw_header_callback,
		.get_cell_height = (MenuLayerGetCellHeightCallback) get_cell_height_callback,
		.draw_row = (MenuLayerDrawRowCallback) draw_row_callback,
		.get_num_rows = (MenuLayerGetNumberOfRowsInSectionsCallback) get_num_rows_callback,
		.select_click = (MenuLayerSelectCallback) select_callback,
		.select_long_click = (MenuLayerSelectCallback) select_long_callback
  //  .draw_separator = (MenuLayerDrawSeparatorCallback) draw_separator_callback
	});
	
  scroll_layer_set_shadow_hidden(menu_layer_get_scroll_layer(menuLayer), true);
	scroll_layer_set_paging(menu_layer_get_scroll_layer(menuLayer), true);
	menu_layer_set_click_config_onto_window(menuLayer, window);
	layer_set_hidden(menu_layer_get_layer(menuLayer), true);
	layer_add_child(windowLayer, menu_layer_get_layer(menuLayer));
	menu_layer_set_normal_colors(menuLayer,PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite), GColorBlack) ;

}

static void window_unload(Window *window) {
	if (timer != NULL) app_timer_cancel(timer);
	menu_layer_destroy(menuLayer);
	text_layer_destroy(textLayer);
  gbitmap_destroy(TelldusOn);
  gbitmap_destroy(TelldusOff);
  gbitmap_destroy(TelldusOnI);
  gbitmap_destroy(TelldusOffI);
  gbitmap_destroy(NexaHome);
  gbitmap_destroy(NexaHomeI);
  
  for (int i = 0; i < 5; i++) {
    gbitmap_destroy(dim_icons[i]);
    gbitmap_destroy(dim_icons_i[i]);
  }
 }


static void init(void) {
	window = window_create();
	
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	window_set_background_color(window,PBL_IF_COLOR_ELSE(GColorVividCerulean, GColorWhite));
	const bool animated = true;
	s_sensor_flag = persist_exists(S_SENSOR_FLAG_PKEY) ? persist_read_bool(S_SENSOR_FLAG_PKEY) : false;
	s_event_flag = persist_exists(S_EVENT_FLAG_PKEY) ? persist_read_bool(S_EVENT_FLAG_PKEY) : false;
//	s_stampenv_flag = persist_exists(S_SHOWEVENT_FLAG_PKEY) ? persist_read_bool(S_SHOWEVENT_FLAG_PKEY) : false;
	s_stamp_flag = persist_exists(S_SHOWTIMESTAMP_FLAG_PKEY) ? persist_read_int(S_SHOWTIMESTAMP_FLAG_PKEY) : 0;
	s_sensor_format_flag = persist_exists(S_SENSOR_FORMAT_FLAG_PKEY) ? persist_read_bool(S_SENSOR_FORMAT_FLAG_PKEY) : false;

	window_stack_push(window, animated);
	app_message_register_inbox_received(in_received_handler);
	app_message_register_inbox_dropped(in_dropped_handler);
	app_message_register_outbox_sent(out_sent_handler);
	app_message_register_outbox_failed(out_failed_handler);

	app_message_open(1024,1024);
	//	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit(void) {

	persist_write_bool(S_SENSOR_FLAG_PKEY, s_sensor_flag);
	persist_write_bool(S_EVENT_FLAG_PKEY, s_event_flag);
//	persist_write_bool(S_SHOWEVENT_FLAG_PKEY, s_stampenv_flag);
	persist_write_int(S_SHOWTIMESTAMP_FLAG_PKEY, s_stamp_flag);
	persist_write_bool(S_SENSOR_FORMAT_FLAG_PKEY, s_sensor_format_flag);
	window_destroy(window);
}

int main(void) {
	init();

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

	app_event_loop();
	deinit();
} 