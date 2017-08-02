/* Name: Jeffrey Xu
 * Email: jeffreyhxu@gmail.com
 * ID: 404768745
 */
#include "SortedList.h"
#include <string.h>
#include <sched.h>

/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
  SortedListElement_t *i = (SortedListElement_t *)list; //starting with the head
  while(i->next != NULL){
    if(strcmp(i->next->key, element->key) > 0)
      break;
    i = i->next;
  }
  SortedListElement_t *inext = i->next; // so it's consistent for the rest
  if(opt_yield & INSERT_YIELD)
    sched_yield();
  element->prev = i;
  element->next = inext;
  if(inext != NULL)
    inext->prev = element;
  i->next = element;
}

/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete(SortedListElement_t *element){
  SortedListElement_t *enext = element->next;
  SortedListElement_t *eprev = element->prev;
  if((enext != NULL && enext->prev != element) || eprev->next != element)
    return 1; // prev should never be NULL because we won't delete the head.
  if(opt_yield & DELETE_YIELD)
    sched_yield();
  if(enext != NULL)
    enext->prev = eprev;
  eprev->next = enext;
  element->next = NULL;
  element->prev = NULL;
  return 0;
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
  SortedListElement_t *i = list->next; // starting with real elements
  while(i != NULL){
    if(!strcmp(i->key, key))
      break;
    if(opt_yield & LOOKUP_YIELD)
      sched_yield();
    i = i->next;
  }
  return i;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list){
  SortedListElement_t *i = list->next; // starting with real elements
  int length = 0;
  while(i != NULL){
    if(i->prev->next != i || (i->next != NULL && i->next->prev != i))
      return -1; // prev should never be NULL because we won't delete the head
    if(opt_yield & LOOKUP_YIELD)
      sched_yield();
    i = i->next;
    length++;
  }
  return length;
}
