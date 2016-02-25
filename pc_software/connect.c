/*Connection Window*/

#include "unified_header.h"

//Globale elemente der GUI
GtkWidget *window;
GtkWidget *table;
GtkWidget *label_port;
GtkWidget *serial;
GtkWidget *feedback_box;
GtkWidget *combo_serial;
GtkWidget *combo_baud;

static void gui_connect_window_destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}



void gui_connect_window_connect2serial( GtkWidget *widget,  gpointer   data ) {
	//
	

	
	
	//char edit[250];
	
	int serialinterface;
	serialinterface=gtk_combo_box_get_active(GTK_COMBO_BOX(combo_serial));
	//sprintf(edit,"device %i: %s",serialinterface,comports[serialinterface]);
	
	
	int bdrate;
	bdrate=gtk_combo_box_get_active(GTK_COMBO_BOX(combo_baud));
	//sprintf(edit,"baudrate %i: %i",bdrate,baudrates[bdrate]);
	
	//gtk_label_set_text(GTK_LABEL(feedback_box),edit);
	//return;
	//serial_test_port(16,9600);
	
	//ToDo: later we should think of a better test here!
	
	int status=test_connection(serialinterface,baudrates[bdrate]);
	if(status==-1){
		gtk_label_set_text(GTK_LABEL(feedback_box),"Serial port could not be opened");
		return;
	}else if(status==-2){
		gtk_label_set_text(GTK_LABEL(feedback_box),"Device not responding");
		return;
	}else if(status==1){
		//gtk_label_set_text(GTK_LABEL(feedback_box),"connection okay");
		gtk_widget_hide(window);
		gui_debug_window();
	}
	
	/*
	//start connection
	monitor_rs232_connect(serialinterface,baudrates[bdrate]);
	monitor_init();
	usleep(10000);
	monitor_rs232_flush_recieved_data();
	monitor_master_streaming_start();
	usleep(10000);
	monitor_rs232_flush_recieved_data();
	
	monitor_rs232_disconnect();*/	
}


void gui_connect_window(void){

    
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW(window),
                             200, //w
                             80); //h
    /* When the window is given the "delete-event" signal (this is given
     * by the window manager, usually by the "close" option, or on the
     * titlebar), we ask it to call the delete_event () function
     * as defined above. The data passed to the callback
     * function is NULL and is ignored in the callback function. */
    g_signal_connect (window, "delete-event",
		      G_CALLBACK (gui_connect_window_destroy), NULL);
    
    /* Here we connect the "destroy" event to a signal handler.  
     * This event occurs when we call gtk_widget_destroy() on the window,
     * or if we return FALSE in the "delete-event" callback. */
    g_signal_connect (window, "destroy",
		      G_CALLBACK (gui_connect_window_destroy), NULL);
	gtk_window_set_title (GTK_WINDOW (window), "Connect...");	    
    gtk_widget_show  (window);
    
	/* Create a 2x2 table */
    table = gtk_grid_new ();

    /* Put the table in the main window */
    gtk_container_add (GTK_CONTAINER (window), table);
    
     
    //Create Label
    label_port=gtk_label_new("Device");
    //atach label to table
    gtk_grid_attach (GTK_GRID (table), label_port, 0, 0, 1, 1);
    //Show Label
  
    

    //Create a combobox
    
	 
    combo_serial = gtk_combo_box_text_new();
    int combo_items;
    for(combo_items=0;combo_items<37;combo_items++){
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_serial), NULL,comports[combo_items]);
	}
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_serial), 16);

	//GtkWidget *testlabel;
	//testlabel=gtk_label_new("test");
    //atach label to table
    //gtk_table_attach_defaults (GTK_TABLE (table), combo_serial, 1, 2, 0, 1);
    
    //Wir fangen mit nem Textfeld an, das ist leichter
    //serial=gtk_entry_new ();
    //gtk_entry_set_text (GTK_ENTRY(serial),"/dev/ttyUSB0");
    gtk_grid_attach (GTK_GRID (table), combo_serial, 1, 0, 1, 1);
    //gtk_widget_show(combo_serial);
    
    //Add baud rate
    
    GtkWidget *baud_label;
    baud_label=gtk_label_new("Baud");
    gtk_grid_attach (GTK_GRID (table), baud_label, 0, 1, 1, 1);
    
    //Add baud rate combo
    combo_baud = gtk_combo_box_text_new();
    char convert2str[20];
    for(combo_items=0;combo_items<sizeof(baudrates);combo_items++){
		sprintf(convert2str,"%i",baudrates[combo_items]);
		gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo_baud), NULL,convert2str);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_baud), 13);
	gtk_grid_attach (GTK_GRID (table), combo_baud, 1, 1, 1, 1);
    
    //Add feedback box
    
    feedback_box=gtk_label_new("please connect");
    gtk_grid_attach (GTK_GRID (table), feedback_box, 0, 2, 2, 1);
    
    //Add quit button
    GtkWidget *connect_quit_button;
    connect_quit_button= gtk_button_new_with_label("Quit");
	gtk_grid_attach (GTK_GRID (table), connect_quit_button, 0, 3, 1, 1);
	//Destroy
	/* This will cause the window to be destroyed by calling
     * gtk_widget_destroy(window) when "clicked".  Again, the destroy
     * signal could come from here, or the window manager. */
    g_signal_connect_swapped (connect_quit_button, "clicked",
			      G_CALLBACK (gui_connect_window_destroy),
                              window);
                              
    //Add connect button
    GtkWidget *connect_connect_button;
    connect_connect_button= gtk_button_new_with_label("connect");
    g_signal_connect_swapped (connect_connect_button,"clicked",G_CALLBACK (gui_connect_window_connect2serial), window);
	gtk_grid_attach (GTK_GRID (table), connect_connect_button, 1, 3, 1, 1);
	
    //Final stuff
    gtk_widget_show_all(window);
    
    
        
    gtk_main ();
    
    return;
}
