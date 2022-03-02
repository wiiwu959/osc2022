.globl put
put:
	str w1,[x0]
	ret

.globl get
get:
	ldr w0,[x0]
	ret

.globl delay
delay:
	subs x0, x0, #1
	bne delay
	ret