//
// Created by Shuhao Zhang on 9/9/19.
//

#ifndef ALLIANCEDB_LOCK_H
#define ALLIANCEDB_LOCK_H

typedef volatile char Lock_t;

inline void unlock(Lock_t * _l)  __attribute__ ((always_inline)); //__attribute__((noinline));
inline void lock(Lock_t * _l) __attribute__((always_inline)); // __attribute__ ((noinline));
inline int  tas(volatile char * lock) __attribute__((always_inline));

/*
 * Non-recursive spinlock. Using `xchg` and `ldstub` as in PostgresSQL.
 */
/* Call blocks and retunrs only when it has the lock. */
inline void lock(Lock_t * _l){
    while(tas(_l)) {
#if defined(__i386__) || defined(__x86_64__)
        __asm__ __volatile__ ("pause\n");
#endif
    }
}

/** Unlocks the lock object. */
inline void unlock(Lock_t * _l){
    *_l = 0;
}

inline int tas(volatile char * lock)
{
    register char res = 1;
#if defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__ (
    "lock xchgb %0, %1\n"
    : "+q"(res), "+m"(*lock)
    :
    : "memory", "cc");
#elif defined(__sparc__)
    __asm__ __volatile__ (
                          "ldstub [%2], %0"
                          : "=r"(res), "+m"(*lock)
                          : "r"(lock)
                          : "memory");
#else
#error TAS not defined for this architecture.
#endif
    return res;
}

#endif //ALLIANCEDB_LOCK_H
