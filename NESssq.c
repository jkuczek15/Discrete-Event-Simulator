#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "NESssq.h"
#include "rngs.h"

//#define DBG
//#define DBG1
//#define DBG2

bool halt;      /* End of simulation flag */
int nsys;          /* Number of customers in system */
int narr;          /* Number of Arrivals */
int ncom;          /* Nuber of Completions */
int node_number;
int event_counter; /* Number of the events processed by the simulkator*/

double Busy;       /* Busy time */
double Area_n;     /* Integral of the function nsys(t)  = Area_w*/
double Sum_w;     /* Sum of the waiting times of the arrived customers = Area_n*/
double stime;       /* Clock of the simulator - Simulation time */
double arrival_t;   /* time of arrival read from trace */
double service_t;   /* service time read from trace */

double sum_service = 0;
double q_size;
double Area_q;
double End_time = SIMULATON_END;
double Expected_Customers=0.0;
double rho = 0.0;

int nRepairStation = 0;
bool repairmanResting = false;
long seed1 = DEFAULT;
long seed2 = DEFAULT;
long seed3 = DEFAULT;
long seed4 = DEFAULT;
long long newSeed = 0;
double Area_repair = 0.0;
double avgRepairTime = 0.0;
double sum = 0.0;
double sum_square = 0.0;
double std_deviation = 0.0;
double variance = 0.0;
double confidence_interval = 0.0;
double lowerBound = 0.0;
double upperBound = 0.0;

long double sumAreaTimesInterval = 0.0;
double sumInterval = 0.0;
long double sumRegArea_square = 0.0;
long double sumInterval_square = 0.0;
double sumRegArea = 0.0;
double mean = 0.0;

double old_regTime = 0.0;
double regTime = 0.0;
double regArea = 0.0;
double regInterval = 0.0;
double pointEstimate = 0.0;
bool startRegenrationCycle = false;
double confidenceRange = 0.0;
double range = 0.0;
double nRegeneration = 0.0;
double rHat = 0.0;




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
    double val;
    SelectStream(1);
    val = Exponential(300.0);
#ifdef DBG1
    printf("Arrival value: %6.2f\n",val);
#endif
    return val;
}

double GetService(void)
{
    double val;
    SelectStream(2);
    val = Exponential(50.0);
#ifdef DBG1
    printf("Service value: %6.2f\n",val);
#endif
    return val;
}

bool isRepairmanResting(){

    SelectStream(3);
    double p = Uniform(0,1);
#ifdef DBG1
    printf("Decision probability value: %6.2f\n",p);
#endif
    if(p<PROBABILITY){
        return true;
    }
    else
        return false;
}

double GetRestTime(){
    double val;
    SelectStream(4);
    val = Exponential(10.0);
#ifdef DBG1
    printf("Resting value: %6.2f\n",val);
#endif
    return val;
}

double getSampleVariance(double sum_square, double sum)
{
    double variance =0;
    variance = (sum_square / (nRegeneration-1)) - ((sum * sum) / (nRegeneration * (nRegeneration-1)));
    return variance;
}

void initializeRegenrationValues()
{
    old_regTime =  regTime;
    regTime = stime;
    regInterval = 0;
    
    sumRegArea = 0;
    sumRegArea_square = 0;

    sumInterval = 0;
    sumInterval_square = 0;
    
    sumAreaTimesInterval = 0;
}

void getRegenrationValues()
{
    old_regTime =  regTime;
    regTime = stime;
    regInterval = regTime - old_regTime;
    
    sumRegArea += regArea;
    sumRegArea_square += regArea * regArea;
    
    sumInterval += regInterval;
    sumInterval_square += regInterval * regInterval;

    sumAreaTimesInterval += regArea * regInterval;
}

double getConfidenceIntervalLowerRegeneration(double mean, double delta){
    double val;
    val = mean - (1.96 * delta);
    return val;
}

double getConfidenceIntervalUpperRegeneration(double mean, double delta){
    double val;
    val = mean + (1.96 * delta);
    return val;
}

//function to get mean value
double getrHat(){
    rHat = sumRegArea/sumInterval;
    return rHat;
}

//function to get delta
double getDelta(){
    double val1,val2,val3,val4,val5,val6;
    val1 = sqrt(nRegeneration/(nRegeneration-1));
    val2 = rHat * sumAreaTimesInterval;
    val3 = rHat * rHat * sumInterval_square;
    val4 = sqrt(sumRegArea_square - (2 * val2) + val3);
    val5 = val4 / sumInterval;
    val6 =  val1 * val5;
    return val6;
}

double getConfidenceRange(){
    double delta = 0.0;

    mean = getrHat();
    delta = getDelta();

    lowerBound = getConfidenceIntervalLowerRegeneration(mean,delta);
    upperBound = getConfidenceIntervalUpperRegeneration(mean,delta);
    range = upperBound - lowerBound;
    return range;
}
int main(int argc, char* argv[]){

    PlantSeeds(DEFAULT);
    simulate();
    report();
    //getchar();
    return 0;
}

void simulate() {
    /* Simulation core */
    initialize();
    while (!(halt))
        engine();
    //report();
}

void engine(void){
    int event_type;
    double  oldtime;
    double  interval;
    tree new_event;
    
    /* Get the first event notice from Future Event List */
    new_event = event_pop();
    
    /* update clock */
    oldtime = stime;
    stime = new_event->event.occur_time;
    interval = stime - oldtime;
    q_size = getQueue_size();
    /* Collect statistics */
    Area_n = Area_n + nsys * interval;
    Area_q = Area_q + (q_size) * interval;
    Area_repair = Area_repair + (nRepairStation) * interval;
    
    if(startRegenrationCycle == true){
        regArea += (nRepairStation) * interval;
    }
    /* Identify and process current event */
    event_type = new_event->event.type;
    switch(event_type){
        case ARRIVAL :
            arrival(new_event);
            break;
        case DEPARTURE :
            departure(new_event);
            break;
        case REST:
            rest(new_event);
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
    Sum_w  = 0;
    stime  = 0;
    Area_q = 0;
    
    q_size          = 0;
    /* Basic  counters  */
    job_number      = 0;
    node_number     = 0;
    nRepairStation  = 0;
    Area_repair     = 0.0;
    repairmanResting= false;
    avgRepairTime   = 0;
    
    /* Future Event List and additional structures*/
    root  = NULL;
    queue = NULL;
    last  = NULL;
    
    q = createQueue();
        
    /* Initialize Event notice of first arrival and Schedule first event */
    for(i=0;i<NUMOFMACHINES;i++){
        nsys++;
        arrival_t = GetArrival();
        service_t = GetService();
        first_notice = get_new_node();
        first_notice->event.arrival_time = arrival_t;
        first_notice->event.service_time = service_t;
        first_notice->event.occur_time = stime + arrival_t;
        first_notice->event.type = ARRIVAL;
        first_notice->next = NULL;
        sprintf(first_notice->event.name, "J%d", (job_number++));
        schedule(first_notice);
    }
 
    // Initialize Event notice of End Simulation and Schedule last event
    first_notice = get_new_node();
    arrival_t = stime;
    first_notice->event.arrival_time = arrival_t;
    first_notice->event.occur_time = End_time;
    first_notice->event.type = END;
    first_notice->next = NULL;
    sprintf(first_notice->event.name, "END");
    schedule(first_notice);
}

void arrival(struct node* node_event){
    /* Update statistics */
    nsys--;
    narr++;
    nRepairStation++;
    /* manage arrival event */
    
    if (nRepairStation == 1 && repairmanResting == false) {
        /* Process arrival at empty server */
        node_event->event.type = DEPARTURE; //departure from repair station
        node_event->event.occur_time = stime+node_event->event.service_time;  //service time = repair time
        sum_service += node_event->event.service_time;
        schedule(node_event);
    }
    else {
        /* Process arrival at busy server */
        enqueue(node_event);
    }
#ifdef DBG
    printEvent(node_event);
    printTopQueueEvent();
#endif
    q_size = getQueue_size();
    if(q_size == 3){
        nRegeneration++;
        if(startRegenrationCycle == false){
            startRegenrationCycle = true;
            initializeRegenrationValues();
        }
        else{
            getRegenrationValues();
        }
        regArea = 0;
    }
    
    if(nRegeneration > 30){
#ifdef DBG1
        printFutureEventList();
        printRepairQueue();
#endif
        confidenceRange = getConfidenceRange();
        if(confidenceRange < (0.1 * mean))
            halt = true;
    }
}

void departure(struct node* node_event){
    /* manage departure event */
    double restTime = 0.0;
    struct node* rest_node;
    struct node* next_job;
    
    /* Update statistics */
    nsys++;
    ncom++;
    nRepairStation--;
    
    //Machine Repaired
    /* get new pair from trace */
    arrival_t = GetArrival(); //arrival time = failure time
    service_t = GetService(); //service time = repair time
    
    /* schedule new arrival */
    node_event->event.service_time = service_t;
    node_event->event.arrival_time = arrival_t;
    node_event->event.occur_time = stime + arrival_t;
    node_event->event.type = ARRIVAL;
    node_event->next = NULL;

    schedule(node_event);
    if(isRepairmanResting() == true){
        repairmanResting = true;
        restTime  = GetRestTime();
        sum_service += restTime;
        rest_node = get_new_node();
        rest_node->event.occur_time = stime + restTime;
        rest_node->event.type = REST;
        rest_node->next = NULL;
        schedule(rest_node);
    }
    else{
        if (q_size > 0) {
            /* Process departure from a server with a queue*/
            next_job = dequeue();
            q_size = getQueue_size();
            next_job->event.type = DEPARTURE;
            next_job->event.occur_time = stime+next_job->event.service_time;
            sum_service += next_job->event.service_time;
            schedule(next_job);
        }
    }
}

void rest(struct node* node_event){
    /* manage departure event */
    node_event = NULL;
    free(node_event);
    struct node* next_job;

    //Repairman Rested
    repairmanResting = false;
    q_size = getQueue_size();
    
    if (q_size > 0) {
        /* Process departure from a server with a queue*/
        next_job = dequeue();
        q_size = getQueue_size();
        next_job->event.type = DEPARTURE;
        next_job->event.occur_time = stime + next_job->event.service_time;
        sum_service += next_job->event.service_time;
        schedule(next_job);
    }
}

void print_results(){
    
    printf("\nSimulation Results:");
    printf("\n---------------------------------");
    printf("\nNumber of considered events                 = %10d", event_counter);
    printf("\nLast Simulation Time                        = %10.2f\n", stime);
#ifdef DBG2
    //printf("\nUtilization                                 = %10.2f", (sum_service/stime));
    //printf("\nThroughput                                  = %10.4f", (ncom/stime));
    //printf("\nAverage Number of Machines in the Server    = %10.2f", (Area_n/stime));
    //printf("\nAverage Number of Machines at RepairStation = %10.2f", (Area_repair/stime));
    //printf("\nAverage time spent by Machines at shop      = %10.2f", avgRepairTime);
#endif
    printf("\n--------------------------------------\n");
    printf("\nSample Mean                                 = %10.2f", mean);
#ifdef DBG2
    //printf("\nSample Standard Deviation                   = %10.2f", delta);
    //printf("\nSample Variance                             = %10.2f", variance);
#endif
    printf("\nRegeration Point is equal to Queue size     = %10d", QSIZE);
    printf("\nConfidence Interval with 95 pcnt confidence = %10.2f - %2.2f", lowerBound,upperBound);
    printf("\nConfidence Range                            = %10.2f", confidenceRange);
    printf("\nNumber of Regenration                       = %10.2f", nRegeneration);


    printf("\n--------------------------------------\n\n");
#ifdef DBG2
    printf("\n ___ Statistics on simulation run ___");
    printf("\n---------------------------------");
    printf("\nNumber of processed events                  = %10d\n", event_counter);
    printf("\nLast Simulation Time                        = %10.2f\n", stime);
    printf("\nAverage Number of Customers in Queue        = %10.2f", (Area_q/stime));
    printf("\n______\n");
#endif
}

void report(){
    print_results();
    destroy_list();
    destroy_queue();
}

double getVariance(double sum_square, double sum)
{
    double variance =0;
    variance = (sum_square / (narr-1)) - ((sum * sum) / (narr * (narr-1)));
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

//A function to get the queue size
double getQueue_size(){
    double i=0.0;
    struct QNode *temp = q->front;
    while(temp!=NULL){
        i++;
        temp= temp->next;
    }
    return i;
}

void printFutureEventList(){
    struct node* node_event;
    node_event = head;
    printf("\nEvent List:\n");
    printf("\nType\t\tName\tOccur Time\tService Time\tArrival Time\n");
    
    while(node_event!=NULL){
        printf("%4d\t\t%s\t\t\t%.2f\t\t%.2f\t\t%.2f\n",node_event->event.type,node_event->event.name,node_event->event.occur_time,node_event->event.service_time,node_event->event.arrival_time);
        
        node_event = node_event->next;
    }
}

void printEvent(struct node* node_event){
    printf("\nEvent\n");
    printf("\nEvent Type        : %10d",node_event->event.type);
    printf("\nEvent Name        : %10s",node_event->event.name);
    printf("\nEvent Occur Time  : %10.2f",node_event->event.occur_time);
    printf("\nEvent Service Time: %10.2f",node_event->event.service_time);
    printf("\nEvent Arrival Time: %10.2f\n",node_event->event.arrival_time);
}

void printRepairQueue(){
    struct QNode *queue_event = q->front;
    printf("\nQueue List:\n");
    printf("\nType\t\tName\tOccur Time\tService Time\tArrival Time\n");
    
    while(queue_event!=NULL){
        printf("%4d\t\t%s\t\t\t%.2f\t\t%.2f\t\t%.2f\n",queue_event->key->event.type,queue_event->key->event.name,queue_event->key->event.occur_time,queue_event->key->event.service_time,queue_event->key->event.arrival_time);
        queue_event = queue_event->next;
    }
}

void printTopQueueEvent(){
    if (q->front != NULL){
        printf("\nQueue Event\n");
        printf("\nEvent Type        : %10d",q->front->key->event.type);
        printf("\nEvent Name        : %10s",q->front->key->event.name);
        printf("\nEvent Occur Time  : %10.2f",q->front->key->event.occur_time);
        printf("\nEvent Service Time: %10.2f",q->front->key->event.service_time);
        printf("\nEvent Arrival Time: %10.2f\n",q->front->key->event.arrival_time);
    }
}
