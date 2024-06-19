#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/opensslconf.h>

#include "dht.h"

struct DHT *dht_init(int num_buckets, int bucket_size) {
    struct DHT *dht = (struct DHT *)malloc(sizeof(struct DHT) + num_buckets * sizeof(struct Bucket *));
    if (dht == NULL) {
        return NULL;
    }

    dht->num_buckets = num_buckets;
    dht->bucket_size = bucket_size;

    for (int i = 0; i < num_buckets; i++) {
        dht->buckets[i] = (struct Bucket *)malloc(sizeof(struct Bucket)); //Allocate buckets
        if (dht->buckets[i] == NULL) {
            // Handle bucket allocation failure (free previously allocated memory)
            dht_free(dht);
            return NULL;
        }

        dht->buckets[i]->nodes = (struct Node **)malloc(bucket_size * sizeof(struct Node));
        if (dht->buckets[i]->nodes == NULL) {
            // Handle node allocation failure (free previously allocated memory)
            dht_free(dht);
            return NULL;
        }

        // Initialize each node in the bucket (optional)
        for (int j = 0; j < bucket_size; j++) {
            memset(&(dht->buckets[i]->nodes[j]), 0, sizeof(struct Node));
        }
    }
    return dht;
}

int dht_get_bucket_index(struct DHT *dht, uint160_t *targetHash)
{
    int index = 0;
    return index;
}

struct Node *dht_find_node(struct DHT *dht, uint160_t *targetHash)
{
    struct Node *closestNode = {0}; // Initialize with an empty node
    int closestDistance = -1;       // Initialize with maximum possible distance
    int k = 20;                     // The Kademlia concurrency parameter (e.g., k = 20)

    printf("FIND NODE\n");

    // 1. Start with the local node's k-bucket closest to the target hash
    int bucketIndex = dht_get_bucket_index(dht, targetHash);
    struct Bucket *bucket = dht->buckets[bucketIndex];

    printf("BUCKET: %d\n", bucketIndex);
    printf("SIZE: %d\n", dht->bucket_size);

    // 2. Iterate through nodes in the bucket
    for (int i = 0; i < dht->bucket_size; i++)
    {
        if (bucket->nodes[0] != 0)
        {
            int distance = dht_xor_distance(targetHash, &bucket->nodes[i]->id);

            if (distance < closestDistance || closestDistance == -1)
            {
                closestDistance = distance;
            }

            k--;
            if (k == 0)
                break; // Stop if we have k closest nodes
        }
    }

    printf("CLOSEST NODE: %d\n", closestDistance);

    if (closestDistance == -1)
    {
        printf("NO NODE FOUND\n");
        return NULL;
    }

    struct Node *foundNode = dht_find_node(dht, targetHash);

    // 5. Compare the found node with the current closest node
    if (dht_xor_distance(targetHash, &foundNode->id) < closestDistance)
    {
        return foundNode;
    }
    else
    {
        return closestNode;
    }
}

void dht_insert(struct DHT *dht, struct Node *node)
{
    uint160_t hash;
    dht_calculate_hash(node->id, &hash);

    printf("Ok");

    struct Node *responsibleNode = dht_find_node(dht, &hash);
}

void dht_print(struct DHT *dht)
{
    for (int i = 0; i < dht->num_buckets; i++)
    {
        printf("bucket %d: ", i);
        for (int j = 0; j < dht->bucket_size; j++)
        {
            printf("%d ", dht->keys[i]);
        }
        printf("\n");
    }
}

void dht_free(struct DHT *dht)
{
    free(dht);
}

void dht_free_node(struct Node *node)
{
    free(node);
}

uint32_t dht_xor_distance(const uint160_t *id1, const uint160_t *id2)
{
    uint32_t distance = 0;
    return distance;
}

struct Node *node_connect(const char *bootstrap_node_address)
{
    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL)
    {
        perror("malloc");
        return NULL;
    }

    char host[INET6_ADDRSTRLEN];
    int port;
    if (sscanf(bootstrap_node_address, "%[^:]:%d", host, &port) != 2)
    {
        fprintf(stderr, "Invalid bootstrap node address format.\n");
        free(node);
        return NULL;
    }

    struct hostent *he = gethostbyname(host);
    if (he == NULL)
    {
        herror("gethostbyname");
        free(node);
        return NULL;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        free(node);
        return NULL;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    memcpy(&servaddr.sin_addr, he->h_addr_list[0], he->h_length);

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect");
        close(sockfd);
        free(node);
        return NULL;
    }

    strcpy(node->host, inet_ntoa(servaddr.sin_addr));
    node->port = port;

    dht_generate_node_id(&node->id, node->host, node->port);

    return node;
}

int find_nodes()
{
    return 1;
}

void dht_calculate_hash(const uint8_t *data, uint160_t *hash)
{
    unsigned char temp_hash[SHA_DIGEST_LENGTH];
    SHA1(data, strlen(data), temp_hash);
    memcpy(hash, temp_hash, SHA_DIGEST_LENGTH);
}

void dht_generate_node_id(uint160_t *id, const char *host, int port)
{
    char data[INET6_ADDRSTRLEN + 10];
    snprintf(data, sizeof(data), "%s:%d", host, port);

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char *)data, strlen(data), hash);

    memcpy(id, hash, SHA_DIGEST_LENGTH);
}

void bootstrap(struct DHT *dht, const char *bootstrap_node_address)
{
    struct Node *bootstrap_node = node_connect(bootstrap_node_address);
    if (bootstrap_node == NULL)
    {
        fprintf(stderr, "Failed to connect to bootstrap node.\n");
        return;
    }

    struct Node *other_nodes[MAX_BOOTSTRAP_NODES];
    int num_nodes = find_nodes();

    for (int i = 0; i < num_nodes; i++)
    {
        dht_insert(dht, other_nodes[i]);
    }
}
