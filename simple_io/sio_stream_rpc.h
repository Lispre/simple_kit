#ifndef SIMPLE_IO_SIO_STREAM_RPC_H
#define SIMPLE_IO_SIO_STREAM_RPC_H

#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

struct sdeque;
struct shash;

struct sio_stream_rpc_server_cloure;

/* �첽����ص� */
typedef void (*sio_stream_rpc_task_callback_t)(void *arg);
/* ��ʱ���ص� */
typedef void (*sio_stream_rpc_timer_callback_t)(uint64_t id, void *arg);
/* ����ص�, cloure������������, ���û�ͨ�����ý���cloure */
typedef void (*sio_stream_rpc_service_callback_t)(struct sio_stream_rpc_server_cloure *cloure, void *arg);

/* �첽���� */
struct sio_stream_rpc_task {
    sio_stream_rpc_task_callback_t cb;
    void *arg;
};

/* �����߳�, ִ���첽���� */
struct sio_stream_rpc_work_thread {
    struct sdeque *task_queue;
    volatile uint64_t task_count;

    pthread_t tid;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
};

/* ��ʱ������ */
struct sio_stream_rpc_timer_task {
    uint64_t id;
    uint64_t expire_time;
    sio_stream_rpc_timer_callback_t cb;
    void *arg;
};

/* ��ʱ�������߳� */
struct sio_stream_rpc_timer_thread {
    uint64_t timer_id;
    struct slist *time_map; /* key: ���Թ���ʱ�� value: Ƕ�׵Ĺ�ϣ��(keyΪtimer_id, valueΪrpc_timer_task��ɵ�slist) */
    struct slist *id_map;   /* key: timer_id, value: rpc_timer_task  */

    pthread_t tid;
    pthread_mutex_t mutex;
};

struct sio_stream_rpc_io_thread;

/* ���ⷢ������� */
struct sio_stream_rpc_client_conn {
    uint64_t conn_id; /* ����ΨһID */
    uint64_t request_id; /* ����ΨһID������ */
    struct sio_stream_rpc_io_thread *io_thread; /* ����IO�߳� */
};

struct sio_stream_rpc_service;
struct sio_stream_rpc_service_item;

/* ���յ��ⲿ���� */
struct sio_stream_rpc_server_conn {
    uint64_t conn_id;
    struct sio_stream *stream;
    struct sio_stream_rpc_io_thread *io_thread; /* ����IO�߳� */
    struct sio_stream_rpc_service *service;
};

/* �첽��������� */
struct sio_stream_rpc_server_cloure {
    struct sio_stream_rpc_service_item *item; /* ����ķ���Ԫ(�û��Ļص�) */

    struct sio_stream_rpc_io_thread *io_thread; /* ���������߳� */
    uint64_t conn_id; /* ������������ID */

    /* �û�ֻ�ܷ����������ֶ� */
    struct shead req_head; /* ����ͷ */
    char *req_body; /* �������� */

    char no_resp; /* ��Ӧ��, �û�����ָ�� */
    char *resp_packet; /* Ӧ���(ͷ+��) */
    uint64_t resp_len; /* Ӧ������� */
};

/* �첽�����Ӧ�� */
struct sio_stream_rpc_client_cloure {
};


/* ע��ķ����Э�� */
struct sio_stream_rpc_service_item {
    uint32_t type; /* ��Ϣ���� */
    sio_stream_rpc_service_callback_t cb; /* �û��ص� */
    void *arg; /* �û����� */
};

/* ������ĳ�˿�, �ṩһϵ�з��� */
struct sio_stream_rpc_service {
    struct sio_stream *sfd; /* �����׽��� */
    struct sio_stream_rpc_io_thread *io_thread; /* �����ĸ�io�߳� */
    struct shash *register_service; /* ע������ЩЭ���(shead�е�type) */
    uint32_t dispatch_index; /* ��ת�ַ����ӵ�io�߳� */
};

/* ����io�߳� */
struct sio_stream_rpc_io_thread {
    uint32_t index; /* �߳��±� */
    struct sio_stream_rpc *rpc; /* rpc��� */

    struct sio *sio; /* �¼�ѭ�� */
    uint64_t conn_id; /* �߳�Ψһ����ID������ */
    struct shash *service_hash; /* ע�����߳��еļ����� */
    struct shash *server_conn_hash; /* ע�����߳��еķ��������  */
    struct shash *client_conn_hash;  /* ע�����߳��еĿͻ������� */
    volatile uint64_t pending_op; /* �߳��ܹ����ڴ�����ٸ�����(�����е��������+�ȴ���Ӧ�����+�����׽�������) */

    pthread_t tid;
    pthread_mutex_t mutex;
    struct sdeque *op_queue; /* �첽��IO������������� */
};

/* RPC���  */
struct sio_stream_rpc {
    char is_running;

    struct sio_stream_rpc_timer_thread *timer_thread;

    uint32_t work_thread_count;
    struct sio_stream_rpc_work_thread *work_threads;

    uint32_t io_thread_count;
    struct sio_stream_rpc_io_thread *io_threads;
};

int sio_stream_rpc_init(uint32_t work_thread_count, uint32_t io_thread_count);
struct sio_stream_rpc_service *sio_stream_rpc_add_service(const char *ip, uint16_t port);
int sio_stream_rpc_register_protocol(struct sio_stream_rpc_service *service, uint32_t type,
        sio_stream_rpc_service_callback_t cb, void *arg);
int sio_stream_rpc_run();

void sio_stream_rpc_run_task(sio_stream_rpc_task_callback_t cb, void *arg);
uint64_t sio_stream_rpc_add_timer(sio_stream_rpc_timer_callback_t cb, void *arg, uint64_t after_ms);
int sio_stream_rpc_del_timer(uint64_t id);

void sio_stream_rpc_finish_cloure(struct sio_stream_rpc_server_cloure *cloure, char no_response, const char *resp, uint32_t resp_len);

#ifdef __cplusplus
}
#endif

#endif
