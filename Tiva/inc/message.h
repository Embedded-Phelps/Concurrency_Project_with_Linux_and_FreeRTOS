// packed structures?
// network layer to be completed
// timestamp functions to be completed

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include <stdint.h>
#include "messageConfig.h"

#ifdef USE_MESSAGE_OVER_LINUX_MQUEUE
#include <string.h>
#include <sys/stat.h>   // For mode constants
#include <fcntl.h>      // For O_* constants
#include <pthread.h>    // For pthread_mutex_t
#include <mqueue.h>     // For mQueue

#define MAX_QUEUE_NAME_LENGTH   (10)
#endif

#ifdef USE_MESSAGE_OVER_FREERTOS_QUEUE
#include "FreeRTOS.h"
#include "queue.h"      // For FreeRTOS queue
#include "semphr.h"     // For FreeRTOS Semaphore

#define MAX_QUEUE_ELEMENTS  (5)
#endif

typedef uint8_t msg_src_t, msg_dst_t, msg_type_t, crc_t;

/* A universal message structure */
typedef struct msg
{
#ifdef USE_SERVER_CLIENT_MESSAGING
    uint8_t id; //0: server, >0: clients
#endif
    msg_src_t src;
    msg_dst_t dst;
    msg_type_t type;
#ifdef USE_MESSAGE_TIMESTAMP
    uint32_t timestamp; // format: mm/dd/yyyy hh:mm:ss
#endif
    //char content[MAX_MESSAGE_LENGTH];
}msg_t;

/*
 ********** Architecture-specific message delivery layer **********
 */
#ifdef USE_MESSAGE_OVER_LINUX_MQUEUE

typedef struct x_queue
{
    pthread_mutex_t lock;   // Protect queue operations
    char name[MAX_QUEUE_NAME_LENGTH];            // Enqueue and dequeue messages
    struct mq_attr attr;
}x_queue_t;

/**
 * @brief create a LINUX mqueue for messaging
 *
 * Create a mqueue and initialize its mutex lock and
 * conditional variable
 *
 * @param   q_name - name of the queue
 *          q_size - maximum number of messages on the queue
 *          q - pointer to a queue handle
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_create_LINUX_mq(char * q_name, uint32_t q_size, x_queue_t * q);

/**
 * @brief destroy a LINUX mqueue in use
 *
 * Close and unlink a mqueue, destroy its mutex lock and
 * conditional variable
 *
 * @param q - pointer to a queue handle
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_destroy_LINUX_mq(x_queue_t * q);

/**
 * @brief send a message via queue
 *
 * @param q - pointer to a queue handle
 *        msg - pointer to the message to be sent
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_send_LINUX_mq(x_queue_t * q, msg_t * msg);

/**
 * @brief receive a messages from queue
 *
 * @param q - pointer to a queue handle
 *        msg - pointer to the message storage
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_receive_LINUX_mq(x_queue_t * q, msg_t * msg);

#endif

#ifdef USE_MESSAGE_PACKET

/* define a packet structure that wraps around the msg_t structure that makes the
 * transmission of msg_t structure over unreliable network channels reliable
 */
typedef struct msg_packet
{
    uint8_t header;
    crc_t crc;
    msg_t msg;
}msg_packet_t;

/**
 * @brief Pack a message into a packet
 *
 * @param msg - pointer to the message to be packed
 *
 * @return  the message packet
 */
msg_packet_t msg_create_messagePacket(msg_t * msg);

/**
 * @brief Compute the CRC of a message
 *
 * @param msg - pointer to a message
 *
 * @return  a CRC value.
 */
crc_t msg_compute_messagePacketCRC(uint8_t * msg, uint32_t length);

/**
 * @brief validate a packet by checking the packet header and CRC
 *
 * @param packet - pointer to a message packet
 *
 * @return 0 - not valid
 *         1 - valid
 */
int8_t msg_validate_messagePacket(msg_packet_t * packet);

#endif

#ifdef USE_MESSAGE_OVER_FREERTOS_QUEUE

typedef struct x_queue
{
    QueueHandle_t queue;      // Enqueue and dequeue messages
    SemaphoreHandle_t lock;   // Protect queue operations
    SemaphoreHandle_t sem;    // Signal and wait for queue availability
}x_queue_t;

/**
 * @brief create a FreeRTOS queue for messaging
 *
 * Create a FreeRTOS queue and initialize its mutex lock and
 * binary semaphore
 *
 * @param q - pointer to a queue handle
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_create_FreeRTOS_queue(x_queue_t * q, uint32_t q_num_element, uint32_t q_element_size);

/**
 * @brief destroy a FreeRTOS queue in use
 *
 * Close and delete a FreeRTOS queue, destroy its mutex lock and
 * conditional variable
 *
 * @param q - pointer to a queue handle
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_destroy_FreeRTOS_queue(x_queue_t * q);

/**
 * @brief send a message via queue
 *
 * @param q - pointer to a queue handle
 *        msg - pointer to the message to be sent
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_send_FreeRTOS_queue(x_queue_t * q, msg_packet_t * msg);

/**
 * @brief receive a messages from queue
 *
 * @param q - pointer to a queue handle
 *        msg - pointer to the message storage
 *
 * @return  0 - success
 *         -1 - failed
 */
int8_t msg_receive_FreeRTOS_queue(x_queue_t * q, msg_packet_t *msg);

#endif

#endif
