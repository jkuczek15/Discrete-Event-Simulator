

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "NESssq.h"
#include "rng.h"



bool halt;      /* End of simulation flag */
int nsys;          /* Number of customers in system */
int narr;          /* Number of Arrivals */
int ncom;          /* Nuber of Completions */
double Busy;       /* Busy time */
double Area_n;     /* Integral of the function nsys(t)  = Area_w*/
double Sum_w;     /* Sum of the waiting times of the arrived customers = Area_n*/

int event_counter; /* Number of the events processed by the simulkator*/

double stime;       /* Clock of the simulator - Simulation time */

double arrival_t;   /* time of arrival read from trace */
double service_t;   /* service time read from trace */

double sum_interArrival_t_square;
double sum_interArrival_t;
double sum_service;
double sum_service_square;
double Sum_w_square;
double service_variance;
double interArrival_variance;
double waiting_variance;
double sum_queue;
double sum_queue_square;
double sum_sys_square;

double q_size;
double queue_variance;
double Area_q;
double End_time = 1000000.00;
int node_number;

double arrive_time[NUM_OF_CUSTOMERS+1];
double departure_time[NUM_OF_CUSTOMERS+1];
double service_time[NUM_OF_CUSTOMERS+1];
double delay_time[NUM_OF_CUSTOMERS];
double waiting_time[NUM_OF_CUSTOMERS];

double mean;
double variance;
double CV;
double CV_interArrival=0.0;
double CV_service=0.0;
double Expected_Customers=0.0;
double rho = 0.0;

double Exponential(double m)
{
    return (-m * log(1.0 - Random()));
}


double Uniform(double a, double b)
{
    return (a + (b - a) * Random());
}


double GetArrival(void)
{
    static double arrival = START;
    arrival += Exponential(50.0);
    return (arrival);
}

double GetService(void)
{
    return (Exponential(40.0));
}
double GetServiceUniform(void)
{
    return (Uniform(A, B));
}

double GetMean(double a, double b){
    return (a+b)/2;
}

double Variance(double a, double b){
    return ((b-a)*(b-a))/12;
}

double GetCoefficientofVariation(double mean, double variance){
    return sqrt(variance/(mean * mean));
}

double GetServiceProbable(){
    double p = Uniform(0,1);
    if(p>0.05){
        return Uniform(0.99,1.01);
    }
    else{
        return Uniform(773.19,788.81);
    }
}

int main(int argc, char* argv[]){
    /*
    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open input file %s\n", FILENAME);
        return (1);
    }
     Simulation Run */
    //if (!feof(fp))
    {
        simulate();
        getchar();
        return 0;
    }
}

void simulate() {
    /* Simulation core */
    initialize();
    while (!(halt))
        engine();
    report();
}



void engine(void){
    int event_type;
    double  oldtime;
    double  interval;
    tree new_event;
    
    /* Get the first event notice from Future Event List */
    //new_event = event_pop(&root);
    new_event = event_pop();
    
    /* update clock */
    oldtime = stime;
    stime = new_event->event.occur_time;
    if (stime == End_time)
        stime = oldtime;
    interval = stime - oldtime;
    
    /* Collect statistics */
    if (nsys > 0){
        Busy = Busy + interval;
        Area_n = Area_n + nsys * interval;
        Area_q = Area_q + (nsys-1) * interval;
        sum_queue_square += (nsys-1) * (nsys-1) * interval;
        sum_sys_square += nsys * nsys * interval;;
    }
    
    /* Identify and process current event */
    event_type = new_event->event.type;
    switch(event_type){
        case ARRIVAL :
            if(nsys == 0 && stime>=End_time){
                halt = true;
                break;
            }
            arrival(new_event);
            break;
        case DEPARTURE :
            /*
            if(nsys == 0 && stime>=End_time){
                halt = true;
                break;
            }
             */
            departure(new_event);
            break;
        case CONTINUE:
            break;
        case END : halt = true;
            break;
    }
    event_counter++;
}



void initialize(){
    struct node* first_notice;
    //double interval;
    int i;
    /* Control Settings  */
    halt = false;
    
    /* Basic Statistic Measures  */
    nsys   = 0;
    narr   = 0;
    ncom   = 0;
    Busy   = 0;
    Area_n = 0;
    Sum_w = 0;
    stime   = 0;
    Area_q = 0;
    
    sum_interArrival_t_square = 0;
    sum_interArrival_t = 0;
    sum_service = 0;
    sum_service_square = 0;
    Sum_w_square = 0;
    sum_queue = 0;
    sum_queue_square = 0;
    sum_sys_square = 0;
    
    q_size = 0;
    queue_variance = 0;
    
    for(i=0;i<NUM_OF_CUSTOMERS;i++){
        arrive_time[i]=0;
        service_time[i]=0;
        waiting_time[i]=0;
        departure_time[i]=0;
        delay_time[i]=0;
        
    }
    /* Basic  counters  */
    job_number = 0;
    event_counter = 0;
    node_number = 0;
    
    /* Future Event List and additional structures*/
    root  = NULL;
    queue = NULL;
    last  = NULL;
    
    mean = GetMean(A,B);
    variance = Variance(A,B);
    CV = GetCoefficientofVariation(mean,variance);
    
    /* Get first Arrival and Service Times pair*/
    arrival_t = 0.0;//GetArrival();
    service_t = GetService();
    //service_t = GetServiceUniform();
    //service_t = GetServiceProbable();
    q = createQueue();
    //if (!feof(fp))
    {
        
        /* Initialize Event notice of first arrival and Schedule first event */
        first_notice = get_new_node();
        first_notice->event.arrival_time = stime;
        first_notice->event.service_time = service_t;
        first_notice->event.occur_time = arrival_t;
        first_notice->event.type = ARRIVAL;
        first_notice->next = NULL;
        //first_notice->prev = NULL;
        sprintf(first_notice->event.name, "J%d", (job_number++));
        arrive_time[job_number] = arrival_t;
        service_time[job_number] = service_t;
        
        schedule(first_notice);
        
        // Initialize Event notice of End Simulation and Schedule last event
        first_notice = get_new_node();
        first_notice->event.arrival_time = stime;
        first_notice->event.occur_time = End_time;
        first_notice->event.type = END;
        first_notice->next = NULL;
        //first_notice->prev = NULL;
        sprintf(first_notice->event.name, "END");
        schedule(first_notice);
        
    }
}



void arrival(struct node* node_event){
    /* manage arrival event */
    
    //double delta_t, cycle_time;
    struct node* next_job;
    
    /* Update statistics */
    nsys++;
    narr++;

    node_event->event.arrival_time = stime;
    if (nsys == 1) {
        /* Process arrival at empty server */
        node_event->event.type = DEPARTURE;
        node_event->event.occur_time = stime+node_event->event.service_time;
        schedule(node_event);
    }
    else {
        /* Process arrival at busy server */
        enqueue(node_event);
        q_size = getQueue_size();
    }
    //if (!feof(fp))
    {
        /* get new pair from trace */
        arrival_t = GetArrival();
        service_t = GetService();
        //service_t = GetServiceUniform();
        //service_t = GetServiceProbable();
        
        /* schedule new arrival */
        next_job = get_new_node();
        next_job->event.service_time = service_t;
        next_job->event.arrival_time = stime;
        next_job->event.occur_time = arrival_t;
        next_job->event.type = ARRIVAL;
        next_job->next = NULL;
        //next_job->prev = NULL;
        sprintf(next_job->event.name, "J%d", (job_number++));
        arrive_time[job_number] = arrival_t;
        service_time[job_number] = service_t;
        schedule(next_job);
    }
}

void departure(struct node* node_event){
    /* manage departure event */
    double waiting_time;
    double waiting_time_square;
    
    struct node* next_job;
    
    /* Update statistics */
    nsys--;
    ncom++;
    if(nsys==0){
        
    }
    waiting_time = stime-node_event->event.arrival_time;
    waiting_time_square = waiting_time * waiting_time;
    
    Sum_w = Sum_w + waiting_time;
    Sum_w_square = Sum_w_square + waiting_time_square;
    
    if (nsys > 0) {
        /* Process departure from a server with a queue*/
        next_job = dequeue();
        q_size = getQueue_size();

        next_job->event.type = DEPARTURE;
        next_job->event.occur_time = stime+next_job->event.service_time;
        schedule(next_job);
    }
}


void print_results(){
    
    printf("\nSimulation Results:");
    printf("\n---------------------------------");
    printf("\nNumber of considered events                 = %d", event_counter);
    printf("\nArrival rate                                = %10.6f", (narr/stime));
    printf("\nService rate                                = %10.6f", (ncom/Busy));
    printf("\nUtilization                                 = %10.6f", (Busy/stime));
    printf("\nThroughput                                  = %10.6f", (ncom/stime));
    printf("\nAverage Number of Customers in the Server   = %10.6f", (Area_n/stime));
    printf("\nExpected Number of Customers in the Server   = %10.6f", (Expected_Customers));
    printf("\nAverage Waiting Time in the Server          = %10.6f", (Sum_w/narr));
    printf("\n--------------------------------------\n\n");
    printf("\n ___ Statistics on simulation run ___");
    printf("\n--------------------------------------");
    //printf("\nNumber of new generated nodes = %d", node_number);
    printf("\n---------------------------------");
    printf("\nNumber of processed events    = %d\n", event_counter);
    printf("\n---------------------------------");
    printf("\nLast Simulation Time          = %10.6f\n", stime);

    printf("\nTotal arrivals in the system          = %d", narr);
    
    printf("\nAverage Service Time          = %10.6f", (sum_service/narr));
    printf("\nVariance of Service Time         = %10.6f", (service_variance));
    printf("\nAverage Waiting Time         = %10.6f", (Sum_w/narr));
    printf("\nVariance of Waiting Time         = %10.6f", (waiting_variance));
    printf("\nAverage InterArrival Time         = %10.6f", (sum_interArrival_t/narr));
    printf("\nVariance of InterArrival Time         = %10.6f", (interArrival_variance));
    printf("\nAverage Number of Customers in Queue          = %10.6f", (Area_q/stime));
    printf("\nVariance of Customers in Queue        = %10.6f", (queue_variance));
    printf("\nCoefficient of Variation of Service Time       = %10.6f", (CV_service));
    printf("\nCoefficient of Variation of InterArrival Time       = %10.6f", (CV_interArrival));
    printf("\n______\n");
}

void report(){
    
    
    int i = 0;
    float value = 0.0;
    
    for(i=0;i<narr;i++){

        value = arrive_time[i+1] - arrive_time[i];
        sum_interArrival_t +=value;
        sum_interArrival_t_square += (value * value);
    }
    
    for(i=1;i<=narr;i++){
        
        sum_service += service_time[i];
        sum_service_square += (service_time[i] * service_time[i]);
    }
    
    interArrival_variance = getVariance(sum_interArrival_t_square, sum_interArrival_t);
    
    service_variance = getVariance(sum_service_square, sum_service);
    waiting_variance = getVariance(Sum_w_square, Sum_w);
    queue_variance =  (sum_queue_square/stime) - ((Area_q/stime) * (Area_q/stime));
    
    CV_service = sqrt(service_variance/((sum_service/narr)*(sum_service/narr)));
    CV_interArrival = sqrt(interArrival_variance/((sum_interArrival_t/narr)*(sum_interArrival_t/narr)));
    
    rho = (narr/stime) / (ncom/Busy);
    Expected_Customers = rho/(1-rho);
    
    
    print_results();
    
    destroy_list();
    destroy_queue();
}

double getVariance(double sum_square, double sum)
{
    double variance =0;
    variance = (sum_square / (NUM_OF_CUSTOMERS-1)) - ((sum * sum) / (NUM_OF_CUSTOMERS * (NUM_OF_CUSTOMERS-1)));
    return variance;
    
}
// A utility function to create a new linked list node.
struct QNode* newNode(struct node* k)
{
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue *createQueue()
{
    struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enqueue(struct node* k)
{
    // Create a new LL node
    struct QNode *temp = newNode(k);
    
    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }
    
    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a key from given queue q
struct node *dequeue()
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return NULL;
    
    // Store previous front and move front one node ahead
    struct QNode *temp = q->front;
    q->front = q->front->next;
    
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;
    return temp->key;
}

void destroy_queue()
{
    free(q);
}

void destroy_list()
{
    free(head);
}

//function to pop event from list

struct node* event_pop()
{
    struct node* n;
    /*
    if(head->next == NULL)
    {
        n = head;
        head = NULL;
        return n;
    }
     */
    n = head;
    head = head->next;
    return n;
}

// function to insert a new_node in a list.
void schedule(struct node* new_node)
{
    struct node* current;
    /* Special case for the head end */
    if (head == NULL || head->event.occur_time >= new_node->event.occur_time)
    {
        new_node->next = head;
        head = new_node;
    }
    else
    {
        /* Locate the node before the point of insertion */
        current = head;
        while (current->next!=NULL &&
               current->next->event.occur_time < new_node->event.occur_time)
        {
            current = current->next;
        }
        new_node->next = current->next;
        current->next = new_node;
    }
}


// A function to create a new node
struct node* get_new_node()
{
    /* allocate node */
    struct node* new_node =
    (struct node*) malloc(sizeof(struct node));
    
    /* put in the data  */
    new_node->next =  NULL;
    
    return new_node;
}


double getQueue_size(){
    double i=0.0;
    struct QNode *temp = q->front;
    while(temp!=NULL){
        i++;
        temp= temp->next;
    }
    return i;
    
}
