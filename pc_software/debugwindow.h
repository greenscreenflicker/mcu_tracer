#include <gtk/gtk.h>

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

typedef struct set_single_var{
	uint16_t addr;
	uint32_t val;
} set_single_var_t;

extern GMainContext *gtk_context_var;


void gui_debug_window(void);

void inject_call(GSourceFunc func, gpointer data);
gboolean variables_window_update(struct set_variables *mydd);
gboolean variables_window_update_vars(uint32_t *datastream);
gboolean variables_window_update_single_var(set_single_var_t *data);
gboolean say_hello(void);
