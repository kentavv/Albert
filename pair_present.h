#ifndef _PAIR_PRESENT_H_
#define _PAIR_PRESENT_H_

#ifdef __cplusplus
extern "C" {
#endif

void pp_clear(void);
void pp_set(int r, int c);
int pp_contains(int r, int c);
int pp_count(void);

#ifdef __cplusplus
}
#endif

#endif
