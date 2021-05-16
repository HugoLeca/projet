#ifndef P_REGULATOR_H
#define P_REGULATOR_H

/*returns the speed evaluated after the proportional regulator*/
int16_t p_regulator(float distance, float goal);

#endif /* P_REGULATOR_H */