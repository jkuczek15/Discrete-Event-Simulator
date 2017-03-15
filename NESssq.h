
#define ARRIVAL 0
#define DEPARTURE 1
#define END 2
#define CONTINUE 3

#define FILENAME   "ssq1.DAT"                  /* input data file */
FILE   *fp;

#define EVENT_NUMBER 50
#define NUM_OF_CUSTOMERS 1000000
#define LAST         10000L                   /* number of jobs processed */
#define START        0.0                      /* initial time             */

#define A 36.0
#define B 44.0

typedef struct node* tree;
typedef struct node* list;

int node_number;
tree root;         /* Pointer to the root of the Future Event List implemented with a tree structure */
list queue;
list last;
int job_number;    /* (progressive) Job identification number */


typedef struct {
    int type;
    char name[256];
    double service_time;
    double occur_time;
    double arrival_time;
} event_notice;


struct node{
    event_notice event;
    struct node* next;	//Nel caso della doppia linkatura corrisponde al prev
    //struct node* prev; //Nel caso di una lista, corrisponde al next
};

//queue



//this link always point to first Link
struct node *head = NULL;

//this link always point to last Link
struct node *last = NULL;

struct node *current = NULL;


// A linked list (LL) node to store a queue entry
struct Queue *q;

struct QNode
{
    struct node* key;
    struct QNode *next;
};

// The queue, front stores the front node of LL and rear stores ths
// last node of LL
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
void report(void);
void print_results();
//double GetArrival(FILE *fp);
//double GetService(FILE *fp);
double GetArrival(void);
double GetService(void);
void schedule(struct node* newevent) ;
struct node* event_pop();
struct node* get_new_node();

void destroy_list();
void destroy_queue();
double getQueue_size();
double getVariance(double sum_square, double sum);

