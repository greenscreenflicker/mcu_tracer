#include "unified_header.h"

GtkWidget *debugwindow;
GtkWidget *debugwindow_grid;
GtkWidget *debugwindow_variable_view;
GtkWidget *debugwindow_set_variables;
GtkWidget *debugwindow_set_variables_grid;
GtkWidget *debugwindow_read_variables;
GtkWidget *debugwindow_init_button;
GtkWidget *debugwindow_stream_button;
GtkWidget *debugwindow_emergency_button;
GtkWidget *debugwindow_quit_button;
GtkWidget *debugwindow_grid_msg_center;
GtkWidget *debugwindow_grid_msg_center_del_button;
GtkWidget *debugwindow_grid_msg_center_treeview;
GtkListStore  *debugwindow_grid_msg_center_treeview_list_store;
GMainContext *gtk_context_var;

GtkWidget *debugwindow_scroll_msg;

int system_streaming=0;



struct set_variables *set_variables_data;
struct set_variables *read_variables_data;

struct set_variables* dummydata(void){
	struct set_variables *mydd;
	//important to use calloc
	
	mydd=calloc(sizeof(set_var_t),100);
	
	strcpy(mydd[0].label,"Long");
	mydd[0].type=1;
	mydd[0].data_l=1234;
	strcpy(mydd[1].label,"Float");
	mydd[1].type=2;
	mydd[1].data_f=5678.4;
	strcpy(mydd[2].label,"Toggle A");
	mydd[2].type=3;
	mydd[2].data_l=1;
	strcpy(mydd[3].label,"Toggle D");
	mydd[3].type=3;
	mydd[3].data_l=0;
	set_variables_data=mydd;
	strcpy(mydd[4].label,"Toggle Dn");
	mydd[4].type=3;
	mydd[4].rw=1;
	mydd[4].data_l=0;
	set_variables_data=mydd;
	strcpy(mydd[5].label,"Float");
	mydd[5].type=2;
	mydd[5].rw=1;
	mydd[5].data_f=5678.4;
	//monitor_master_get_variables((char*) mydd);
	return mydd;
}

//Toogle Button variable change. Writes changes to memory
void callback_set_variables_changed(GtkWidget *widget, gpointer   data ){

    gint loop=0;
    while(set_variables_data[loop].type){
		if(set_variables_data[loop].data_widget==widget){	
			if(set_variables_data[loop].type!=3){
				printf("this is not a toogle button.\n");
			}
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(set_variables_data[loop].data_widget))==TRUE){
				set_variables_data[loop].data_l=1;
				//printf("%s: active\n",set_variables_data[loop].label);
			}else{
				set_variables_data[loop].data_l=0;
				//printf("%s: deactive\n",set_variables_data[loop].label);
			}
			monitor_master_write_var(loop,set_variables_data[loop].data_l);
			return;
		}

		loop=loop+1;
	}
}

//If you press enter, the value is saved to the right elements.
void callback_set_variables_changed_text(GtkWidget *widget, gpointer   data ){
	//printf("We had a text enter.\n");
    gint loop=0;
    while(set_variables_data[loop].type){
		if(set_variables_data[loop].data_widget==widget){	
			if(set_variables_data[loop].type==1){
				set_variables_data[loop].data_l=atoi(gtk_entry_get_text(GTK_ENTRY(set_variables_data[loop].data_widget)));
				//printf("entered: %i\n",set_variables_data[loop].data_l);
			}else if(set_variables_data[loop].type==2){
				set_variables_data[loop].data_f=atof(gtk_entry_get_text(GTK_ENTRY(set_variables_data[loop].data_widget)));
			}
			//transfer data to mcu
			monitor_master_write_var(loop,set_variables_data[loop].data_l);
			return;
		}
		loop=loop+1;
	}
}

//If an element is entered or left, it's value its set to the orignal value.
//This does this function
void callback_set_variables_no_enter(GtkWidget *widget, gpointer   usrdata ){
    gint loop=0;
    while(set_variables_data[loop].type){
		if(set_variables_data[loop].data_widget==widget){
			//printf("%s was changed...\n",set_variables_data[loop].label);
			//Set string to orginal
			char str[30];
			if(set_variables_data[loop].type==1){
				sprintf(str, "%i", set_variables_data[loop].data_l);
			}else if(set_variables_data[loop].type==2){
				sprintf(str, "%lf", set_variables_data[loop].data_f);
			}
			gtk_entry_set_text(GTK_ENTRY(set_variables_data[loop].data_widget),str);
			return;
		}
		loop=loop+1;
	}
}


void
variables_window (void)
{
	debugwindow_set_variables=gtk_frame_new ("MCU variables");
	debugwindow_set_variables_grid=gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(debugwindow_set_variables), debugwindow_set_variables_grid);
    //Now we can add elements to the grid
    set_variables_data=NULL;
 

}

gboolean variables_window_update(struct set_variables *mydd){
	//recrusivly destroy grid
	gtk_widget_destroy(debugwindow_set_variables_grid);
	free(set_variables_data); //free óld memory
	
	//create new grid
	debugwindow_set_variables_grid=gtk_grid_new();
	//add it to container 
	gtk_container_add(GTK_CONTAINER(debugwindow_set_variables), debugwindow_set_variables_grid);
	 
	//struct set_variables *mydd;
    if(mydd==NULL){
		printf("data aquisition failed\n");
		//return;
	}
    set_variables_data=mydd;
    gint loop=0;
    
    while(mydd[loop].type){
		//printf("%i: creating %s\n",loop,mydd[loop].label);
		
		mydd[loop].label_widget=gtk_label_new(mydd[loop].label);
	
		char str[30];
		if(mydd[loop].type==1){
			sprintf(str, "%i", mydd[loop].data_l);
			mydd[loop].data_widget=gtk_entry_new();
			gtk_entry_set_text(GTK_ENTRY(mydd[loop].data_widget),str);
			g_signal_connect (mydd[loop].data_widget, "activate",  G_CALLBACK (callback_set_variables_changed_text), NULL);
			g_signal_connect (mydd[loop].data_widget, "state-changed",  G_CALLBACK (callback_set_variables_no_enter), NULL);
		}else if(mydd[loop].type==2){
			sprintf(str, "%lf", mydd[loop].data_f);
			mydd[loop].data_widget=gtk_entry_new();
			gtk_entry_set_text(GTK_ENTRY(mydd[loop].data_widget),str);
			g_signal_connect (mydd[loop].data_widget, "activate",  G_CALLBACK (callback_set_variables_changed_text), NULL);
			g_signal_connect (mydd[loop].data_widget, "state-changed",  G_CALLBACK (callback_set_variables_no_enter), NULL);
		}else if(mydd[loop].type==3){
			mydd[loop].data_widget=gtk_check_button_new ();
			//gboolean active=(mydd[0].data_l!=0);
			gboolean active=TRUE;
			if(mydd[loop].data_l==0){
				active=FALSE;
			}
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(mydd[loop].data_widget),active);
			g_signal_connect (mydd[loop].data_widget, "toggled",  G_CALLBACK (callback_set_variables_changed), NULL);
		}
		
		//Horizontaly expand
		gtk_widget_set_hexpand (mydd[loop].label_widget, TRUE);
		gtk_widget_set_hexpand (mydd[loop].data_widget, TRUE);
		
		//read write selection
		if(mydd[loop].rw>0){
			gtk_widget_set_sensitive (mydd[loop].data_widget, FALSE);
		}
		
		gtk_grid_attach (GTK_GRID (debugwindow_set_variables_grid), mydd[loop].label_widget, 0, loop, 1, 1); //pos x, pos y, width x, with y
		gtk_grid_attach (GTK_GRID (debugwindow_set_variables_grid), mydd[loop].data_widget, 1, loop, 1, 1); //pos x, pos y, width x, with y
	
		loop=loop+1;
	}
	//printf("%i: creating %s\n",1,mydd[1].label);
	gtk_widget_show_all(debugwindow_set_variables_grid);
	return G_SOURCE_REMOVE;
}

gboolean variables_window_update_vars(uint32_t *datastream){
	//Updates the values in the specific input types
	struct set_variables *mydd;
	mydd=set_variables_data; //local working copy to easy my life
    gint loop=0;
    
    while(mydd[loop].type){
		mydd[loop].data_l=datastream[loop];
		char str[30];
		if(mydd[loop].type==1){
			sprintf(str, "%i", mydd[loop].data_l);
			//do not overwrite, when user is editing
			if(gtk_widget_is_focus (GTK_WIDGET(mydd[loop].data_widget))==FALSE){
				gtk_entry_set_text(GTK_ENTRY(mydd[loop].data_widget),str);
			}
		}else if(mydd[loop].type==2){
			sprintf(str, "%lf", mydd[loop].data_f);
			//do not overwrite, when user is editing
			if(gtk_widget_is_focus (GTK_WIDGET(mydd[loop].data_widget))==FALSE){
				gtk_entry_set_text(GTK_ENTRY(mydd[loop].data_widget),str);
			}
		}else if(mydd[loop].type==3){
			mydd[loop].data_widget=gtk_check_button_new ();
			//gboolean active=(mydd[0].data_l!=0);
			gboolean active=TRUE;
			if(mydd[loop].data_l==0){
				active=FALSE;
			}
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(mydd[loop].data_widget),active);
		}	
		loop=loop+1;
	}
	free(datastream);
	return G_SOURCE_REMOVE;	
}
gboolean variables_window_update_single_var(set_single_var_t *data){
	//
	int loop=data->addr; //we should check addr later!!! TODO!!! RISK!!! FOR CRASHING!
	struct set_variables *mydd;
	mydd=set_variables_data;
	mydd[loop].data_l=data->val;
	//printf("addr:%i|data:%i\n",data->addr,data->val);
	char str[30];
	if(mydd[loop].type==1){
		sprintf(str, "%i", mydd[loop].data_l);
		gtk_entry_set_text(GTK_ENTRY(mydd[loop].data_widget),str);
	}else if(mydd[loop].type==2){
		sprintf(str, "%lf", mydd[loop].data_f);
		gtk_entry_set_text(GTK_ENTRY(mydd[loop].data_widget),str);
	}else if(mydd[loop].type==3){
		mydd[loop].data_widget=gtk_check_button_new ();
		//gboolean active=(mydd[0].data_l!=0);
		gboolean active=TRUE;
		if(mydd[loop].data_l==0){
			active=FALSE;
		}
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(mydd[loop].data_widget),active);
	}
	free(data);
	return G_SOURCE_REMOVE;	
}

unsigned int gui_msgcenterid=0;
gboolean gui_msg_center_add_msg(char* msg){
	GtkTreeIter    iter;
	char timeformat[100];
	time_t rawtime;
	struct tm *info;
	time( &rawtime );
    info = localtime( &rawtime );
    strftime(timeformat,80,"%H:%M:%S %d.%m.%Y", info);
   
	gtk_list_store_prepend (debugwindow_grid_msg_center_treeview_list_store, &iter);
	gtk_list_store_set (debugwindow_grid_msg_center_treeview_list_store, &iter,
					  COL_ID,gui_msgcenterid,
					  COL_TIME,timeformat,
					  COL_MSG,msg,
                      -1);
    /*
    if(gui_msgcenterid>15){
		GtkTreePath *path;
		path = gtk_tree_path_new_from_string ("16"); //who required that bad workaround? stupid implementation!
		gtk_tree_model_get_iter (GTK_TREE_MODEL (debugwindow_grid_msg_center_treeview_list_store),
                           &iter,
                           path);
        gtk_list_store_remove (debugwindow_grid_msg_center_treeview_list_store,
                       &iter);
    }*/
	gui_msgcenterid=gui_msgcenterid+1;
	gtk_widget_show(debugwindow_grid_msg_center_treeview); //make sure we see added element
	//gtk_widget_show_all(debugwindow);
	free(msg);
	return G_SOURCE_REMOVE;	
}

void gui_treeview_fix(void){
	//
	GtkTreeIter    iter;
	gtk_list_store_prepend (debugwindow_grid_msg_center_treeview_list_store, &iter);
	gtk_list_store_remove (debugwindow_grid_msg_center_treeview_list_store,&iter);
	gtk_widget_show(debugwindow_grid_msg_center_treeview);
}

void gui_msg_center_clear_msg(void){
	gtk_list_store_clear (debugwindow_grid_msg_center_treeview_list_store);
	gui_msgcenterid=0;
}

void gui_msg_center_add_msg_test(void){
	 char *str=malloc(50);
	 strcpy(str,"This is a test");
	 gui_msg_center_add_msg(str);
 }
 
static GtkTreeModel *
gui_msg_center_message_list_dummy_data (void)
{
   GtkListStore  *store;
  GtkTreeIter    iter;
  
  store = gtk_list_store_new (NUM_COLS, G_TYPE_UINT, G_TYPE_STRING,G_TYPE_STRING);
  debugwindow_grid_msg_center_treeview_list_store=store;
  
  return GTK_TREE_MODEL (store);
}

void gui_msg_center_message_list_create (void){
  GtkCellRenderer     *renderer;
  GtkTreeModel        *model;
  GtkWidget           *view;

  view = gtk_tree_view_new ();
  gui_msgcenterid=0;
  /* --- Column #1 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "#",  
                                               renderer,
                                               "text", COL_ID,
                                               NULL);

  /* --- Column #2 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "time",  
                                               renderer,
                                               "text", COL_TIME,
                                               NULL);
  /* --- Column #3 --- */

  renderer = gtk_cell_renderer_text_new ();
  gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
                                               -1,      
                                               "message",  
                                               renderer,
                                               "text", COL_MSG,
                                               NULL);

  model = gui_msg_center_message_list_dummy_data ();

  gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);

  /* The tree view has acquired its own reference to the
   *  model, so we can drop ours. That way the model will
   *  be freed automatically when the tree view is destroyed */

  g_object_unref (model);
  gtk_widget_set_hexpand (view, TRUE);
  debugwindow_grid_msg_center_treeview=view;
}



void gui_msg_center_init(void){
	//New GRID
	//debugwindow_grid_msg_center=gtk_grid_new ();
	//Button for deleting messages

	

	gui_msg_center_message_list_create();

	
	
	//gtk_grid_attach (GTK_GRID (debugwindow_grid_msg_center), debugwindow_grid_msg_center_treeview, 0, 0, 1, 1); //pos x, pos y, width x, with y
	
	debugwindow_scroll_msg=gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (debugwindow_scroll_msg),
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_container_add(GTK_CONTAINER(debugwindow_scroll_msg),debugwindow_grid_msg_center_treeview);
}
 
void gui_debug_emergency(void){
	//
	monitor_master_emergency(); //send to microcontroller
	char *msg=malloc(100);
	strcpy(msg,"Emergency button pressed");
	gui_msg_center_add_msg(msg);
}

void gui_debug_stream(void){
	if(system_streaming==0){
		gtk_button_set_label (GTK_BUTTON(debugwindow_stream_button),"Stop Streaming");
		system_streaming=1;
		//ToDO: add code for starting streaming here.
		monitor_master_req_frequently_enable();
	}else{
		gtk_button_set_label (GTK_BUTTON(debugwindow_stream_button),"Start Streaming");
		system_streaming=0;
		monitor_master_req_frequently_disable();
	}		
	monitor_master_req_data();
}

void gui_debug_quit(void){
	monitor_rs232_disconnect();
	gtk_main_quit ();
}

void gui_debug_init_data(void){
	monitor_master_req_init();
	//a short sleep so we dont messed up
	//usleep(5000); -->This delay is not required, as we added portionierer
	monitor_master_req_data();
	gui_msg_center_clear_msg();
	char *msg=malloc(100);
	strcpy(msg,"Debugging session started");
	gui_msg_center_add_msg(msg);
}

void gui_notebook_switched_page(void){
	gui_treeview_fix();
}

void gui_debug_window(void){
	//printf("Starting debug window\n");
	
    debugwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(debugwindow),GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW(debugwindow),
                             800, //w
                             600); //h
	
	gtk_window_set_title (GTK_WINDOW (debugwindow), "MCU Tracer");	  
	
	debugwindow_grid=gtk_grid_new (); //Creating Grid

	//adding grid to window
	gtk_container_add (GTK_CONTAINER (debugwindow), debugwindow_grid);

	//Adding variable view. A notebook is used to have different display options.
	debugwindow_variable_view=gtk_notebook_new();
	gtk_widget_set_hexpand (debugwindow_variable_view, TRUE);
	gtk_widget_set_vexpand (debugwindow_variable_view, TRUE);
	//For now we add only dummy elements
	//gui_read_variables_init();
	variables_window();
	
	gtk_notebook_append_page (GTK_NOTEBOOK(debugwindow_variable_view),
                          debugwindow_set_variables,
                          gtk_label_new("Variables"));
    gui_msg_center_init();
    

	gtk_notebook_append_page (GTK_NOTEBOOK(debugwindow_variable_view),
                          debugwindow_scroll_msg,
                          gtk_label_new("Messages"));
                          /*
    gtk_notebook_append_page (GTK_NOTEBOOK(debugwindow_variable_view),
                          gtk_label_new("unimplemented feature"),
                          gtk_label_new("Setup"));    */                  
     
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_variable_view, 0, 0, 5, 1);
	//adding signal
	g_signal_connect (debugwindow_variable_view, "switch-page",G_CALLBACK (gui_notebook_switched_page), NULL);
	
	//Adding the control variable box
	
	
	
	//gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_set_variables, 3, 0, 1, 1); //pos x, pos y, width x, with y
	//Set selection mode of debug window to zero.

	///Adding buttons
	//Init
	debugwindow_init_button=gtk_button_new_with_label ("Reinit");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_init_button, 0, 1, 1, 1);
	g_signal_connect (debugwindow_init_button, "clicked",G_CALLBACK (gui_debug_init_data), NULL);

	//Stream
	debugwindow_stream_button=gtk_button_new_with_label ("Start Streaming");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_stream_button, 1, 1, 1, 1);
	g_signal_connect (debugwindow_stream_button, "clicked",G_CALLBACK (gui_debug_stream), NULL);
	//Clear message
	debugwindow_grid_msg_center_del_button=gtk_button_new_with_label("Clear messages");
	g_signal_connect (debugwindow_grid_msg_center_del_button, "clicked",G_CALLBACK (gui_msg_center_clear_msg), NULL);
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_grid_msg_center_del_button, 2, 1, 1, 1);
	
	//Emergency
	debugwindow_emergency_button=gtk_button_new_with_label ("Emergency");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_emergency_button, 3, 1, 1, 1);
	g_signal_connect (debugwindow_emergency_button, "clicked",G_CALLBACK (gui_debug_emergency), NULL);
	
	//Quit
	debugwindow_quit_button=gtk_button_new_with_label ("Quit");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_quit_button, 4, 1, 1, 1);
	g_signal_connect (debugwindow_quit_button, "clicked",G_CALLBACK (gui_debug_quit), NULL);
	
	


	//allow the read threat to change our window
	monitor_master_allow_decoding();
	//Load our variables
	gui_debug_init_data();
	gui_debug_stream(); //Start streaming automatically
	

	
	
	//Finally showing all
	gtk_widget_show_all(debugwindow);
}

gboolean say_hello(void){
	printf("Hello\n");
	return G_SOURCE_REMOVE;
}

void inject_call(GSourceFunc func, gpointer data){
	GSource *source;
	source = g_idle_source_new();
	g_source_set_callback(source, func, data, NULL);
	g_source_attach(source, gtk_context_var);
	
	g_source_unref(source);
}
