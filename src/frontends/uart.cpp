#include "frontend.h"

#include "avr/generic.h"

#define BAUD 19200

frontend *_this_frontend;

extern void BackendPing();
extern char __stack_end;

#ifdef __AVR__
void putchar(char data)
{
	/* Wait for empty transmit buffer */
	while ( !(UCSR0A & 0b00100000) );

	/* Put data into buffer, sends the data */
	UDR0 = data;
}

char getchar()
{
	/* Wait for data to be received */
	while ( !(UCSR0A & 0b10000000 ));

	/* Get and return received data from buffer */
	return UDR0;
}
#else
#include <stdlib.h>
#include <stdio.h>
#endif


class shell
{
public:
	shell(){};
	shell(cfifo *mycfifo)
	{
		_cfifo = mycfifo;
		bufflen = 0;
		imode = None;

		// Set baud rate
		UBRR0H = (unsigned char)(UBRR(BAUD)>>8);
		UBRR0L = (unsigned char)UBRR(BAUD);
		// Enable receiver and transmitter, as well as the receive interrupt
		UCSR0B = 0b10011000;
		// Set frame format: 8data, 2stop bit
		UCSR0C = 0b00001110;

		cls();
		update_header();
	}

void update_header()
{
	// Store the cursor position
	putchar('\e');
	putchar('[');
	putchar('s');

	// Hide the cursor
	putchar('\e');
	putchar('[');
	putchar('?');
	putchar('2');
	putchar('5');
	putchar('l');

	// Move to top left corner
	putchar('\e');
	putchar('[');
	putchar(';');
	putchar('H');

	// Invert the colors
	putchar('\e');
	putchar('[');
	putchar('7');
	putchar('m');

	write(" Length: ");

	// Perform an itoa here, assume the input won't be longer than 9999 bytes long
	static int numbufflen = 4;
	char numbuff[numbufflen];
	uintToString(bufflen, numbuff, numbufflen);
	for (uint i = numbufflen; i; --i)
	{
		putchar(numbuff[i-1]);
	}

	write(" | Mode: ");
	switch(imode)
	{
	case Writing:
		write("Write");
		break;
	case None:
		write("None");
		break;
	}

	write(" | RAM Free: ");

	// Perform an itoa here, assume the input won't be longer than 9999 bytes long
	static int spbufflen = 4;
	char spbuff[spbufflen];
	uintToString((uint)&spbuff[spbufflen] - (uint)&__stack_end, spbuff, spbufflen);
	for (uint i = spbufflen; i; --i)
	{
		putchar(spbuff[i-1]);
	}

	putchar(' ');

	// Clear line
	putchar('\e');
	putchar('[');
	putchar('K');

	// Reset the colors
	putchar('\e');
	putchar('[');
	putchar('m');

	// Restore the cursor position
	putchar('\e');
	putchar('[');
	putchar('u');

	// Show the cursor
	putchar('\e');
	putchar('[');
	putchar('?');
	putchar('2');
	putchar('5');
	putchar('h');

	return;
}

void cls()
{
	// Reset the colors
	putchar('\e');
	putchar('[');
	putchar('m');

	// Erase entire display xterm
	putchar('\e');
	putchar('[');
	putchar('3');
	putchar('J');

	// Erase entire display regular
	putchar('\e');
	putchar('[');
	putchar('2');
	putchar('J');

	// Move to top left corner, down one line
	putchar('\e');
	putchar('[');
	putchar('2');
	putchar(';');
	putchar('H');

	const char *helpmsg = "Press Enter to begin. Then, press w to enter sending mode.";
	write(helpmsg);
}

void print_message()
{
	// Store the cursor position
	putchar('\e');
	putchar('[');
	putchar('s');

	// Hide the cursor
	putchar('\e');
	putchar('[');
	putchar('?');
	putchar('2');
	putchar('5');
	putchar('l');

	// Down one line
	putchar('\r');
	putchar('\n');

	// Erase after cursor
	putchar('\e');
	putchar('[');
	putchar('J');

	// Down three lines
	putchar('\r');
	putchar('\n');
	putchar('\r');
	putchar('\n');
	putchar('\r');
	putchar('\n');

	char ByteBuffer;
	do {
		_cfifo->getch(&ByteBuffer);
		putchar(ByteBuffer);
	} while (ByteBuffer != '\0');

	// Restore the cursor position
	putchar('\e');
	putchar('[');
	putchar('u');


	// Show the cursor
	putchar('\e');
	putchar('[');
	putchar('?');
	putchar('2');
	putchar('5');
	putchar('h');
}

void parse()
{
	char data = getchar();

	if (data >= 32 && data <= 126) // Regular data characters
	{
		switch(imode)
		{
		case None:
			switch (data)
			{
			case 'w':
				imode = Writing;

				// Clear line
				putchar('\r');
				putchar('\e');
				putchar('[');
				putchar('K');

				write("\r > ");
				break;

			case 's':
				break;

			case 'g':
				break;

			case 'c':
				cls();
				imode = None;
				break;
			}
			break;
		case Writing:
			putchar(data);
			++bufflen;
			while ( !_this_frontend->GetWriteFifo()->putch(data) );
			break;
		}

		update_header();
	}
	else if (data == '\n' || data == '\r') // Newline
	{
		if ( bufflen )
		{
			_this_frontend->GetWriteFifo()->putch('\0');
			BackendPing();
		}
		bufflen = 0;
		imode = None;

		write("\r\n(Select Mode)");

		update_header();
	}
	else if (data == '\e') // Escape
	{
		putchar('^');
		while ( !_this_frontend->GetWriteFifo()->putch('^') );
		++bufflen;
		update_header();
	}
	else if ( data == 0b01111111 ) // Backspace (Delete?)
	{
		if ( bufflen )
		{
			putchar('\b');
			putchar(' ');
			putchar('\b');

			_this_frontend->GetWriteFifo()->pop();

			--bufflen;
			update_header();
		}
	}
	else // Otherwise, just show a ?
	{
		putchar('?');
		while ( !_this_frontend->GetWriteFifo()->putch('?') );
		++bufflen;
		update_header();
	}

	return;
}

private:
	enum InputMode
	{
		None,
		Writing,
	};
	InputMode imode;

	void uintToString(unsigned int n, char *buffer, uint bufflen)
	{
		for (uint pos = 0; pos < bufflen; ++pos) {
			char digit = (n % 10) + '0';
			buffer[pos] = digit;
			n /= 10;
		}
		
		return;
	}

	void write(const char *buff, uint bufflen)
	{
		for (uint i = 0; i < bufflen; --i)
			putchar(buff[i]);
	}

	void write(const char *string)
	{
		for (uint i = 0; string[i]; ++i)
			putchar(string[i]);
	}

	uint bufflen;
	cfifo *_cfifo;
};
shell mysh;

void frontend::FrontendInit()
{
	_this_frontend = this;

	mysh = shell(_read_fifo);
}

EXTERNAL void INT usart_rx_interrupt()
{
	mysh.parse();
}

void FrontendPing()
{
	mysh.print_message();
}