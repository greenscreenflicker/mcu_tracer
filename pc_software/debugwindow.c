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

int system_streaming=0;


struct set_variables{
	char label[30];
	int type; //0 notused (terminator), 1 long, 2 float, 3 toogle
	int rw;
	long data_l;
	double data_f;
	GtkWidget* label_widget;
	GtkWidget* data_widget;
};

struct set_variables *set_variables_data;
struct set_variables *read_variables_data;

struct set_variables* dummydata(void){
	struct set_variables *mydd;
	//important to use calloc
	mydd=calloc(sizeof(struct set_variables),10);
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
			}else if(set_variables_data[loop].type==2){
				set_variables_data[loop].data_f=atof(gtk_entry_get_text(GTK_ENTRY(set_variables_data[loop].data_widget)));
			}				
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
				sprintf(str, "%li", set_variables_data[loop].data_l);
			}else if(set_variables_data[loop].type==2){
				sprintf(str, "%lf", set_variables_data[loop].data_f);
			}
			gtk_entry_set_text(GTK_ENTRY(set_variables_data[loop].data_widget),str);
			return;
		}
		loop=loop+1;
	}
}

static GtkWidget *
create_view_and_model (void)
{
	debugwindow_set_variables=gtk_frame_new ("Set variables");
	debugwindow_set_variables_grid=gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(debugwindow_set_variables), debugwindow_set_variables_grid);
    //Now we can add elements to the grid
   
    struct set_variables *mydd;
    mydd=dummydata();
    gint loop=0;
    
    while(mydd[loop].type){
		mydd[loop].label_widget=gtk_label_new(mydd[loop].label);
	
		char str[30];
		if(mydd[loop].type==1){
			sprintf(str, "%li", mydd[loop].data_l);
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
		
		//read write selection
		if(mydd[loop].rw>0){
			gtk_widget_set_sensitive (mydd[loop].data_widget, FALSE);
		}
		
		gtk_grid_attach (GTK_GRID (debugwindow_set_variables_grid), mydd[loop].label_widget, 0, loop, 1, 1); //pos x, pos y, width x, with y
		gtk_grid_attach (GTK_GRID (debugwindow_set_variables_grid), mydd[loop].data_widget, 1, loop, 1, 1); //pos x, pos y, width x, with y
		loop=loop+1;
	}
}

void gui_get_dummy_read_data(void){
	//
	read_variables_data=calloc(sizeof (struct set_variables),10);
	strcpy(read_variables_data[0].label,"Eins");
	read_variables_data[0].type=1;
	read_variables_data[0].data_l=1;
	strcpy(read_variables_data[1].label,"Zwei");
	read_variables_data[1].type=1;
	read_variables_data[1].data_l=2;
	strcpy(read_variables_data[2].label,"Float");
	read_variables_data[2].type=2;
	read_variables_data[2].data_f=2.456;
	strcpy(read_variables_data[3].label,"bool-");
	read_variables_data[3].type=3;
	read_variables_data[3].data_l=0;
	strcpy(read_variables_data[4].label,"bool+");
	read_variables_data[4].type=3;
	read_variables_data[4].data_l=1;
 }
 
void gui_read_variables_init(){
	//
	gui_get_dummy_read_data();
	debugwindow_read_variables=gtk_grid_new();
	gtk_widget_set_hexpand (debugwindow_read_variables, TRUE);
	gtk_widget_set_vexpand (debugwindow_read_variables, TRUE);
	
	int loop=0;
	while(read_variables_data[loop].type){
		read_variables_data[loop].label_widget=gtk_label_new(read_variables_data[loop].label);
		
		char str[30];
		if(read_variables_data[loop].type==1){
			sprintf(str, "%li", read_variables_data[loop].data_l);
		}else if(read_variables_data[loop].type==2){
			sprintf(str, "%lf", read_variables_data[loop].data_f);
		}else if(read_variables_data[loop].type==3){
			if(read_variables_data[loop].data_l==0){
				strcpy(str,"-");
			}else{
				strcpy(str,"+");
			}
		}
		read_variables_data[loop].data_widget=gtk_label_new(str);
		gtk_grid_attach (GTK_GRID (debugwindow_read_variables), read_variables_data[loop].label_widget, 0, loop, 1, 1); //pos x, pos y, width x, with y
		gtk_widget_set_hexpand (read_variables_data[loop].label_widget, TRUE);
		gtk_grid_attach (GTK_GRID (debugwindow_read_variables), read_variables_data[loop].data_widget, 1, loop, 1, 1); //pos x, pos y, width x, with y
		gtk_widget_set_hexpand (read_variables_data[loop].data_widget, TRUE);
		loop++;
	}
 }
 
void gui_debug_init(void){
	//
}

void gui_debug_stream(void){
	if(system_streaming==0){
		gtk_button_set_label (GTK_BUTTON(debugwindow_stream_button),"Stop Streaming");
		system_streaming=1;
		//ToDO: add code for starting streaming here.
	}else{
		gtk_button_set_label (GTK_BUTTON(debugwindow_stream_button),"Start Streaming");
		system_streaming=0;
	}		
}

void gui_debug_quit(void){
	gtk_main_quit ();
}

void gui_debug_window(void){
	printf("Starting debug window\n");
	
    debugwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(debugwindow),GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW(debugwindow),
                             800, //w
                             600); //h
	
	gtk_window_set_title (GTK_WINDOW (debugwindow), "Debug Window");	  
	
	debugwindow_grid=gtk_grid_new (); //Creating Grid
	
	//adding grid to window
	gtk_container_add (GTK_CONTAINER (debugwindow), debugwindow_grid);

	//Adding variable view. A notebook is used to have different display options.
	debugwindow_variable_view=gtk_notebook_new();
	//For now we add only dummy elements
	gui_read_variables_init();
	gtk_notebook_append_page (GTK_NOTEBOOK(debugwindow_variable_view),
                          debugwindow_read_variables,
                          gtk_label_new("Read variables"));
	gtk_notebook_append_page (GTK_NOTEBOOK(debugwindow_variable_view),
                          gtk_label_new("Clild Label 2"),
                          gtk_label_new("Tab Label 2"));
    gtk_notebook_append_page (GTK_NOTEBOOK(debugwindow_variable_view),
                          gtk_label_new("Clild Label 3"),
                          gtk_label_new("Tab Label 3"));                      
     
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_variable_view, 0, 0, 3, 1);
	
	//Adding the control variable box
	create_view_and_model();
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_set_variables, 3, 0, 1, 1); //pos x, pos y, width x, with y
	//Set selection mode of debug window to zero.

	///Adding buttons
	//Init
	debugwindow_init_button=gtk_button_new_with_label ("Reinit");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_init_button, 0, 1, 1, 1);

	//Stream
	debugwindow_stream_button=gtk_button_new_with_label ("Start Streaming");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_stream_button, 1, 1, 1, 1);
	g_signal_connect (debugwindow_stream_button, "clicked",G_CALLBACK (gui_debug_stream), NULL);
	//Emergency
	debugwindow_emergency_button=gtk_button_new_with_label ("Emergency");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_emergency_button, 2, 1, 1, 1);
	//Quit
	debugwindow_quit_button=gtk_button_new_with_label ("Quit");
	gtk_grid_attach (GTK_GRID (debugwindow_grid), debugwindow_quit_button, 3, 1, 1, 1);
	g_signal_connect (debugwindow_quit_button, "clicked",G_CALLBACK (gui_debug_quit), NULL);
	
	
	//Finally showing all
	gtk_widget_show_all(debugwindow);

}
