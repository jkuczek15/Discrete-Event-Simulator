#define ARRIVAL 0
#define DEPARTURE 1
#define REST 2
#define END 3

#define NUMOFMACHINES 10
#define PROBABILITY 0.3
#define SIMULATON_END 20000000
#define DEFAULT    123456789L  /* initial seed, use 0 < DEFAULT < MODULUS   */
#define NUM_OF_RUN 50
#define QSIZE 8

typedef struct node* tree;
typedef struct node* list;

int node_number;
tree root;         /* Pointer to the root of the Future Event List implemented with a tree structure */
list queue;
list last;
int job_number;    /* (progressive) Job identification number */

typedef struct {
    int index;
    int type;
    char name[256];
    double service_time;
    double occur_time;
    double arrival_time;
} event_notice;

struct node{
    event_notice event;
    struct node* next;
};

//queue Data Structure

//this link always point to first Link
struct node *head = NULL;
//this link always point to last Link
struct node *last = NULL;
//this link will point to current Link
struct node *current = NULL;

// A linked list (LL) node to store a queue entry
struct Queue *q;

struct QNode
{
    struct node* key;
    struct QNode *next;
};

// The queue, front stores the front node of LinkedList and rear stores ths
// last node of LinkedList
struct Queue
{
    struct QNode *front, *rear;
};

struct QNode* newNode(struct node* k);
struct Queue *createQueue();
void enqueue(struct node* k);
struct node *dequeue();

void simulate(void);
void initialize(void);
void engine(void);
void arrival(struct node*);
void departure(struct node*);
void rest(struct node* node_event);
void report(void);
void print_results();
double GetArrival(void);
double GetService(void);
void schedule(struct node* newevent) ;

struct node* event_pop();
struct node* get_new_node();
void destroy_list();
void destroy_queue();
double getQueue_size();
void printTopQueueEvent();
void printRepairQueue();
void printEvent(struct node* node_event);
void printFutureEventList();
