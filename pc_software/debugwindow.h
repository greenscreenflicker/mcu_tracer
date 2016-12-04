#include <gtk/gtk.h>
#define FUNC_NAME_LENGTH 30

typedef struct set_variables{
	char label[30];
	int type; //0 notused (terminator), 1 long, 2 float, 3 toogle
	int rw;
	union{
		int32_t data_l;
		float data_f;
	};
	GtkWidget* label_widget;
	GtkWidget* data_widget;
} set_var_t;


typedef struct mcu_func{
	char name[FUNC_NAME_LENGTH];
	int id;
	GtkWidget* button;
} mcu_func_t;

typedef struct set_single_var{
	uint16_t addr;
	uint32_t val;
} set_single_var_t;

enum
{
  COL_ID = 0,
  COL_TIME,
  COL_MSG,
  NUM_COLS
} ;

extern GMainContext *gtk_context_var;


void gui_debug_window(void);

void inject_call(GSourceFunc func, gpointer data);
gboolean variables_window_update(struct set_variables *mydd);
gboolean variables_window_update_vars(uint32_t *datastream);
gboolean variables_window_update_single_var(set_single_var_t *data);
gboolean gui_msg_center_add_msg(char* msg);
gboolean say_hello(void);
gboolean FuncOnMCU_update(mcu_func_t *mcufunctions);

gboolean FuncOnMCU_update(mcu_func_t *mcufunctions);
void callback_func_click_reactivate(int *id_to_reactivate);
void timeout_reset_register(guint function);
void inject_timeout(GSourceFunc func, gpointer data, guint ms);
void timeout_reset_of_fail(guint function);
void func_reset_register_callback(guint function);
char* func_get_desciptor_by_id(int id);
void func_report_execution_fail(int id,int status);

//delete later
void FuncOnMCU_dummydata(void);
