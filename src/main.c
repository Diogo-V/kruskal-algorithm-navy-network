#include "stdlib.h"
#include "stdio.h"


/* ################################ Globals ################################ */


/**
 * @brief Tells us the current state of the highway.
 * 
 * @param PENDING is still waiting to be selected 
 * @param PLANNED has been selected but has not yet been approved
 * @param BUILT was built and commit
 */
typedef enum build_status {
  PENDING,
  PLANNED,
  BUILT,
} BuildStatus;

/**
 * @brief Graph node that contains connection to the other possible highways of this city.
 * 
 * @param id number that identifies the city
 * @param next next highway node in the list of possible highways
 * @param previous previous highway node in the list of possible highways
 */
typedef struct node {
  int id;
  struct highway* next;
  struct highway* previous;
} *Node;

/**
 * @brief Highway configuration that is going to be associated to a city and used
 * in a linked list.
 * 
 * @param city_1 city node whose holder can be connected to through a highway
 * @param city_2 other city node whose holder can be connected to through a highway
 * @param cost cost of building this highway
 * @param status signals us if the highway has been built or is still pending
 */
typedef struct highway {
  struct node city_1;
  struct node city_2;
  int cost;
  BuildStatus status;
} *Highway;

/**
 * @brief City configuration that can be turned into a graph.
 * 
 * @param highways linked list of highways that can be built
 * @param port_cost cost of building a port (0 if no port can be built)
 * @param n_highways number of highways that can be built to this city
 */
typedef struct city {
  struct highway* highways;
  int port_cost;
  int n_highways;
} *City;

/**
 * @brief Holds the number of cities in our graph.
 */
int n_cities = -1;

/**
 * @brief Number of possible ports that can exist in our graph.
 */
int n_ports = -1;

/**
 * @brief Number of possible highways that can exist in our graph.
 */
int n_highways = -1;

/**
 * @brief Holds cities and their configurations.
 */
City cities;

/**
 * @brief Holds highways that can be built in this city.
 */
Highway highways;


/* ################################ Helpers ################################ */


/**
 * @brief Gets index of city in cities array.
 * 
 * @param city city to fetch the location from
 * 
 * @return int index of city
 */
int ptr_to_loc(City city) {
  int r;
  r = -1;
  if(NULL != city) r = ((size_t) city - (size_t) cities) / sizeof(struct city);
  return (int) r;
}

/**
 * @brief Helps to debug the building of the city graph.
 */
void debug_print_cities() {
  int i, k;
  for (i = 1; i < n_cities + 1; i++) {
    printf("City %d: port_cost %d | n_highways %d\n", i, cities[i].port_cost, cities[i].n_highways);
  }
  for (k = 0; k < n_highways; k++) {
    printf("Highway %d: c1 %d | c2 %d | cost %d | status %d\n", k,
      highways[k].city_1.id, highways[k].city_2.id, highways[k].cost, highways[k].status);
  }
}

/**
 * @brief Get the city highway linked list node object.
 * 
 * @param city city that holds this highway
 * @param highway highway that can be built in this city
 * 
 * @return Node linked list node that keeps track of all highways in this city
 */
Node get_city_highway_node(City city, Highway highway) {
  int city_index = ptr_to_loc(city);
  if (city_index == highway->city_1.id) {
    return &highway->city_1;
  } else {
    return &highway->city_2;
  }
}

/**
 * @brief Inserts a new highway in this city's linked list.
 * 
 * @param city city id that is going to holds this highway
 * @param highway highway reference
 */
void insert_highway(City city, Highway highway) {
  Node node = get_city_highway_node(city, highway);

  /* If this is the first highway being added, we just need to update all pointers */
  if (city->n_highways == 0) {
    city->highways = highway;
    node->next = NULL;
    node->previous = NULL;
    return;
  }

  /* When we only have one highway, we should just connect both */
  Node city_node = get_city_highway_node(city, city->highways);
  if (city->n_highways == 1) {
    city_node->next = highway;
    city_node->previous = highway;
    node->next = city->highways;
    node->previous = city->highways;
    return;
  }

  /* Since it is not the first highway that has been added to this city, we have to append it */
  Node last_node = get_city_highway_node(city, city_node->previous);

  node->next = city->highways;
  node->previous = city_node->previous;
  last_node->next = highway;
  city_node->previous = highway;
}

/**
 * @brief Creates highway reference and links it to both cities.
 * 
 * @param city_1 one of the cities involved in this highway
 * @param city_2 another city in the highway
 * @param cost cost of building the highway
 * @param index index of the highway in the highways object
 */
void build_highway(int city_1, int city_2, int cost, Highway h) {
  /* Saves cities identifiers*/
  h->city_1.id = city_1;
  h->city_2.id = city_2;

  /* Inserts highway in both cities's linked list of highways */
  insert_highway(&cities[city_1], h);
  insert_highway(&cities[city_2], h);

  /* Updates number of highways that can be built in both cities */
  cities[city_1].n_highways++;
  cities[city_2].n_highways++;

  /* Updates highway cost */
  h->cost = cost;
  h->status = PENDING;
}

/**
 * @brief Frees all the allocated memory in all nodes.
 */
void free_program_memory() {
  free(cities);
  free(highways);
}


/* ################################# Funcs ################################# */


/**
 * @brief Builds cities inital configuration from the standard input.
 */
void build_cities() {
  int i = 0, cost = 0, city_1 = 0, city_2 = 0;

  /* Reads number of cities and build structure for it */
  scanf("%d", &n_cities);
  cities = (City) calloc(n_cities + 1, sizeof(struct city));

  /* Builds ports using the configuration from standard in */
  scanf("%d", &n_ports);
  for (i = 0; i < n_ports; i++) {
    scanf("%d %d", &city_1, &cost);
    cities[city_1].port_cost = cost;
  }

  /* Reads max number of highways that can be built and builds struct for it */
  scanf("%d", &n_highways);
  highways = (Highway) calloc(n_highways, sizeof(struct highway));

  /* Inserts highways in the struct and connects them to the cities */
  for (i = 0, city_1 = 0; i < n_highways; i++) {
    scanf("%d %d %d", &city_1, &city_2, &cost);
    build_highway(city_1, city_2, cost, &highways[i]);
  }
}

/**
 * @brief Uses the previously built city and plans the connections between the cities
 * using the ports and the highways.
 */
void plan_city() {
  // empty
}

/**
 * @brief Computes the total city cost and counts the number of ports and highways built.
 */
void compute_city_plan_cost() {
  // empty
}

/**
 * @brief Main driver code.
 * 
 * @return int 0 for success or 1 for error
 */
int main() {

  /* Builds cities configuration */
  build_cities();

  debug_print_cities();

  /* Creates the minimum spanning tree of this city */
  plan_city();

  /* Computes the cost of the city plan */
  compute_city_plan_cost();

  /* Cleans up the program by freeing all the allocated memory */
  free_program_memory();

  exit(0);
}
