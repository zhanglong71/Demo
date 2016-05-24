#ifndef __ARCH_H__
#define __ARCH_H__

void IRQ_disable(void);
void IRQ_enable(void);

void	fstack_init(fstack_t *s);
int	fstack_pop(fstack_t *s);
int	fstack_push(fstack_t *s, func_t *func);
int	fstack_top(const fstack_t *s, func_t *f);

int msg_init(msg_t *pMsg);
void msgq_init(msgq_t *q);
int	msgq_in(msgq_t *q, msg_t *val);
int	msgq_in_irq(msgq_t *q, msg_t *val);
int	msgq_out(msgq_t *q, msg_t *val);
int	msgq_out_irq(msgq_t *q, msg_t *val);

void SetTimer(Timer_t * timer, int tick, msgType_t msgType); 
void SetTimer_irq(Timer_t * timer, int tick, msgType_t msgType);

void ClrTimer(Timer_t * timer);
void ClrTimer_irq(Timer_t * timer);

void delay(int __ms);
void delaySpin(u32 _loop);
void delaySpin_us(u32 _us);
void delaySpin_ms(u32 _ms);
//void delaySpinMs(void);

void atomic_set(short *_var, short _value);
short atomic_get(short *_var);

//void actionQueueInit(actionQueue_t *q);
//void actionQueueInit(actionQueue_t *q, Timer_t *timer, msgType_t msgType);
void adcSample_Init(adcData_t *adcData, Timer_t *timer, msgType_t stepMsgType, int tick);
int adcSample_In(adcData_t *adcData, u16 __value);
int adcSample_Out(adcData_t *adcData, u16 *__average);

void actionQueueInit(actionQueue_t *q, Timer_t *timer, msgType_t stepMsgType, msgType_t overMsgType);
int actionQueueIn(actionQueue_t *q, action_t *action);
int actionQueueOut(actionQueue_t *q, action_t *action);

#endif
