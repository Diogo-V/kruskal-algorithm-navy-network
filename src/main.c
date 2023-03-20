#include "stdlib.h"
#include "stdio.h"


/* ################################ Globals ################################ */


/**
 * @brief Highway configuration that is going to be associated to a city and used
 * in a linked list.
 * 
 * @param city_1 city node whose holder can be connected to through a highway
 * @param city_2 other city node whose holder can be connected to through a highway
 * @param cost cost of building this highway
 */
typedef struct highway {
  int city_1;
  int city_2;
  int cost;
} *Highway;

/**
 * @brief City configuration that can be turned into a graph.
 * 
 * @param id id of the city
 * @param port_cost cost of building a port (0 if no port can be built)
 * @param capital parent city that is the parent of this one in the MST sub-trees
 * @param n_connected_cities number of connected cities in the MST sub-tree
 * @param cheapest cheapest highway that can be built in the current iteration
 */
typedef struct city {
  int id;
  int port_cost;
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
 * @brief Creates highway reference and links it to both cities.
 * 
 * @param city_1 one of the cities involved in this highway
 * @param city_2 another city in the highway
 * @param cost cost of building the highway
 * @param index index of the highway in the highways object
 */
void build_highway(int city_1, int city_2, int cost, Highway h) {
  /* Saves cities identifiers*/
  h->city_1 = city_1;
  h->city_2 = city_2;

  /* Updates highway cost */
  h->cost = cost;
}

/**
 * @brief Used in qsort to sort all highways.
 * 
 * @param h1 highway 1
 * @param h2 highway 2
 * 
 * @return int 1 if left is less or -1 if not
 */
int highway_compare(const Highway h1, const Highway h2) {
  return h1->cost >= h2->cost ? 1 : -1;
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
  while (parent->capital != parent) {
    parent = child->capital;
  }
  return parent;
  /* if (child->capital == child)
    return child;
  return child->capital = find(child->capital); */
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

void kruskalAlgo() {
  int 
    i = 0, 
    n_components = n_cities - n_ports, 
    n_highways_used = 0;

  if (n_ports != 0) {
    n_components++;
  }

  for (i = 0; i < n_highways; i++) {
    Highway h = &highways[i];

    City v1 = find(&cities[h->city_1]);
    City v2 = find(&cities[h->city_2]);

    if (!cities_are_connected(v1, v2)) {
        union_set(v1, v2);
        total_plan_cost += h->cost;
        n_highways_used++;
        n_components--;
    }
    
  }

  if (n_components > 1) {
    printf("Impossible\n");
    return;
  }

  /* Algorithm finished and all cities are connected */
  printf("%d\n%d %d\n", total_plan_cost, n_ports, n_highways_used);
}

/**
 * @brief Implementation of the boruvka algorithm to compute a minimum
 * spanning tree. Sources for it are:
 * https://www.geeksforgeeks.org/boruvkas-algorithm-greedy-algo-9/
 * https://en.wikipedia.org/wiki/Bor%C5%AFvka%27s_algorithm
 * 
 * @param n_city_components number of cities to be connected
 */
void boruvka_mst(int n_city_components) {
  int 
    i = 0, 
    previous_city_components = n_cities, 
    n_highways_found_in_loop = 0,  /* Used to stop the loop of finding highways earlier */
    n_highways_used = 0;

  /* While cities are not totally connected, we need to find the cheapest highways to connect them */
  while (n_city_components > 1) {

    /* Loops over all possible highways that can be built to connect the city and chooses the cheapest
     * for each of the city components that are not yet connected */
    for (i = 0, n_highways_found_in_loop = 0; i < n_highways && n_highways_found_in_loop < n_city_components - 1; i++) {
      Highway h = &highways[i];

      /* Gets parents of both cities associated with this highway */
      City c1 = find(&cities[h->city_1]);
      City c2 = find(&cities[h->city_2]);

      /* If they are from different city components, we should try to update their cheapest edges */
      if (!cities_are_connected(c1, c2)) {
        if (c1->cheapest == NULL || (c1->cheapest->cost <= h->cost && h->city_1 < c1->cheapest->city_1)) {
          c1->cheapest = h;
        }
        if (c2->cheapest == NULL || (c2->cheapest->cost <= h->cost && h->city_1 < c2->cheapest->city_1)) {
          c2->cheapest = h;
        }
        n_highways_found_in_loop++;
      }

    }

    /* Traverses all cities and build cheapest highways associated */
    for (i = 0; i < n_cities; i++) {
      if (cities[i].cheapest != NULL) {

        /* Find capital city of the cities in this highway */
        City c1 = find(&cities[cities[i].cheapest->city_1]);
        City c2 = find(&cities[cities[i].cheapest->city_2]);

        /* If cities are not in the same component, unites them into a single one */
        if (!cities_are_connected(c1, c2)) {
          total_plan_cost += cities[i].cheapest->cost;
          union_set(c1, c2);
          n_city_components--;
          n_highways_used++;
        }

        cities[i].cheapest = NULL;
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
  scanf("%d\n", &n_cities);
  cities = (City) calloc(n_cities + 1, sizeof(struct city));

  /* Each city will start off by being connected to itself and having only one connection */
  for (i = 1; i < n_cities + 1; i++) {
    cities[i].capital = &cities[i];
    cities[i].cheapest = NULL;
    cities[i].n_connected_cities = 1;
    cities[i].id = i + 1;
  }

  /* Builds ports using the configuration from standard in */
  scanf("%d\n", &n_ports);
  for (i = 0; i < n_ports; i++) {
    scanf("%d %d", &city_1, &cost);
    cities[city_1].port_cost = cost;
    total_plan_cost += cost;
  }

  /* Reads max number of highways that can be built and builds struct for it */
  scanf("%d\n", &n_highways);
  highways = (Highway) calloc(n_highways, sizeof(struct highway));

  /* Inserts highways in the struct and connects them to the cities */
  for (i = 0, city_1 = 0; i < n_highways; i++) {
    scanf("%d %d %d\n", &city_1, &city_2, &cost);
    build_highway(city_1, city_2, cost, &highways[i]);
  }

  /* Sorts highways to make it faster to loop for them */
  qsort(highways, n_highways, sizeof(struct highway), (int (*) (const void *, const void *)) &highway_compare);
}

/**
 * @brief Uses the previously built city and plans the connections between the cities
 * using the ports and the highways and computes the total city cost and counting the 
 * number of ports and highways built.
 */
void compute_city_plan() {
  int n_city_components = n_cities - n_ports;

  /* Fixes number of city components in the case that there is no ports */
  if (n_ports != 0) {
    n_city_components++;
  }

  boruvka_mst(n_city_components);

  /* kruskalAlgo(); */
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
