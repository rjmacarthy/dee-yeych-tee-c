#include <stdlib.h>
#include <string.h>
#include <CUnit/Basic.h>
#include "dht.h"

struct Node *get_test_node(const char *host, int port)
{
   struct Node *node = malloc(sizeof(struct Node));
   memset(node, 0, sizeof(struct Node));
   strcpy(node->host, host);
   node->port = port;
   return node;
}

void dht_calculate_hash_test()
{
   const char *data1 = "abc";
   const char *data2 = "cdf";
   uint32_t hash1 = dht_calculate_hash((const uint8_t *)data1);
   uint32_t hash2 = dht_calculate_hash((const uint8_t *)data2);
   CU_ASSERT_EQUAL(hash1, 1663389532);
   CU_ASSERT_EQUAL(hash2, 2072896544);
}

void dht_xor_distance_test()
{
    uint8_t id1[ID_SIZE] = {0x80, 0x00, 0x00, 0x00}; // 100000000
    uint8_t id2[ID_SIZE] = {0x40, 0x00, 0x00, 0x00}; // 010000000
    uint32_t distance = dht_xor_distance(id1, id2); // 
    CU_ASSERT_EQUAL(distance, 0xC0000000);         
}

void dht_test_init()
{
   struct DHT *dht = dht_init(1, 8);
   CU_ASSERT_PTR_NOT_NULL(dht);
   struct Node *node = get_test_node("127.0.0.1", 8080);
   dht_insert(dht, node);
   printf("Buckets: %d Nodes: %d\n", dht->num_buckets, dht->num_nodes);
   CU_ASSERT_EQUAL(dht->nodes[0][0].port, 8080);
   dht_free(dht);
   dht_free_node(node);
   dht_xor_distance_test();
}

int main()
{
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   CU_pSuite pSuite = NULL;

   pSuite = CU_add_suite("sum_test_suite", 0, 0);
   if (pSuite == NULL)
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_pTest pTest = CU_add_test(pSuite, "dht", dht_test_init);

   if (pTest == NULL)
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   CU_basic_set_mode(CU_BRM_VERBOSE);

   CU_basic_run_tests();
   return CU_get_error();
}