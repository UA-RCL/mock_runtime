#include <vector>
#include <string>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <deque>
#include "dash.h"

#define MAX_ARGS 15
#define ENABLE_LOGGING

#ifdef ENABLE_LOGGING
#define LOG(...) printf(__VA_ARGS__ )
#else
#define LOG(...) 
#endif

struct task_node_t {
  std::string name;
  std::vector<void*> args;
  void* run_function;
  pthread_barrier_t* completion_barrier;
};
typedef struct task_node_t task_node;

std::deque<task_node*> task_list;
pthread_mutex_t task_list_mutex;

// Declare extern kernel implementations
extern "C" void DASH_FFT_cpu(double** input, double** output, size_t* size, bool* isForwardTransform);
extern "C" void DASH_GEMM_cpu(double** A_re, double** A_im, double** B_re, double** B_im, double** C_re, double** C_im, size_t* A_ROWS, size_t* A_COLS, size_t* B_COLS);
extern "C" void DASH_ZIP_cpu(double** input_1, double** input_2, double** output, size_t* size, zip_op_t* op);

extern "C" void enqueue_kernel(const char* kernel_name, ...) {
  std::string kernel_str(kernel_name);

  va_list args;
  va_start(args, kernel_name);

  if (kernel_str == "DASH_ZIP") {
    LOG("[nk] I am inside the runtime's codebase, unpacking my args to enqueue a new ZIP task\n");

    double** input_1 = va_arg(args, double**);
    double** input_2 = va_arg(args, double**);
    double** output = va_arg(args, double**);
    size_t* size = va_arg(args, size_t*);
    zip_op_t* op = va_arg(args, zip_op_t*);
    // Last arg: needs to be the synchronization barrier
    pthread_barrier_t* barrier = va_arg(args, pthread_barrier_t*);
    va_end(args);

    // Create some sort of task node to represent this task
    task_node* new_node = (task_node*) calloc(1, sizeof(task_node));
    new_node->name = "ZIP";
    new_node->args.push_back(input_1);
    new_node->args.push_back(input_2);
    new_node->args.push_back(output);
    new_node->args.push_back(size);
    new_node->args.push_back(op);
    new_node->run_function = (void*) DASH_ZIP_cpu;
    new_node->completion_barrier = barrier; 
    
    LOG("[nk] I have finished initializing my ZIP node, pushing it onto the task list\n");

    // Push this node onto the ready queue
    // Note: this would be a GREAT place for a lock-free multi-producer queue
    // Otherwise, every application trying to push in new work is going to get stuck waiting for some eventual ready queue mutex
    pthread_mutex_lock(&task_list_mutex);
    task_list.push_back(new_node);
    pthread_mutex_unlock(&task_list_mutex);
    LOG("[nk] I have pushed a new task onto the work queue, time to go sleep until it gets scheduled and completed\n");
  } else if (kernel_str == "DASH_FFT") {
    LOG("[nk] I am inside the runtime's codebase, unpacking my args to enqueue a new FFT task\n");

    double** input = va_arg(args, double**);
    double** output = va_arg(args, double**);
    size_t* size = va_arg(args, size_t*);
    bool* forwardTrans = va_arg(args, bool*);
    // Last arg: needs to be the synchronization barrier
    pthread_barrier_t* barrier = va_arg(args, pthread_barrier_t*);
    va_end(args);

    // Create some sort of task node to represent this task
    task_node* new_node = (task_node*) calloc(1, sizeof(task_node));
    new_node->name = "FFT";
    new_node->args.push_back(input);
    new_node->args.push_back(output);
    new_node->args.push_back(size);
    new_node->args.push_back(forwardTrans);
    new_node->run_function = (void*) DASH_FFT_cpu;
    new_node->completion_barrier = barrier; 
    
    LOG("[nk] I have finished initializing my FFT node, pushing it onto the task list\n");

    // Push this node onto the ready queue
    // Note: this would be a GREAT place for a lock-free multi-producer queue
    // Otherwise, every application trying to push in new work is going to get stuck waiting for some eventual ready queue mutex
    pthread_mutex_lock(&task_list_mutex);
    task_list.push_back(new_node);
    pthread_mutex_unlock(&task_list_mutex);
    LOG("[nk] I have pushed a new task onto the work queue, time to go sleep until it gets scheduled and completed\n");
  } else if (kernel_str == "DASH_GEMM") {
    // Unpack args and enqueue an mmult task
    double** A_re = va_arg(args, double**);
    double** A_im = va_arg(args, double**);
    double** B_re = va_arg(args, double**);
    double** B_im = va_arg(args, double**);
    double** C_re = va_arg(args, double**);
    double** C_im = va_arg(args, double**);
    size_t* A_Rows = va_arg(args, size_t*);
    size_t* A_Cols = va_arg(args, size_t*);
    size_t* B_Cols = va_arg(args, size_t*);
    // Last arg: needs to be the synchronization barrier
    pthread_barrier_t* barrier = va_arg(args, pthread_barrier_t*);
    va_end(args);

    // Create some sort of task node to represent this task
    task_node* new_node = (task_node*) calloc(1, sizeof(task_node));
    new_node->name = "GEMM";
    new_node->args.push_back(A_re);
    new_node->args.push_back(A_im);
    new_node->args.push_back(B_re);
    new_node->args.push_back(B_im);
    new_node->args.push_back(C_re);
    new_node->args.push_back(C_im);
    new_node->args.push_back(A_Rows);
    new_node->args.push_back(A_Cols);
    new_node->args.push_back(B_Cols);
    new_node->run_function = (void*) DASH_GEMM_cpu;
    new_node->completion_barrier = barrier;

    LOG("[nk] I have finished initializing my GEMM node, pushing it onto the task list\n");

    // Push this node onto the ready queue
    // Note: this would be a GREAT place for a lock-free multi-producer queue
    // Otherwise, every application trying to push in new work is going to get stuck waiting for some eventual ready queue mutex
    pthread_mutex_lock(&task_list_mutex);
    task_list.push_back(new_node);
    pthread_mutex_unlock(&task_list_mutex);
    LOG("[nk] I have pushed a new task onto the work queue, time to go sleep until it gets scheduled and completed\n");
  } else if (kernel_str == "POISON_PILL") {
    LOG("[nk] I am inside the runtime's codebase, injecting a poison pill to tell the host thread that I'm done executing\n");
    va_end(args);

    task_node* new_node = (task_node*) calloc(1, sizeof(task_node));
    new_node->name = "POISON PILL";

    pthread_mutex_lock(&task_list_mutex);
    task_list.push_back(new_node);
    pthread_mutex_unlock(&task_list_mutex);
    LOG("[nk] I have pushed the poison pill onto the task list\n");
  } else {
    LOG("[nk] Unrecognized kernel specified! (%s)\n", kernel_name);
    va_end(args);
    exit(1);
  }
}

struct user_obj_call_t {
  void * func;
  int num_args;
  char** args;
};

void thread_exec_function(void * call_setup) {

  user_obj_call_t * callStruct = (user_obj_call_t *)call_setup;

  // Cast our nullptr argument to a function pointer #justCThings
  void (*libmain)(int, char**) = (void(*)(int, char**)) callStruct->func;

  // Call the function
  (*libmain)(callStruct->num_args, callStruct->args);

  // Once the library's main function exits, enqueue a poison pill to tell the runtime
  enqueue_kernel("POISON_PILL");
}

int main(int argc, char** argv) {
  LOG("Launching the main function of the mock 'runtime' thread [cedr].\n\n");

  pthread_mutex_init(&task_list_mutex, NULL);

  std::string shared_object_name;
  int appInstances = 1;
  int nbCompletedApps = 0;
  user_obj_call_t objCallStruct;
  objCallStruct.args = new char * [argc]; // Create a new list of args, so we can control order
  objCallStruct.num_args = 1;
  
  if (argc > 3) {
    shared_object_name = std::string(argv[1]);
    appInstances = atoi(argv[2]);
    objCallStruct.num_args = argc - 2; // Assumes Call is "mock_runtime x.so 1 <args to x.so>

    for(int i = 3; i < argc; i++)
    {
      objCallStruct.args[i - 2] = argv[i];
    }
  } else if (argc > 2) {
    shared_object_name = std::string(argv[1]);
    appInstances = atoi(argv[2]);
  } else if (argc > 1){
    shared_object_name = std::string(argv[1]);
  } else {
    shared_object_name = "./child.so";
  }

  objCallStruct.args[0] = strdup(shared_object_name.c_str()); // Pass copy so we don't violate const

  void *dlhandle = dlopen(shared_object_name.c_str(), RTLD_LAZY);
  if (dlhandle == NULL) {
    fprintf(stderr, "Unable to open child shared object: %s (perhaps prepend './'?)\n", shared_object_name.c_str());
    return -1;
  } 
  
  objCallStruct.func = (void*)dlsym(dlhandle, "main");

  if (objCallStruct.func == NULL) {
    fprintf(stderr, "Unable to get function handle\n");
    return -1;
  } 

  pthread_t worker_thread[appInstances];
  LOG("[cedr] Launching %d instances of the received application!\n", appInstances);
  for (size_t p = 0; p < appInstances; p++) {
    // Before, we were just calling the provided shared object's main function directly
    //pthread_create(&worker_thread[p], nullptr, (void *(*)(void *))lib_main, nullptr);
    // Now, we call a wrapper function and pass the main function as an argument so that we can insert a hook for enqueueing a poison pill
    // (or otherwise telling the runtime that the application is done executing)
    pthread_create(&worker_thread[p], nullptr, (void *(*)(void *)) thread_exec_function, &objCallStruct);
  }
  void* args[MAX_ARGS];

  while (true) {
    pthread_mutex_lock(&task_list_mutex);
    if (!task_list.empty()) {
      LOG("[cedr] I have a task to do!\n");
      task_node* curr_node = task_list.front();
      task_list.pop_front();
      pthread_mutex_unlock(&task_list_mutex);

      if (curr_node->name == "POISON PILL") {
        LOG("[cedr] I received a poison pill task!\n");
        nbCompletedApps++;
        free(curr_node);
        if (nbCompletedApps == appInstances) {
          LOG("[cedr] Time to break out of my loop and die...\n");
          break;
        }
        else {
          continue;
        }
      }

      LOG("[cedr] It is time for me to process a node named %s\n", curr_node->name.c_str());
      
      if (curr_node->args.size() > MAX_ARGS) {
        fprintf(stderr, "[cedr] This node has too many arguments! I can't run it!\n");
        exit(1);
      }
      for (size_t i = 0; i < curr_node->args.size(); i++) {
        if (i < curr_node->args.size()) {
          args[i] = curr_node->args.at(i);
        } else {
          args[i] = nullptr;
        }
      }

      void* task_run_func = curr_node->run_function;
      void (*run_func)(void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *, void *);
      *reinterpret_cast<void **>(&run_func) = task_run_func;  

      LOG("[cedr] Calling the implementation of this node\n");
      (run_func)(args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], 
                 args[10], args[11], args[12], args[13], args[14]);
      
      LOG("[cedr] Execution is complete. Triggering barrier so that the other thread continues execution\n");
      pthread_barrier_wait(curr_node->completion_barrier);
      
      LOG("[cedr] Barrier finished, going to delete this task node now\n");
      free(curr_node); 
    } else {
      pthread_mutex_unlock(&task_list_mutex);
    }
    pthread_yield();
  }

  for(size_t p = 0; p < appInstances; p++) {
    pthread_join(worker_thread[p], nullptr);
  }

  printf("[cedr] The worker thread has joined, shutting down...\n");

  free(objCallStruct.args[0]);
  delete[] objCallStruct.args;

  dlclose(dlhandle);
  return 0;
}

