#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <nuttx/config.h>
#include <nuttx/analog/adc.h>
#include <nuttx/analog/ioctl.h>
#include <arch/board/board.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#include <nuttx/can/can.h>
#include <pthread.h>

int fd0, fd1;

static void *canbus_producer(void *foo)
{
  struct can_msg_s txmsg; 
  uint16_t id = 1;
  txmsg.cm_hdr.ch_dlc = 1;
  txmsg.cm_hdr.ch_rtr = false;
  txmsg.cm_data[0] = 5;
#ifdef CONFIG_CAN_ERRORS
  txmsg.cm_hdr.ch_error = 0;
#endif
  // txmsg.cm_hdr.ch_unused = 0;
  while (true)
    {
      txmsg.cm_hdr.ch_id = id;
      size_t msgsize = CAN_MSGLEN(1);
      int nbytes = write(fd0, &txmsg, msgsize);
      if (nbytes != msgsize)
        {
          printf("ERROR writing CAN0!\n");
        }
      else
        {
          printf("CAN0 write OK: id=%u, dlc=%u\n", txmsg.cm_hdr.ch_id, txmsg.cm_hdr.ch_dlc);
        }
      usleep(1000);
      id = (id + 1) % 0x07FF;
    }
}

static void *canbus_consumer(void *foo)
{
  struct can_msg_s rxmsg;
  size_t msgsize = sizeof(struct can_msg_s);
  while (true)
    {
      int nbytes = read(fd1, &rxmsg, msgsize);
      if (nbytes < 0)
        {
          printf("ERROR reading CAN1!\n");
        }
      printf("CAN1 read OK: id=%u, dlc=%u\n", rxmsg.cm_hdr.ch_id, rxmsg.cm_hdr.ch_dlc);
    }
}

int canbus_example(void)
{
  /* Open up two Canbuses */
  fd0 = open("/dev/can0", O_RDWR);
  fd1 = open("/dev/can1", O_RDWR);
  if (fd0 < 0)
    {
      printf("Error opening CAN0!\n");
      return EXIT_FAILURE;
    }
  if (fd1 < 0)
    {
      printf("Error opening CAN1\n");
      return EXIT_FAILURE;
    }

  pthread_t producer, consumer;
  pthread_create(&consumer, NULL, canbus_consumer, NULL);
  usleep(10000);
  pthread_create(&producer, NULL, canbus_producer, NULL);

  pthread_join(producer, NULL);
  pthread_join(consumer, NULL);
  return EXIT_SUCCESS;
}




