.global put
put:
	str w1,[x0]
	ret

.global get
get:
	ldr w0,[x0]
	ret

.global delay
delay:
	subs x0, x0, #1
	bne delay
	ret