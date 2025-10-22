#include "p_merge.h"

//#define NUM_THREADS 4
#define QUEUE_SIZE 1000

typedef struct task
{
    void (*function)(void*);
    void* arg;
    struct task* next;
} task_t;

typedef struct
{
    task_t* head;
    task_t* tail;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} task_queue_t;

typedef struct
{
    pthread_t threads[NUM_THREADS];
    task_queue_t queue;
    int shutdown;
    pthread_mutex_t pool_lock;
} thread_pool_t;

thread_pool_t pool;

typedef struct
{
    int* arr;
    int left;
    int right;
    pthread_mutex_t* completion_lock;
    pthread_cond_t* completion_cond;
    int* tasks_remaining;
} sort_task_data_t;

void merge(int* arr, int l, int m, int r) 
{
    int n1 = m - l + 1;
    int n2 = r - m;

    int* L = (int*)malloc(n1 * sizeof(int));
    int* R = (int*)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int i = 0; i < n2; i++) R[i] = arr[m + 1 + i];

    int i = 0, j = 0, k = l;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

void merge_sort_seq(int* arr, int l, int r)
{
    if (l < r)
    {
        int m = (l + r) / 2;
        merge_sort_seq(arr, l, m);
        merge_sort_seq(arr, m + 1, r);
        merge(arr, l, m, r);
    }
}

void queue_init(task_queue_t* q)
{
    q->head = NULL;
    q->tail = NULL;
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

void queue_push(task_queue_t* q, task_t* task)
{
    pthread_mutex_lock(&q->lock);
    
    while (q->count >= QUEUE_SIZE)
        pthread_cond_wait(&q->not_full, &q->lock);
    
    task->next = NULL;
    if (q->tail)
        q->tail->next = task;
    else
        q->head = task;
    q->tail = task;
    q->count++;
    
    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->lock);
}

task_t* queue_pop(task_queue_t* q)
{
    pthread_mutex_lock(&q->lock);
    
    while (q->count == 0 && !pool.shutdown)
        pthread_cond_wait(&q->not_empty, &q->lock);
    
    if (pool.shutdown && q->count == 0)
    {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }
    
    task_t* task = q->head;

    q->head = task->next;
    if (!q->head)
        q->tail = NULL;

    q->count--;
    
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->lock);
    
    return task;
}

void* worker_thread(void* arg)
{
    while (1)
    {
        task_t* task = queue_pop(&pool.queue);
        
        if (task == NULL)
            break;
        
        task->function(task->arg);
        
        free(task);
    }
    
    pthread_exit(NULL);
}

void pool_init()
{
    queue_init(&pool.queue);
    pool.shutdown = 0;
    pthread_mutex_init(&pool.pool_lock, NULL);
    
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&pool.threads[i], NULL, worker_thread, NULL);
}

void pool_shutdown()
{
    pthread_mutex_lock(&pool.pool_lock);
    pool.shutdown = 1;
    pthread_mutex_unlock(&pool.pool_lock);
    
    pthread_cond_broadcast(&pool.queue.not_empty);
    
    for (int i = 0; i < NUM_THREADS; i++)
        pthread_join(pool.threads[i], NULL);
    
    pthread_mutex_destroy(&pool.pool_lock);
    pthread_mutex_destroy(&pool.queue.lock);
    pthread_cond_destroy(&pool.queue.not_empty);
    pthread_cond_destroy(&pool.queue.not_full);
}

void pool_submit(void (*function)(void*), void* arg)
{
    task_t* task = (task_t*)malloc(sizeof(task_t));
    task->function = function;
    task->arg = arg;
    queue_push(&pool.queue, task);
}

void sort_task(void* arg)
{
    sort_task_data_t* data = (sort_task_data_t*)arg;
    
    merge_sort_seq(data->arr, data->left, data->right);
    
    pthread_mutex_lock(data->completion_lock);
    (*data->tasks_remaining)--;
    pthread_cond_signal(data->completion_cond);
    pthread_mutex_unlock(data->completion_lock);
    
    free(data);
}

void* parallel_merge_sort(void* arg)
{
    thread_data_t* data = (thread_data_t*)arg;
    merge_sort_seq(data->arr, data->left, data->right);
    pthread_exit(NULL);
}

void merge_sort_p(int* arr, int l, int r) 
{
    int n = r - l + 1;
    int segment_size = n / NUM_THREADS;
    
    pthread_mutex_t completion_lock;
    pthread_cond_t completion_cond;
    int tasks_remaining = NUM_THREADS;
    
    pthread_mutex_init(&completion_lock, NULL);
    pthread_cond_init(&completion_cond, NULL);
    
    pool_init();
    
    for (int i = 0; i < NUM_THREADS; i++)
    {
        sort_task_data_t* task_data = (sort_task_data_t*)malloc(sizeof(sort_task_data_t));
        task_data->arr = arr;
        task_data->left = l + i * segment_size;
        
        if (i == NUM_THREADS - 1)
            task_data->right = r;
        else
            task_data->right = l + (i + 1) * segment_size - 1;
        
        task_data->completion_lock = &completion_lock;
        task_data->completion_cond = &completion_cond;
        task_data->tasks_remaining = &tasks_remaining;
        
        pool_submit(sort_task, task_data);
    }
    
    pthread_mutex_lock(&completion_lock);
    while (tasks_remaining > 0)
        pthread_cond_wait(&completion_cond, &completion_lock);

    pthread_mutex_unlock(&completion_lock);
    
    for (int size = segment_size; size < n; size *= 2)
    {
        for (int left = l; left < r; left += 2 * size)
        {
            int mid = left + size - 1;
            int right = (left + 2 * size - 1 < r) ? left + 2 * size - 1 : r;
            
            if (mid < r)
                merge(arr, left, mid, right);
        }
    }
    
    pool_shutdown();
    
    pthread_mutex_destroy(&completion_lock);
    pthread_cond_destroy(&completion_cond);
}


void printArray(int* arr, int n) 
{
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);

    printf("\n");
}

double time_diff_ms(struct timespec start, struct timespec end)
{
    return (end.tv_sec - start.tv_sec) * 1000.0 +
           (end.tv_nsec - start.tv_nsec) / 1000000.0;
}
