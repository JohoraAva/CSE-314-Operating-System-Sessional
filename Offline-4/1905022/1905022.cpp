#include<bits/stdc++.h>
#include<pthread.h>
#include<semaphore.h>
#include <unistd.h>
using namespace std;

#define TOTAL_PRINTER 4
#define BINDING_STATION 2
#define TOTAL_STAFF 2

int total_student,group_size,total_group;
int w,x,y;
bool isAllDone=false;



random_device rd;
mt19937 gen(rd()); // Mersenne Twister engine is commonly used

// Mean value for the Poisson distribution
double lambda = 3.0;

// Create the Poisson distribution object
poisson_distribution<int> distribution(lambda);
time_t start_time,cur_time;

pthread_t *student_threads;
pthread_t staff[2];


sem_t binding;
sem_t sem_staff;
pthread_mutex_t submission;
pthread_mutex_t output_mutex,printing_mutex;
pthread_mutex_t *sem_std;

//submission , reader-writer
int rc=0;
int total_submit=0;
ofstream outputFile;


void* printThread(void* std_id);

//printer state
bool isBusy[TOTAL_PRINTER]; //is printer busy?


void init_semaphore()
{
	sem_init(&binding,0,BINDING_STATION);
	pthread_mutex_init(&submission,0);
    pthread_mutex_init(&printing_mutex,0);
    //printer state init
    for(int i=0;i<TOTAL_PRINTER;i++)
    {
        isBusy[i]=false;
    }
    sem_std=new pthread_mutex_t[total_student];
    sem_init(&sem_staff,0,TOTAL_STAFF);

    for(int i=0;i<total_student;i++)
    {
        pthread_mutex_init(&sem_std[i],0);
    }
}

int get_current_time()
{
    time(&cur_time);

    double time_taken = double(cur_time - start_time);
	return (int) time_taken;
}

int group_no(int id)
{
    return id/group_size; //0 indexed
}

struct Group{
    int *grp_members; //id set
    int *state; //0 not working, 1 trying, 2 working    
    int no_of_submission;
    int leader;
    pthread_mutex_t submission_mutex;

    void init_std_id(int grp_no)
    {
        for(int i=0;i<group_size;i++)
        {
            grp_members[i]=grp_no*group_size+i; //0 indexed
            state[i]=0; //not working
        }
        pthread_mutex_init(&submission_mutex,0);
    }
    bool isDonePrinting()
    {
        return no_of_submission==group_size;
    }
    void print_thread(int start_id)
    {
        int* id;
        for(int i=start_id;i<start_id+group_size;i++)
        {
            id=new int(i);
            sleep(distribution(gen));//random time
            pthread_create(&student_threads[i],NULL,printThread,(void*)(id));       
        }

    }
    void endOfPrinting()
    {
        pthread_mutex_lock(&submission_mutex);
        no_of_submission++;
        pthread_mutex_unlock(&submission_mutex);
    }
    void bind()
    {
        pthread_mutex_lock(&output_mutex);
          //cout<<"Group "<<group_no(leader)+1<<" has started binding at time "<<get_current_time()<<endl;
        outputFile<<"Group "<<group_no(leader)+1<<" has started binding at time "<<get_current_time()<<endl;
        pthread_mutex_unlock(&output_mutex);
        sem_wait(&binding);
       
        sleep(x);

        sem_post(&binding);

        pthread_mutex_lock(&output_mutex);
          //cout<<"Group "<<group_no(leader)+1<<" has finished binding at time "<<get_current_time()<<endl;
        outputFile<<"Group "<<group_no(leader)+1<<" has finished binding at time "<<get_current_time()<<endl;
        pthread_mutex_unlock(&output_mutex);
    }

    void submit()
    {
       
        pthread_mutex_lock(&submission);
        total_submit++;
        pthread_mutex_unlock(&submission);

        pthread_mutex_lock(&output_mutex);
          //cout<<"Group "<<group_no(leader)+1<<" has submitted the report at time "<<get_current_time()<<endl;
        outputFile<<"Group "<<group_no(leader)+1<<" has submitted the report at time "<<get_current_time()<<endl;
        pthread_mutex_unlock(&output_mutex);

    }
};
struct Group *groups;

void test(int id,int printer_id)
{
    int grp_no=group_no(id);
    int member_no=(id)%group_size;

    if( groups[grp_no].state[member_no]==1 && isBusy[printer_id]==false)
    {
        groups[grp_no].state[member_no]=2; //working state 
        isBusy[printer_id]=true;
        pthread_mutex_unlock(&sem_std[id]);
    }
}


void enterPrintingRegion(int id,int printer_id)
{
    pthread_mutex_lock(&printing_mutex);
    int grp_no=group_no(id);
    int member_no=(id)%group_size;

    pthread_mutex_lock(&output_mutex);
      //cout<<"Student "<<id+1<<" has arrived at the print station at time "<<get_current_time()<<endl;//" at printer = "<<printer_id+1<<endl;
    outputFile<<"Student "<<id+1<<" has arrived at the print station at time "<<get_current_time()<<endl;//" at printer = "<<printer_id+1<<endl;
    pthread_mutex_unlock(&output_mutex);

    groups[grp_no].state[member_no]=1; //onGoing... state



    test(id,printer_id);
   

    pthread_mutex_unlock(&printing_mutex);

    pthread_mutex_lock(&sem_std[id]);

}

void leavePrintingRegion(int id, int printer_id)
{
    pthread_mutex_lock(&printing_mutex);
    int grp_no=group_no(id);
    int member_no=(id)%group_size;

    groups[grp_no].state[member_no]=0; //not working state
    isBusy[printer_id]=false;

    int start_id=grp_no*group_size;

    for(int i=start_id;i<start_id+group_size;i++)
    {
        test(i,(i%4));
    }

    for(int i=0;i<total_student;i++)
    {
        if(i<start_id || i>= start_id+group_size)
            test(i,(i%4));
    }


    pthread_mutex_lock(&output_mutex);
      //cout<<"Student "<<id+1<<" has finished printing at time "<<get_current_time()<<endl;
    outputFile<<"Student "<<id+1<<" has finished printing at time "<<get_current_time()<<endl;
    pthread_mutex_unlock(&output_mutex);

    pthread_mutex_unlock(&printing_mutex);

    
}
void* printThread(void* std_id)
{
    int* id=(int*) std_id;
    int printer_id=(*id%4); //0 indexed


    enterPrintingRegion(*id,printer_id);

    sleep(w);

   leavePrintingRegion(*id,printer_id);


    groups[group_no(*id)].endOfPrinting();
    if(groups[group_no(*id)].isDonePrinting())
    {
        groups[group_no(*id)].bind();
        groups[group_no(*id)].submit();
    }


   return nullptr;


}
void* reader(int staff_no)
{
    int id=staff_no;

    sem_wait(&sem_staff);
    rc++;
    if(rc==1)
    {
        pthread_mutex_lock(&submission);
    }
    sleep(y);

    sem_post(&sem_staff);

    pthread_mutex_lock(&output_mutex);
      //cout<<"Staff "<<id<<" has started reading the entry book at time "<<get_current_time()<<". No. of submission = "<<total_submit<<endl;
    outputFile<<"Staff "<<id<<" has started reading the entry book at time "<<get_current_time()<<". No. of submission = "<<total_submit<<endl;
    pthread_mutex_unlock(&output_mutex);

    sem_wait(&sem_staff);
    rc--;
    if(rc==0)
        pthread_mutex_unlock(&submission);
            
    sem_post(&sem_staff);

    if(total_submit==total_group)
    {
        isAllDone=true;
        // all group has submitted their assignment
    }
    return nullptr;
}
void* readThread(void* n)
{
    int* id=(int*) n;
   while(1)
   {
        sleep(distribution(gen));
        reader(*id);
   }

   return nullptr;
}

int main()
{

    time(&start_time);


    ifstream inputFile("input.txt");
   // ofstream outputFile("output.txt");
    outputFile.open("output.txt",ios::out | ios::trunc);
    inputFile>>total_student>>group_size>>w>>x>>y;
    inputFile.close();

    total_group=total_student/group_size;
    student_threads=new pthread_t[total_student];

    init_semaphore();

    groups=new Group[total_group];

   
    for(int i=0;i<total_group;i++)
    {
        groups[i].grp_members=new int[group_size];
        groups[i].state=new int[group_size];
        groups[i].init_std_id(i);
        groups[i].no_of_submission=0;
        groups[i].leader=(i+1)*group_size-1; //0 indexed

    }

    int staff_id[2]={1,2};
    pthread_create(&staff[0],NULL,readThread,(void*)&staff_id[0]); 
    pthread_create(&staff[1],NULL,readThread,(void*)&staff_id[1]); 

    for(int i=0;i<total_group;i++)
    {
        groups[i].print_thread(i*group_size);
    }

     while(!isAllDone);

    outputFile.close();
    delete[] student_threads;
    delete[] groups;
    delete[] sem_std;


}
