	nop			Comments following instructions work fine!
.main:
	loi	1023		Should have 1023 in gpr0 long load register

	li	gpr1	2	Load 2 into GPR1
	rsl	gpr0	gpr1	Shift GPR0 contents by GPR1 contents (GPR1 < 32)

	addi	gpr0	1	GPR0 = GPR0 + 1 should be 256