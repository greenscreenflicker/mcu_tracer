
 
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "interfacer.h"
#include "connect.h"
#include "rs232.h"
#include "debugwindow.h"
extern GMutex *lock_rs232;
extern GMutex *lock_paritycheckbit;
extern GMutex *lock_rec_thread;
