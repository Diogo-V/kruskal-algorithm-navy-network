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
 * 
 * @param capital parent city that is the parent of this one in the MST sub-trees
 * @param n_connected_cities number of connected cities in the MST sub-tree
 * @param cheapest cheapest highway that can be built in the current iteration
 */
typedef struct city {
  struct highway* highways;
  int port_cost;
  int n_highways;

  struct city* capital;
  int n_connected_cities;
  struct highway* cheapest;
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

/**
 * @brief Holds the total cost that has to be paid for the current city plan.
 */
int total_plan_cost = 0;


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


/* ############################# MST Algorithm ############################# */


/**
 * @brief Checks if these two cities are not in the same sub-tree.
 * 
 * @param c1 one of the cities to check
 * @param c2 other city to check
 * 
 * @return int 0 if false and 1 if true
 */
int cities_are_connected(City c1, City c2) {
  return c1 == c2 || (c1->port_cost != 0 && c2->port_cost != 0);
}

/**
 * @brief Finds the capital city of another child city in a disjoint set.
 * 
 * @param city city to look for the capital
 * 
 * @return City parent city of this child
 */
City find(City child) {
  City parent = child;
  while (parent->capital != NULL) {
    parent = child->capital;
  }
  return parent;
}

/**
 * @brief Perform a union of two disjoint sets. Attaches the smaller rank tree under the root of the higher rank tree
 * 
 * @param x one of the cities to fuse into a single component
 * @param y another city to fuse into a single connected graph
 */
void union_set(City x, City y) {
  City x_root = find(x);
  City y_root = find(y);

  if (x_root->n_connected_cities < y_root->n_connected_cities) {
    x_root->capital = y_root;
  } else if (x_root->n_connected_cities > y_root->n_connected_cities) {
    y_root->capital = x_root;
  } else {
    y_root->capital = x_root;
    x_root->n_connected_cities++;
  }
}

/**
 * @brief Implementation of the boruvka algorithm to compute a minimum
 * spanning tree. Sources for it are:
 * https://www.geeksforgeeks.org/boruvkas-algorithm-greedy-algo-9/
 * https://en.wikipedia.org/wiki/Bor%C5%AFvka%27s_algorithm
 */
void boruvka_mst() {
  int 
    i = 0, 
    previous_city_components = n_cities, 
    n_city_components = n_cities - n_ports + 1, 
    n_highways_used = 0;

  /* While cities are not totally connected, we need to find the cheapest highways to connect them */
  while (n_city_components > 1) {

    /* Resets all highways to be -1 */
    for (i = 0; i < n_cities; i++) {
      cities[i].cheapest = NULL;
    }

    /* Loops over all possible highways that can be built to connect the city and chooses the cheapest
     * for each of the city components that are not yet connected */
    for (i = 0; i < n_highways; i++) {
      Highway h = &highways[i];

      /* Gets parents of both cities associated with this highway */
      City c1 = find(&cities[h->city_1.id]);
      City c2 = find(&cities[h->city_2.id]);

      /* If they are from different city components, we should try to update their cheapest edges */
      if (!cities_are_connected(c1, c2)) {
        if (c1->cheapest == NULL || c1->cheapest->cost < h->cost) {
          c1->cheapest = h;
        }
        if (c2->cheapest == NULL || c2->cheapest->cost < h->cost) {
          c2->cheapest = h;
        }
      }

    }

    /* Traverses all cities and build cheapest highways associated */
    for (i = 0; i < n_cities; i++) {
      if (cities[i].cheapest != NULL) {

        /* Find capital city of the cities in this highway */
        City c1 = find(&cities[cities[i].cheapest->city_1.id]);
        City c2 = find(&cities[cities[i].cheapest->city_2.id]);

        /* If cities are not in the same component, unites them into a single one */
        if (!cities_are_connected(c1, c2)) {
          total_plan_cost += cities[i].cheapest->cost;
          union_set(c1, c2);
          n_city_components--;
          n_highways_used++;
        }
      }
    }

    /* Nothing changed and so, it has finished without connecting all cities */
    if (previous_city_components == n_city_components) {
      printf("Impossible\n");
      return;
    }
    previous_city_components = n_city_components;
  }

  /* Algorithm finished and all cities are connected */
  printf("%d\n%d %d\n", total_plan_cost, n_ports, n_highways_used);

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

  /* Each city will start off by being connected to itself and having only one connection */
  for (i = 0; i < n_cities; i++) {
    cities[i].capital = NULL;
    cities[i].cheapest = NULL;
    cities[i].n_connected_cities = 1;
  }

  /* Builds ports using the configuration from standard in */
  scanf("%d", &n_ports);
  for (i = 0; i < n_ports; i++) {
    scanf("%d %d", &city_1, &cost);
    cities[city_1].port_cost = cost;
    total_plan_cost += cost;
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
 * using the ports and the highways and computes the total city cost and counting the 
 * number of ports and highways built.
 */
void compute_city_plan() {
  boruvka_mst();
}

/**
 * @brief Main driver code.
 * 
 * @return int 0 for success or 1 for error
 */
int main() {

  /* Builds cities configuration */
  build_cities();

  /* Computes the minimum spanning tree plan of this city and its cost */
  compute_city_plan();

  /* Cleans up the program by freeing all the allocated memory */
  free_program_memory();

  exit(0);
}
