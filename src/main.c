#include "stdlib.h"
#include "stdio.h"
#include "string.h"


/* ################################ Globals ################################ */


/**
 * @brief heap node definition.
 * 
 * @param value the value to store
 * @param left left child
 * @param right right child
 * @param hook pointer to field in the father node
 */
typedef struct node {
  int value;                    
  struct node* left;   
  struct node* right;  
  struct node* hook;
} *Node;

/**
 * @brief number of nodes present in the array of nodes.
 */
int number_of_nodes = -1;

/**
 * @brief seed number that should be fed to the random bit function.
 */
int seed = -1;

/**
 * @brief nodes pointer array that has 'number_of_nodes' size.
 */
Node nodes;


/* ################################ Helpers ################################ */


/**
 * @brief Gets index of node in nodes array.
 * 
 * @param node node to fetch the index from
 * 
 * @return int node index in nodes array
 */
int ptr_to_loc(Node node) {
  int r;
  r = -1;
  if(NULL != node) r = ((size_t) node - (size_t) nodes) / sizeof(struct node);
  return (int) r;
}

/**
 * @brief Receives the index of a node and returns the corresponding node.
 * 
 * @param index index of the requested node
 * 
 * @return Node requested node
 */
Node loc_to_ptr(int index) {
  return &nodes[index];
}

/**
 * @brief Randomly generates 0 or 1 to decide if we are going to meld the heaps
 * in the left (0) or the right (1) child.
 * 
 * @return int 0 or 1
 */
static int random_bit(void) {

  static unsigned int M = 0; 
  static unsigned int P; 
  int r;

  if(0 == M){
    M = ~0;
    P = random();
  }

  r = P & 1;
  M = M>>1;
  P = P>>1;

  printf("Flipped a %d\n", r);
  return r; 
}

/**
 * @brief Swaps child and father (hook) pointers.
 * 
 * @param f father (hook) node
 * @param c child node
 * @param d which child is swapped (0 for left and 1 for right)
 */
static void link(Node f, Node c, int d) {
  printf("link A[%d] as %s of A[%d]\n", ptr_to_loc(c), d ? "rightChild" : "leftChild", ptr_to_loc(f)); 
}

/**
 * @brief Receives a node and tries to find the root of the heap that is connected to it.
 * 
 * @param node node from which we are trying to find the heap root
 * 
 * @return Node heap root
 */
Node find_node_root(Node node) {
  Node root = node;
  printf("root A[%d] is ", ptr_to_loc(node));

  /* Loops until we find a node whose father is NULL (is heap root) */
  while (root->hook != NULL) {
    root = root->hook;
  }

  printf("A[%d]\n", ptr_to_loc(root));
  return root;
}

/**
 * @brief Receives a node and splits it from it's current heap, creating two heaps.
 * 
 * @param node node to be split from it's heap
 * 
 * @return Node new heap root (input node)
 */
Node split_node_from_heap(Node node) {

  /* If node is root, then we don't do anything */
  if (node->hook == NULL) return node;
  
  /* We need to check if this node is the parent's left or right child and sever
   * the connection between them */
  if (node->hook->left == node) {
    node->hook->left = NULL;
    node->hook = NULL;
  } else {
    node->hook->right = NULL;
    node->hook = NULL;
  }

  return node;

}

/**
 * @brief Get the first argument from the user command.
 * 
 * @param cmd user command
 * 
 * @return int node index or value requested by the user
 */
int get_first_argument_from_cmd(char* cmd) {

  /* Gets node index from user input */
  int argument = 0;
  sscanf(cmd, "%*s %d", &argument);

  return argument;
}

/**
 * @brief Get the second argument from the user command.
 * 
 * @param cmd user command
 * 
 * @return int node index or value requested by the user
 */
int get_second_argument_from_cmd(char* cmd) {

  /* Gets node index from user input */
  int argument = 0;
  sscanf(cmd, "%*s %*d %d", &argument);

  return argument;
}


/* ################################ Commands ############################### */


/**
 * @brief Command that shows node information
 * 
 * @param node node that is going to be displayed
 */
void cmd_show_node(Node node) {
  int f;
  if(NULL == node)
    printf("NULL\n");
  else {
    printf("node: %d ", ptr_to_loc(node));
    printf("v: %d ", node->value);
    printf("leftChild: %d ", ptr_to_loc(node->left)); 
    printf("rightChild: %d ", ptr_to_loc(node->right));
    if (node->hook != NULL) {
      f = ptr_to_loc(node->hook);
      printf("father: %c %d", nodes[f].left == node ? 'l' : 'r', f);
    } else {
      printf("father: r -1");
    }
    printf("\n");
  }
}

/**
 * @brief Command that displays heap recursively.
 * 
 * @param heap start of heap
 */
void cmd_show_heap(Node heap) {

  /* Calls show node on the current node */
  cmd_show_node(heap);

  /* Recursively calls show heap on the left child and then the right child */
  if (heap->left != NULL) {
    cmd_show_heap(heap->left);
  } 

  if (heap->right != NULL) {
    cmd_show_heap(heap->right);
  }
}

/**
 * @brief Changes node value if the node is a heap by itself.
 * 
 * @param node_index index of node whose value is going to be changed
 * @param value new node value
 */
void cmd_set_node_value(int node_index, int value) {
  Node node = loc_to_ptr(node_index);
  if (node->left == NULL && node->right == NULL && node->hook == NULL) {
    node->value = value;
    printf("set A[%d] to %d\n", node_index, value);
  }
}

/**
 * @brief Melds and joins two heaps into a single tree.
 * 
 * @param heap_1 left heap that is going to be melded
 * @param heap_2 right heap that is going to be melded
 * 
 * @return int index of the parent node
 */
int cmd_meld_heaps(Node heap_1, Node heap_2) {

  int bit;
  Node child;
  int heap_1_index = ptr_to_loc(heap_1);
  int heap_2_index = ptr_to_loc(heap_2);
  printf("Meld A[%d] A[%d]\n", heap_1_index, heap_2_index);

  /* If either of the heaps is the null, pointer then we finish and return the other */
  if (heap_1 == NULL) {
    return ptr_to_loc(heap_2);
  }
  if (heap_2 == NULL) {
    return ptr_to_loc(heap_1);
  }

  /* According to the algorithm, we swap the indexes of the roots if second heap's
   * value is larger than the first heap's value 
   */
  if (heap_2->value < heap_1->value) {
    heap_1 = loc_to_ptr(heap_2_index);
    heap_2 = loc_to_ptr(heap_1_index);
    printf("Swap to A[%d] A[%d]\n", heap_2_index, heap_1_index);
  }

  /* Now, we flip a coin to see if we do recursion by the left or the right nodes */
  bit = random_bit();

  /* Applies a recursion according to bit and updates the left or right pointers
   * of the parent and updates the parent pointer of the child node */ 
  if (bit == 1) {
    child = loc_to_ptr(cmd_meld_heaps(heap_1->right, heap_2));
    heap_1->right = child;
    child->hook = heap_1;
    link(heap_1, child, bit);
  } else if (bit == 0) {
    child = loc_to_ptr(cmd_meld_heaps(heap_1->left, heap_2));
    heap_1->left = child;
    child->hook = heap_1;
    link(heap_1, child, bit);
  } else {
    printf("Bit value failed: %d !!", bit);
  }

  return ptr_to_loc(heap_1);

}

/**
 * @brief Decreases node value safely. Maintains heap properties.
 * 
 * @param node node whose value is going to be changed
 * @param value new node value
 * 
 * @return int index of new heap root
 */
int cmd_decrease_key(Node node, int value) {
  Node root;
  int node_index = ptr_to_loc(node);
  printf("decKey A[%d] to %d\n", node_index, value);

  /* If the node is the heap root, then we can change its value safely since we
   * are reducing a value that was already the lowest of the heap */
  if (node->hook == NULL) {
    node->value = value;
    return node_index;
  }

  /* Since the input node is not the root node, we have to find it to apply a Meld
   * operation to it and merge the heaps again */
  root = find_node_root(node);

  /* Changes value of the node and splits it from the upper heap into a new heap */
  node->value = value;
  node = split_node_from_heap(node);

  /* Fuses the two heaps together into a new one and returns the index of the new root */
  return cmd_meld_heaps(root, node);
}

/**
 * @brief Finds the minimum value of the heap which holds the input node.
 * 
 * @param node node that is part of a heap
 * 
 * @return int minimum value in heap 
 */
int cmd_min(Node node) {
  Node root;
  printf("min A[%d]\n", ptr_to_loc(node));
  root = find_node_root(node);
  return root->value;
}

/**
 * @brief Finds the index of the node that has the minimum value of the heap which holds the input node.
 * 
 * @param node node that is part of a heap
 * 
 * @return int index of root node
 */
int cmd_arg_min(Node node) {
  Node root = find_node_root(node);
  return ptr_to_loc(root);
}

/**
 * @brief Extracts min node (heap root) that holds the input node.
 * 
 * @param node node that is part of a heap
 * 
 * @return int new root index
 */
int cmd_extract_min(Node node) {
  Node root, left, right;
  printf("extractMin A[%d]\n", ptr_to_loc(node));

  /* Finds node's root and gets its' children to be merged afterwards */
  root = find_node_root(node);
  left = root->left;
  right = root->right;

  /* Updates root's min value to zero (to simulate being extracted) */
  root->value = 0;

  /* If it has no children, then we return the input node's index */
  if (left == NULL && right == NULL) return ptr_to_loc(node);

  /* If only one of them exists, returns only that side's index */
  if (left == NULL && right != NULL) {
    split_node_from_heap(right);
    return ptr_to_loc(right); 
  } else if (left != NULL && right == NULL) { 
    split_node_from_heap(left);
    return ptr_to_loc(left); 
  }

  /* If both of them exist, then we need to apply a meld operation
   * to them and see who gets to be the root */
  left = split_node_from_heap(left);
  right = split_node_from_heap(right);
  return cmd_meld_heaps(left, right);
}

/**
 * @brief Deletes node from a heap.
 * 
 * @param node node to be delete
 * 
 * @return int root of the remaining heap
 */
int cmd_delete(Node node) {
  Node root, sub_tree, heap;
  printf("delete A[%d]\n", ptr_to_loc(node));

  /* If the input node is the root node, then we just need to do an extract min */
  if (node->hook == NULL) {
    return cmd_extract_min(node);
  }

  /* Since it is not the root, we have to find the real root */
  root = find_node_root(node);

  /* Cuts the heap at the input node */
  sub_tree = split_node_from_heap(node);

  /* Extracts min value and gets the new root of the sub tree that was cut */
  heap = loc_to_ptr(cmd_extract_min(sub_tree));

  /* Checks if this new heap is singular (does not have children). If so,
   * then we just need to return the previous root's index */
  if (heap->left == NULL && heap->right == NULL) {
    return ptr_to_loc(root);
  }

  /* Since it was not a singular heap, then we need to meld them together into one */
  return cmd_meld_heaps(root, heap);
}

/**
 * @brief Before executing the function, we print the final configuration of the heap.
 */
void cmd_finish_execution() {
  int i;
  printf("Final configuration:\n");
  for (i = 0; i < number_of_nodes; i++) {
    cmd_show_node(&nodes[i]);
  }
}


/* ################################# Funcs ################################# */


/**
 * @brief Decides which function to execute based on the input from the stdin.
 * 
 * @param input input from stdin that corresponds to a valid command
 */
void control_command(char* input) {
  int command = input[0];

  switch (command) {
    case 'S': {  /* Shows node information */
      int index = get_first_argument_from_cmd(input);
      cmd_show_node(loc_to_ptr(index));
      break;
    }

    case 'P': {  /* Displays whole heap recursively */
      int index = get_first_argument_from_cmd(input);
      cmd_show_heap(loc_to_ptr(index));
      break; 
    }

    case 'V': {  /* Changes value of a node */
      int index = get_first_argument_from_cmd(input);
      int value = get_second_argument_from_cmd(input);
      cmd_set_node_value(index, value);
      break; 
    }     
    
    case 'U': {  /* Join two heaps */
      int heap_1_index = get_first_argument_from_cmd(input);
      int heap_2_index = get_second_argument_from_cmd(input);
      int result = cmd_meld_heaps(loc_to_ptr(heap_1_index), loc_to_ptr(heap_2_index));
      printf("%d\n", result);
      break;
    }
    
    case 'R': {  /* Decreases value of a node */
      int index = get_first_argument_from_cmd(input);
      int value = get_second_argument_from_cmd(input);
      int result = cmd_decrease_key(loc_to_ptr(index), value);
      printf("%d\n", result);
      break;
    }
    
    case 'M': {  /* Returns minumum value of a heap */
      int index = get_first_argument_from_cmd(input);
      int result = cmd_min(loc_to_ptr(index));
      printf("%d\n", result);
      break;
    } 

    case 'A': {  /* Returns the identification of the node that holds the min value */
      int index = get_first_argument_from_cmd(input);
      int result = cmd_arg_min(loc_to_ptr(index));
      printf("%d\n", result);
      break;
    } 

    case 'E': {  /* Extracts the minimum value of the heap */
      int index = get_first_argument_from_cmd(input);
      int result = cmd_extract_min(loc_to_ptr(index));
      printf("%d\n", result);
      break;
    }
    
    case 'D': {  /* Deletes a specific node */
      int index = get_first_argument_from_cmd(input);
      int result = cmd_delete(loc_to_ptr(index));
      printf("%d\n", result);
      break;
    } 

    case 'X': {  /* Finish execution */
      cmd_finish_execution();
      break;
    }

    default:
      printf("Command not supported!\n");
      break;
  }
}

/**
 * @brief Main driver code.
 * 
 * @return int 0 for success or 1 for error
 */
int main() {

  /* Used as parameters in getline to store input and size of input */ 
  size_t len = 0;
  char* input = NULL;

  /* Reads number of nodes and seed value for random function */
  getline(&input, &len, stdin);
  sscanf(input, "%d %d", &number_of_nodes, &seed);

  /* Inits nodes structure and sets seed value for random function calls */
  nodes = (Node) calloc(number_of_nodes, sizeof(struct node));
  srandom(seed);

  do {
    memset(input, 0, len);
    getline(&input, &len, stdin);
    control_command(input);
  } while (input[0] != 'X');

  free(input);
  free(nodes);
  exit(0);
}
