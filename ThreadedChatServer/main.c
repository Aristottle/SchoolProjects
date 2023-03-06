// Mason Stott
// COSC 360; Dr. Huang
// Lab A: Chat Server
// 12/9/2022

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "dllist.h"
#include "jrb.h"
#include "sockettome.h"

// Global tree for keeping track of the rooms
JRB all_rooms;

// This struct stores the necessary information for a client
typedef struct __client__ {
  char *name;
  int fd;
  FILE *fin, *fout;
} client;

// This struct stores the necessary information for a room
typedef struct __room__ {
  char *name;
  pthread_mutex_t *m_lock;
  pthread_cond_t *m_cond;
  Dllist clients, print_queue;
} room;

// Listens for message singals and prints them to the users.
void *r_thread(void *new_room) 
{
  room *curr_room = (room *) new_room;
  Dllist to_print, curr_client;
  FILE *fout;

  pthread_mutex_lock(curr_room->m_lock);
  while (1) 
  {
    pthread_cond_wait(curr_room->m_cond, curr_room->m_lock);
    // Iterate through the users
    dll_traverse(to_print, curr_room->print_queue) 
    {
      // Display the messages
      dll_traverse(curr_client, curr_room->clients) 
      {
        fout = ((client *) curr_client->val.v)->fout;
        // Delete the node when finished
        if (fputs(to_print->val.s, fout) == EOF || fflush(fout) != 0) { fclose(fout); dll_delete_node(curr_client); }
      }
    }
    // Reset the print queue
    free_dllist(curr_room->print_queue);
    curr_room->print_queue = new_dllist();
  }
  return NULL;
}

// This function does exactly what you'd think
void kill_client_thread(client *to_kill) 
{
  // Close the streams (don't cross the streams!!!)
  if (to_kill->fout) fclose(to_kill->fout);
  if (to_kill->fin) fclose(to_kill->fin);
  // Free memory
  free(to_kill->name);
  free(to_kill);
  // Close the thread
  pthread_exit(NULL);
}

// This function takes a user a user and connects them to a room. Prints the entry message on completion.
void connect_user_to_room(client *user, room *room)
{
  // Stores the message to print
  char message[256];
  // Lock the mutex and add the user to the room's list
  pthread_mutex_lock(room->m_lock);
  dll_append(room->clients, new_jval_v(user));
  // Set the message and put it in the print queue
  strcpy(message, user->name);
  strcat(message, " has joined\n\0");
  dll_append(room->print_queue, new_jval_s(strdup(message)));
  // Unlock the mutex and send signal
  pthread_mutex_unlock(room->m_lock);
  pthread_cond_signal(room->m_cond);
}

// Handles the client-side input and output. 
void *c_thread(void *new_client) 
{
  // String buffers
  char in_buff[256], out_buff[256];
  // Iterators/Nodes
  Dllist user_it;
  JRB room_it;
  // Pointers to current room and user
  room *curr_room;
  client *user = (client *) new_client;

  // Initialize the user
  user->fout = fdopen(user->fd, "w");
  user->fin = fdopen(user->fd, "r");
  // Make sure our filestreams were initialized
  if (user->fin == NULL || user->fout == NULL) { perror("client fd"); exit(1); }

  // Present the rooms and their clients to the user
  if (fputs("Chat Rooms:\n\n", user->fout) == EOF) kill_client_thread(user);
  jrb_traverse(room_it, all_rooms) 
  {
    curr_room = room_it->val.v;
    // Print the room name
    if (fputs(curr_room->name, user->fout) == EOF) kill_client_thread(user);
    if (fputs(":", user->fout) == EOF) kill_client_thread(user);
    // Print the clients
    dll_traverse(user_it, curr_room->clients)
    {
      // Print the clients. Kill the thread on error
      if (fputs(" ", user->fout) == EOF) kill_client_thread(user);
      if (fputs(((client *) user_it->val.v)->name, user->fout) == EOF) kill_client_thread(user);
    }
    // Just a newline for formatting. Kill the thread on error
    if (fputs("\n", user->fout) == EOF) kill_client_thread(user);
  }

  // Prompt the user for their display name
  if (fputs("\nEnter your chat name (no spaces):\n", user->fout) == EOF) kill_client_thread(user);
  if (fflush(user->fout) == EOF) kill_client_thread(user);
  if (fgets(in_buff, sizeof(in_buff), user->fin) == NULL) kill_client_thread(user);
  // Add null terminator
  in_buff[strlen(in_buff) - 1] = '\0';
  // Set the user's name
  user->name = strdup(in_buff);

  // Prompt the user for the room they wish to join
  if (fputs("Enter chat room:\n", user->fout) == EOF) kill_client_thread(user);
  if (fflush(user->fout) == EOF) kill_client_thread(user);
  if (fgets(in_buff, sizeof(in_buff), user->fin) == NULL) kill_client_thread(user);
  // Null termination 
  in_buff[strlen(in_buff) - 1] = '\0';
  // Get the user's desired room
  room_it = jrb_find_str(all_rooms, in_buff);
  // Check that the room exists, exit if it doesn't
  if (room_it == NULL) { fprintf(stderr, "Couldn't find room\n"); exit(1); }

  // Connect the user to the room
  curr_room = (room *) room_it->val.v;
  connect_user_to_room(user, curr_room);

  // Run the chat until the user leaves (on error or otherwise).
  while (1) 
  {
    strcpy(out_buff, user->name);
    // If we're getting input, just keep running
    if (fgets(in_buff, sizeof(in_buff), user->fin) != NULL) 
    {
      // Buffer the message
      strcat(out_buff, ": ");
      strcat(out_buff, in_buff);
      pthread_mutex_lock(curr_room->m_lock);
      // Queue the message for printing
      dll_append(curr_room->print_queue, new_jval_s(strdup(out_buff)));
      // Unlock and send signal
      pthread_mutex_unlock(curr_room->m_lock);
      pthread_cond_signal(curr_room->m_cond);
      // Continue looping
      continue;
    }

    // End the thread when we have no user input
    // Close input stream
    fclose(user->fin);
    // Buffer the leave message
    strcpy(out_buff, user->name);
    strcat(out_buff, " has left\n");
    pthread_mutex_lock(curr_room->m_lock);
    // Queue the exit message
    dll_append(curr_room->print_queue, new_jval_s(strdup(out_buff)));
    // Close the output stream if it isn't already
    if (user->fout != 0) 
    {
      fclose(user->fout);
      // Remove the user from the room's list
      dll_traverse(user_it, curr_room->clients) 
      { 
        if (strcmp(user->name, ((client *) user_it->val.v)->name) == 0) break;
      }
      dll_delete_node(user_it);
    }
    pthread_mutex_unlock(curr_room->m_lock);
    // Free the user
    free(user->name);
    free(user);
    // Send the signal and exit the thread
    pthread_cond_signal(curr_room->m_cond);
    pthread_exit(NULL);
  }
  return NULL;
}

// The main function 
int main(int argc, char **argv) 
{
  pthread_t *new_thread;
  room *new_room;
  client *new_client;
  int port, socket;

  // Validate the number of arguments
  if (argc < 3) { printf("Usage: %s <port> <chat room names...>\n", argv[0]); exit(1); }

  // Set the port number based on user input
  port = atoi(argv[1]);

  // Initialize the rooms tree
  all_rooms = make_jrb();
  // Create the rooms based on arguments
  for (int i = 2; i < argc; i++) 
  { 
    // Allocate memory
    new_thread = malloc(sizeof(pthread_t));
    // Create room
    new_room = malloc(sizeof(room));
    // Set the room name and slap it in the tree
    new_room->name = strdup(argv[i]);
    // Create the lists
    new_room->clients = new_dllist();
    new_room->print_queue = new_dllist();
    new_room->m_lock = malloc(sizeof(pthread_mutex_t));
    new_room->m_cond = malloc(sizeof(pthread_cond_t));
    // Initialize the mutex and condition
    pthread_mutex_init(new_room->m_lock, NULL);
    pthread_cond_init(new_room->m_cond, NULL);
    jrb_insert_str(all_rooms, new_room->name, new_jval_v(new_room));
    // Create the threads with our r_thread() function
    pthread_create(new_thread, NULL, r_thread, new_room);
    // Set it free!!!
    pthread_detach(*new_thread);
    // free(new_thread); // Freeing breaks things. Makes sense
    // free(new_room);
  }

  // Set our port
  socket = serve_socket(port);

  // Set up the client and their thread
  while (1) 
  {
    new_thread = malloc(sizeof(pthread_t));
    new_client = malloc(sizeof(client));
    new_client->fd = accept_connection(socket);
    // Make the thread
    pthread_create(new_thread, NULL, c_thread, new_client);
    // RELEASE THEM!!!
    pthread_detach(*new_thread);
    // free(new_thread);
    // free(new_room);
  }

  return 0;
}