
#ifndef __LIST__
#define __LIST__

#define LIST_POISON (void *)0xDEADDEAD

struct list_head
{
    struct list_head *next;
    struct list_head *prev;
};

static inline void init_list_head(struct list_head *list)
{
    list->prev = list;
    list->next = list;
}

static inline void add_list(struct list_head *member, struct list_head *new_member)
{
    // Setup new member
    new_member->next = member->next;
    new_member->prev = member;

    // Setup original member
    member->next->prev = new_member;
    member->next = new_member;
}

static inline void del_list(struct list_head *member)
{
    member->next->prev = member->prev;
    member->prev->next = member->next;

    member->next = LIST_POISON;
    member->prev = LIST_POISON;
}

//
//  ptr - Pointer to the 'member'
//  type - type of the container
//  member - name of member
//
// (type *)0) // cast address 0x00 to type
// (((type *)0)->member ) // get the 'offset' of the member
// __typeof__(((type *)0)->member )  // get the type of member
// 
// const __typeof__(((type *)0)->member ) *__mptr = (ptr);
//  // Declare __mptr to be of the type of member and equal to ptr
//
// Last line, is get the offset into the container that member variable will be
//
// (char *)__mptr - offsetof(type,member)
//      Starting at the address of the member, subtract away its
//      offset into the container to get the container address.
//      
// (type *)( (char *)__mptr - offsetof(type,member) );
//      Cast the container to that type
//
#define container_of(ptr, type, member) __extension__       \
    ({                                                       \
        const __typeof__(((type *)0)->member ) *__mptr = (ptr); \
        (type *)( (char *)__mptr - offsetof(type,member) );  \
    })

#define list_entry(ptr, type, member) container_of(ptr, type, member)

#endif // #define __LIST__

