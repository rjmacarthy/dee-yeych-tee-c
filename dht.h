#include <arpa/inet.h>
#include <stdint.h> // for uint8_t
#include <openssl/sha.h>
#include <stdint.h>
#include <gmp.h>

#ifndef NODE_H
#define NODE_H
#define MAX_BUCKET_SIZE 8
#define MAX_BOOTSTRAP_NODES 10
#define ID_SIZE 20
#define UINT160_MAX ((uint160_t) -1) 

typedef unsigned char uint160_t[20];

struct DHT
{
    int num_buckets;
    int num_nodes;
    int bucket_size; 
    int *keys;
    int *values;
    struct Bucket *buckets[];
};

struct Bucket
{
    int num_nodes;  
    struct Node **nodes;
};

struct Node
{
    uint160_t id;
    char host[INET6_ADDRSTRLEN];
    int port;
    time_t lastSeen;
    int rtt;
};



struct DHT *dht_init(int num_buckets, int bucket_size);

struct Node *node_connect(const char *bootstrap_node_address);

void dht_print(struct DHT *dht);

void dht_insert(struct DHT *dht, struct Node *node);

uint32_t dht_xor_distance(const uint160_t *id1, const uint160_t *id2);

void dht_generate_node_id(uint160_t *id, const char *host, int port);

void dht_free_node(struct Node *node);

void dht_free(struct DHT *dht);

void dht_calculate_hash(const uint8_t *data, uint160_t *hash);

#endif