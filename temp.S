.org 0x00 ; RESET
	jmp boot 

.org 0x04 ; INT0
	jmp rising_interrupt

.org 0x08 ; INT1
	reti

.org 0x0C ; PCINT0
	reti

.org 0x10 ; PCINT1
	reti

.org 0x14 ; PCINT2
	reti

.org 0x18 ; WDT
	reti

.org 0x1C ; TIMER2 COMPA
	reti

.org 0x20 ; TIMER2 COMPB
	reti

.org 0x24 ; TIMER2 OVF
	reti

.org 0x28 ; TIMER1 CAPT
	reti

.org 0x2C ; TIMER1 COMPA
	reti

.org 0x30 ; TIMER1 COMPB
	reti

.org 0x34 ; TIMER1 OVF
	reti

.org 0x38 ; TIMER0 COMPA
	reti 

.org 0x3C ; TIMER0 COMPB
	reti 

.org 0x40 ; TIMER0 OVF
	jmp timer_interrupt

.org 0x44 ; SPI, STC
	reti 

.org 0x48 ; USART, RX
	jmp usart_rx_interrupt 

.org 0x4C ; USART, UDRE
	reti

.org 0x50 ; USART, TX
	reti

.org 0x54 ; ADC
	reti 

.org 0x58 ; EE READY
	reti 

.org 0x5C ; ANALOG COMP
	reti 

.org 0x60 ; TWI
	reti 

.org 0x64 ; SPM READY
	reti 

.section .bootsec.__boot
__boot:
	jmp boot