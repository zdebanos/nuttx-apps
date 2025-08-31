/****************************************************************************
 * apps/examples/shv-nxboot-updater/shv-nxboot-updater.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <shv/tree/shv_tree.h>
#include <shv/tree/shv_file_node.h>
#include <shv/tree/shv_connection.h>
#include <shv/tree/shv_methods.h>
#include <shv/tree/shv_clayer_posix.h>
#include <shv/tree/shv_dotdevice_node.h>

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <semaphore.h>

#ifdef __NuttX__
#include <nxboot.h>
#include <nuttx/mtd/mtd.h>
#include <sys/boardctl.h>
#endif

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int shv_nxboot_opener(shv_file_node_t *item);
static int shv_root_device_type(shv_con_ctx_t * shv_ctx, shv_node_t *item,
                                int rid);
static int shv_dotapp_vmajor(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                             int rid);
static int shv_dotapp_vminor(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                             int rid);
static int shv_dotapp_name(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                           int rid);
static int shv_dotapp_ping(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                           int rid);
static int shv_fwstable_confirm(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                                int rid);
static void quit_handler(int signum);
static void print_help(char *name);

static shv_node_t *shv_tree_create(void);
static void attention_cb(shv_con_ctx_t *shv_ctx,
                         enum shv_attention_reason r);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* An execution barrier */

static sem_t running;

/* ------------------------- ROOT METHODS --------------------------------- */

const shv_method_des_t shv_dev_root_dmap_item_device_type =
{
  .name = "deviceType",
  .method = shv_root_device_type
};

const shv_method_des_t * const shv_dev_root_dmap_items[] =
{
  &shv_dev_root_dmap_item_device_type,
  &shv_dmap_item_dir,
  &shv_dmap_item_ls,
};

const shv_dmap_t shv_dev_root_dmap =
  SHV_CREATE_NODE_DMAP(root, shv_dev_root_dmap_items);

/* ------------------------- .app METHODS ---------------------------- */

const shv_method_des_t shv_dev_dotapp_dmap_item_vmajor =
{
  .name = "shvVersionMajor",
  .method = shv_dotapp_vmajor
};

const shv_method_des_t shv_dev_dotapp_dmap_item_vminor =
{
  .name = "shvVersionMinor",
  .method = shv_dotapp_vminor
};

const shv_method_des_t shv_dev_dotapp_dmap_item_name =
{
  .name = "name",
  .method = shv_dotapp_name
};

const shv_method_des_t shv_dev_dotapp_dmap_item_ping =
{
  .name = "ping",
  .method = shv_dotapp_ping
};

const shv_method_des_t * const shv_dev_dotapp_dmap_items[] =
{
  &shv_dmap_item_dir,
  &shv_dmap_item_ls,
  &shv_dev_dotapp_dmap_item_name,
  &shv_dev_dotapp_dmap_item_ping,
  &shv_dev_dotapp_dmap_item_vmajor,
  &shv_dev_dotapp_dmap_item_vminor,
};

const shv_dmap_t shv_dev_dotapp_dmap =
  SHV_CREATE_NODE_DMAP(dotapp, shv_dev_dotapp_dmap_items);

/* ------------------------- fwstable METHODS ---------------------------- */

const shv_method_des_t shv_dev_fwstable_dmap_item_confirm =
{
  .name = "confirm",
  .method = shv_fwstable_confirm
};

const shv_method_des_t * const shv_dev_fwstable_dmap_items[] =
{
  &shv_dev_fwstable_dmap_item_confirm,
  &shv_dmap_item_dir,
  &shv_dmap_item_ls
};

const shv_dmap_t shv_dev_fwstable_dmap =
  SHV_CREATE_NODE_DMAP(dotdevice, shv_dev_fwstable_dmap_items);

/****************************************************************************
 * Private Functions
 ****************************************************************************/

static int shv_dotapp_vmajor(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                             int rid)
{
  shv_unpack_data(&shv_ctx->unpack_ctx, 0, 0);
  shv_send_int(shv_ctx, rid, 1);
  return 0;
}

static int shv_dotapp_vminor(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                             int rid)
{
  shv_unpack_data(&shv_ctx->unpack_ctx, 0, 0);
  shv_send_int(shv_ctx, rid, 0);
  return 0;
}

static int shv_dotapp_name(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                           int rid)
{
  shv_unpack_data(&shv_ctx->unpack_ctx, 0, 0);
  shv_send_str(shv_ctx, rid, "SHV Firmware Updater");
  return 0;
}

static int shv_dotapp_ping(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                           int rid)
{
  shv_unpack_data(&shv_ctx->unpack_ctx, 0, 0);
  shv_send_empty_response(shv_ctx, rid);
  return 0;
}

static int shv_root_device_type(shv_con_ctx_t * shv_ctx, shv_node_t *item,
                                int rid)
{
  const char *str = "SHV4LIBS Testing";
  shv_unpack_data(&shv_ctx->unpack_ctx, 0, 0);
  shv_send_str(shv_ctx, rid, str);
  return 0;
}

static int shv_fwstable_confirm(shv_con_ctx_t *shv_ctx, shv_node_t *item,
                                int rid)
{
  shv_unpack_data(&shv_ctx->unpack_ctx, 0, 0);
  nxboot_confirm();
  shv_send_int(shv_ctx, rid, 0);
  return 0;
}

static int shv_nxboot_opener(shv_file_node_t *item)
{
  struct shv_file_node_fctx *fctx = (struct shv_file_node_fctx *)item->fctx;
  if (!(fctx->flags & SHV_FILE_POSIX_BITFLAG_OPENED))
    {
      fctx->fd = nxboot_open_update_partition();
      if (fctx->fd < 0)
        {
          return -1;
        }

      fctx->flags |= SHV_FILE_POSIX_BITFLAG_OPENED;
    }

  return 0;
}

static shv_node_t *shv_tree_create(void)
{
  shv_node_t *tree_root, *fwstable_node, *dotapp_node;
  shv_dotdevice_node_t *dotdevice_node;
  shv_file_node_t *fwupdate_node;

  struct mtd_geometry_s geometry;
  int flash_fd;
  flash_fd = nxboot_open_update_partition();
  if (flash_fd < 0)
    {
      return NULL;
    }

  puts("Creating the SHV Tree root");
  tree_root = shv_tree_node_new("", &shv_dev_root_dmap, 0);
  if (tree_root == NULL)
    {
      close(flash_fd);
      return NULL;
    }

  fwupdate_node = shv_tree_file_node_new("fwUpdate",
                                         &shv_file_node_dmap, 0);
  if (fwupdate_node == NULL)
    {
      close(flash_fd);
      free(tree_root);
      return NULL;
    }

  if (ioctl(flash_fd, MTDIOC_GEOMETRY,
            (unsigned long)((uintptr_t)&geometry)) < 0)
    {
      close(flash_fd);
      free(tree_root);
      free(fwupdate_node);
      return NULL;
    }

  fwupdate_node->file_type = SHV_FILE_MTD;
  fwupdate_node->file_maxsize = geometry.erasesize * geometry.neraseblocks;
  fwupdate_node->file_pagesize = geometry.blocksize;
  fwupdate_node->file_erasesize = geometry.erasesize;

  /* Update the fops table in the file node */

  fwupdate_node->fops.opener = shv_nxboot_opener;
  shv_tree_add_child(tree_root, &fwupdate_node->shv_node);
  close(flash_fd);

  dotapp_node = shv_tree_node_new(".app", &shv_dev_dotapp_dmap, 0);
  if (dotapp_node == NULL)
    {
      free(tree_root);
      free(fwupdate_node);
      return NULL;
    }

  shv_tree_add_child(tree_root, dotapp_node);

  dotdevice_node = shv_tree_dotdevice_node_new(&shv_dotdevice_dmap, 0);
  if (dotdevice_node == NULL)
    {
      free(tree_root);
      free(fwupdate_node);
      free(dotapp_node);
      return NULL;
    }

  dotdevice_node->name = "SHV Compatible Device";
  dotdevice_node->serial_number = "0xDEADBEEF";
  dotdevice_node->version = "0.1.0";
  shv_tree_add_child(tree_root, &dotdevice_node->shv_node);

  fwstable_node = shv_tree_node_new("fwStable", &shv_dev_fwstable_dmap, 0);
  if (fwstable_node == NULL)
    {
      free(tree_root);
      free(fwupdate_node);
      free(dotapp_node);
      return NULL;
    }

  shv_tree_add_child(tree_root, fwstable_node);

  return tree_root;
}

static void quit_handler(int signum)
{
  puts("Stopping SHV FW Updater!");
  sem_post(&running);
}

static void print_help(char *name)
{
  printf("%s: <user> <passwd> <mnt-point> <ip-addr> <tcp/ip-port>\n", name);
  puts("SHV Firmware Updater for NXBoot");
  puts("The SHV tree is comprised of the following nodes:");
  puts("  - .app     // A standard node, shows info about the app");
  puts("  - .device  // A standard node, shows info about the device");
  puts("  - fwUpdate // A file node, abstraction of the NXBoot update "
       "partition");
  puts("  - fwStable // A standard node, contains the confirm method to "
       "confirm");
  puts("                the validity of the newly booted up image.");
}

static void attention_cb(shv_con_ctx_t *shv_ctx, enum shv_attention_reason r)
{
  if (r == SHV_ATTENTION_ERROR)
    {
      printf("Error occured in SHV, the reason is: %s\n",
             shv_errno_str(shv_ctx));
      sem_post(&running);
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 ****************************************************************************/

int main(int argc, char *argv[])
{
  /* Define the SHV Communication parameters */

  int ret;
  struct shv_connection connection;
  shv_node_t *tree_root;
  shv_con_ctx_t *ctx;

  /* Initalize the communication. But only if parameters are passed. */

  if (argc != 6)
    {
      print_help(argv[0]);
      return 1;
    }

  const char *user = argv[1];
  const char *passwd = argv[2];
  const char *mount = argv[3];
  const char *ip = argv[4];
  const char *port_s = argv[5];
  int port = atoi(port_s);

  shv_connection_init(&connection, SHV_TLAYER_TCPIP);
  connection.broker_user =     user;
  connection.broker_password = passwd;
  connection.broker_mount =    mount;
  connection.reconnect_period = 10;
  connection.reconnect_retries = 0;
  if (shv_connection_tcpip_init(&connection, ip, port) < 0)
    {
      fprintf(stderr, "Have you supplied valid params to shv_connection?\n");
      return 1;
    }

  puts("SHV Connection Init OK");

  tree_root = shv_tree_create();
  if (tree_root == NULL)
    {
      fprintf(stderr, "Can't create the SHV tree.");
      return 1;
    }

  puts("SHV Tree created!");
  ctx = shv_com_init(tree_root, &connection, attention_cb);
  if (ctx == NULL)
    {
      fprintf(stderr, "Can't establish the comm with the broker.\n");
      return 1;
    }

  ret = shv_create_process_thread(99, ctx);
  if (ret < 0)
    {
      fprintf(stderr, "%s\n", shv_errno_str(ctx));
      free(ctx);
      return 1;
    }

  sem_init(&running, 0, 0);
  signal(SIGTERM, quit_handler);

  sem_wait(&running);

  puts("Close the communication");
  shv_com_destroy(ctx);

  return 0;
}
