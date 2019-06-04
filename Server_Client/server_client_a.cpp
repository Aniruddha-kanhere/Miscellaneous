/*
    Copyright ©: Aniruddha Kanhere
    C socket server & client, handles multiple clients and talks to multiple servers
    using threads
*/

#include<stdio.h>           			//printf, other IOs etc.
#include<string.h>                              //strlen
#include<stdlib.h>                              //strlen
#include<unistd.h>                              //write
#include<iostream>				//cout and/or printf etc..
#include<time.h>                                //time - sync all
#include<sys/socket.h>				//for sockets
#include<arpa/inet.h>                           //inet_addr
#include<pthread.h>                             //for threading , link with lpthread

#define NUM 3 				    //Number of nodes
int my_port=8000;                           //we will start the numbering from 8000 onwards; this is temporary since main updates this variable
int port_numbers[NUM-1] = {};
int socket_desc;
using namespace std;

double perron[][3] = {{(double)2/3,(double)1/3,0.000000000},
                      {(double)1/3,(double)1/3,(double)1/3},
                      {0.000000000,(double)1/3,(double)2/3}};

volatile int number_of_connections=0;
volatile int received_flag=0,done_flag=0;
double initial_val=0;

volatile int ready[NUM-1];
volatile int go[NUM-1];
int my_num=0;
int updated=0,updated_client=0;
time_t time_val;
int global_port_offset = 0, busy = 0,server_busy=0;
int server_acknowledge[NUM];
char* temp_server_reply;
volatile int ready_client[NUM];
volatile int go_client[NUM];
double my_val_new =initial_val;
double my_val_old=initial_val;
double converge[1000];
double client_data[NUM];
int client_changed=0,diff_value=20;
int current_counter = 1;

void calculate(void)
{
//  static int current_counter=0;
  int go_flag_client=0;

  //time_val = time(NULL);

  while(!go_flag_client)         //wait till all clients have sent the present data;
  {
    go_flag_client=1;

    for(int i=0;i<NUM;i++)
    {
      if(!ready_client[i] && (i!=(my_port - 8000)))
      {
        go_flag_client=0;
      }
    }
  }

  //Renew the current value to be sent
  my_val_new = 0;//perron[my_port-8000][my_port-8000]*my_val_old + perron[my_port-8000][;
  for(int i=0;i<NUM;i++)
  {
     if(i == (my_port - 8000))
        my_val_new += perron[my_port-8000][i]*my_val_old;
     else
        my_val_new += perron[my_port-8000][i]*client_data[i];
//     printf("va %f\n",client_data[i]);
  }

  converge[current_counter] = my_val_new;
  current_counter++;
  printf("curr: %f\n",my_val_new);
  my_val_old = my_val_new;

  //ask the clients to send this value
  for(int i=0;i<NUM;i++)
    go_client[i]=1;

  return;
}


//class for the communication with threads. doesn't contain much but maybe will be used later
class client_class
{
  public:
  int active=0;
  static void *connection_handler(void *);
  static void *send_client_req(void *);
};

void *client_class::send_client_req(void *ip)
{
    while(busy);            //wait while others are doing something

    busy = 1;                //stop others from accessing the array
    int port_to_connect = 8000 + global_port_offset;
    global_port_offset ++;
    if(port_to_connect == my_port)
    {
        port_to_connect = 8000 + global_port_offset;
        global_port_offset++;
    }
    cout<<"Connect to "<<port_to_connect;
    busy=0;                    //allow others to access this

    int sock;
    struct sockaddr_in server_to_connect;
    char message[500], server_reply[1000];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)
    {
        printf("could not create socket (%d)",port_to_connect);
        while(1);
    }
    puts("Socket created");

    server_to_connect.sin_addr.s_addr = inet_addr((char*)ip);
    server_to_connect.sin_family = AF_INET;
    server_to_connect.sin_port = htons(port_to_connect);

    int print_flag = 0;
    while(1)
    {
        if(connect(sock, (struct sockaddr*) &server_to_connect, sizeof(server_to_connect))<0)
        {
            if(!print_flag)
            {
                cout<<"Connect failed. Retrying..."<<port_to_connect;
                print_flag=1;
            }
        }
        else
            break;
    }

    cout<<"connected "<<port_to_connect<<endl;

    int my_index = port_to_connect - 8000;

    while(1)
    {
        double chr[] = {(double)my_port,my_val_old};

        while(!go_client[my_index])
        {
           //if(time(NULL)- time_val >2)
             // printf(" %d",my_index);
        }
        go_client[my_index] = 0;
        ready_client[my_index] = 0;
        updated_client = 0;

        client_changed = 1;

        if(send(sock,chr,sizeof(chr),0)<0)
        {
            puts("send failed");
        }

        ready_client[my_index] = 1;

        if(recv(sock,server_reply,200,0)<0)
        {
            puts("receive failed");
        }

//      cout<<"server reply: "<<*(int*)(server_reply)<<" ";
        server_acknowledge[my_index] = *(int*)server_reply;
    }

    while(1);
}

//This will handle connection for each client
void *client_class::connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];

    while(server_busy == 1);

    server_busy = 1;
    int my_val = my_num;
    my_num++;
    server_busy = 0;

    //Send some messages to the client
    message = (char*)"Greetings! I am your connection handler\n";
    puts(message);

    //Receive a message from client
    number_of_connections++;
    while(number_of_connections < (NUM-1));

    for(int i =0;i<NUM;i++)
      go_client[i]=1;

    go[my_val]=0;

    while((read_size = recv(sock , client_message , 2000 , 0))>0)
    {
        ready[my_val]=1;

//        updated=0;

        double *temp_data = (double *)&client_message[0];
//        printf(" %d-%d ",(int)(*temp_data),(int)*(temp_data+1));
        client_data[(int)(*temp_data)-8000] = *(temp_data + 1);

        while(!go[my_val]);

        ready[my_val]=0;
        go[my_val]=0;
        updated=0;

        static int feedback;
        if(current_counter>diff_value)
           feedback = 0;
        else
           feedback = 1;

        write(sock , &feedback , sizeof(feedback));
    }

    if(read_size == 0)
    {
        printf("Client %d disconnected", my_val);
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

    number_of_connections--;
    ready[my_val]=1;
    //Free the socket pointer
    free(socket_desc);
}

//controls synchronous execution of all threads.
void *controller(void *)
{
    while(1)
    {
       while(updated || updated_client);
       updated = 1;
       updated_client = 1;

       int controller_flag=1;

       while(controller_flag)
       {
          controller_flag=0;
          for(int i=0;i<NUM-1;i++)
  	    if(!ready[i])
            {
               controller_flag=1;
            }
       }

       calculate();

       for(int i=0;i<NUM-1;i++)
         go[i]=1;

       time_val = time(NULL);
       while(time(NULL) - time_val == 0);

       int sum = 0;
       for(int i=0;i<NUM;i++)
         sum += server_acknowledge[i];

  /*     if(sum==1)
       {
//         while(1)
//           printf("DONE!");
        // exit(0);
//         while(1);
       }*/
     }
}

void *accept_clients(void *)
{
    client_class function[NUM-1];                                  //make corresponding handler function instances
    pthread_t sniffer_thread[NUM-1];                               //make sniffer thread IDs
    int client_sock , c , *new_sock;        			   //various variables used
    c = sizeof(struct sockaddr_in);
    struct sockaddr_in client;                             	   //instances of server and client structures
    int i=0;

    while(client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c))
    {
        puts("Connection accepted");

        new_sock = (int*)malloc(1);
        *new_sock = client_sock;

        if(pthread_create( &sniffer_thread[i] , NULL ,  function[i].connection_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
        }
    }

    if (client_sock < 0)
    {
        perror("accept failed");
    }

    while(1);
}

int main(int argc , char *argv[])
{
//    printf("%f ",((double)10/3));
    my_port = (int)strtoul(argv[1],NULL,10);
    //temp_server_reply = (char*)argv[2];
    my_val_old = (int)strtoul(argv[2],NULL,10);

    current_counter = 0;

    struct sockaddr_in server , client;                              //instances of server and client structures

    for (int i =0; i<NUM-1;i++)
    {
        if((8000+i) != my_port)
        {
            port_numbers[i] = 8000+i;
        }

        ready[i] = 0;
        go[i] = 0;
//        go_client[i]=0;
//        ready_client[i]=0;
//        server_acknowledge[i] = 1;
    }

    for(int  i=0; i<NUM; i++)
    {
      go_client[i]=0;
      ready_client[i]=0;
      server_acknowledge[i] = 1;
    }

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);

    if (socket_desc == -1)
    {
        printf("Could not create socket");
        return 1;
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(my_port);

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen and set the queue to max 10. Anyways the clients are going to keep trying
    listen(socket_desc , (NUM<10?NUM:10));

    //Accept and incoming connection
    puts("Waiting for incoming connections...");

    pthread_t controller_thr, accept_clients_thr;
    pthread_create(&controller_thr, NULL, controller,NULL);

//    struct sockaddr_in client_temp;
  //  int c_temp;
/*    while(accept(socket_desc,(struct sockaddr *)&client_temp,(socklen_t*)&c_temp))
    {
      cout<<"ACCepted ";
    }*/
    pthread_create(&accept_clients_thr, NULL, accept_clients,NULL);
    client_class clients[NUM-1];

    pthread_t client_req[NUM];

    pthread_create(&client_req[0],NULL,clients[0].send_client_req, (void*)"192.168.7.2");
    pthread_create(&client_req[1],NULL,clients[1].send_client_req, (void*)"192.168.7.2");
    pthread_create(&client_req[2],NULL,clients[2].send_client_req, (void*)"192.168.7.2");

    while(1);        //empty loop since all other work is done by the threads
    return 0;
}

