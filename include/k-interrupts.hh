#ifndef	PIKADEE_K_INTERRUPTS_HH
#define	PIKADEE_K_INTERRUPTS_HH

extern "C" {

    void init_irq_vector( void );
    void enable_irq( void );
    void disable_irq( void );
};

void init_interrupts();

void init_timer ( void );
void handle_timer_irq ( void );


#endif  /*_IRQ_H */
