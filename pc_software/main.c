#include "unified_header.h"



int main(int argc, char **argv)
{
    /* Do stuff as usual */
    gtk_init( &argc, &argv );
    
    //Open connect window
    gtk_context_var = g_main_context_default();
	gui_connect_window();
	 
    gtk_main ();

	return 0;
}

