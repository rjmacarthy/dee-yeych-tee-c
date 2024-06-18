#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/opensslconf.h>

#include "dht.h"

struct DHT *dht_init(int num_buckets, int bucket_size)
{
    struct DHT *dht = (struct DHT *)malloc(sizeof(struct DHT));
    if (dht == NULL)
        return NULL;
    dht->num_buckets = num_buckets;
    dht->bucket_size = bucket_size;

    dht->nodes = (struct Node **)malloc(num_buckets * sizeof(struct Node *));
    if (dht->nodes == NULL)
    {
        free(dht);
        return NULL;
    }

    for (int i = 0; i < num_buckets; i++)
    {
        dht->nodes[i] = (struct Node *)malloc(bucket_size * sizeof(struct Node));
        if (dht->nodes[i] == NULL)
        {
            for (int j = 0; j < i; j++)
            {
                free(dht->nodes[j]);
            }
            free(dht->nodes);
            free(dht);
            return NULL;
        }
    }

    printf("DHT initialized with %d buckets and %d nodes per bucket.\n", num_buckets, bucket_size);

    return dht;
}

void dht_insert(struct DHT *dht, struct Node *node)
{
    uint32_t hash = dht_calculate_hash(node->id);
    int index = hash % dht->num_buckets;

    printf("Inserting node %s:%d into bucket %d.\n", node->host, node->port, index);

    for (int i = 0; i < dht->bucket_size; i++)
    {
        if (dht->nodes[index][i].id[0] == 0)
        {
            dht->nodes[index][i] = *node;
            dht->num_nodes++;
            return;
        }
    }
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

uint32_t dht_xor_distance(const uint8_t *id1, const uint8_t *id2)
{
    uint32_t distance = 0;
    for (int i = 0; i < ID_SIZE; i++)
    {
        distance |= ((uint32_t)(id1[i] ^ id2[i])) << (8 * (ID_SIZE - i - 1));
        printf("Iteration %d: Distance = 0x%08X\n", i, distance);
    }
    printf("Final distance: 0x%08X\n", distance);
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

    dht_generate_node_id(node->id, node->host, node->port);

    return node;
}

int find_nodes()
{
    return 1;
}

uint32_t dht_calculate_hash(const uint8_t *data)
{
    unsigned char hash[SHA_DIGEST_LENGTH];

    SHA1(data, strlen(data), hash);

    uint32_t result = 0;
    for (int i = 0; i < 4; i++)
    {
        result = (result << 8) | hash[i];
    }

    return result;
}

void dht_generate_node_id(uint8_t *id, const char *host, int port)
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
