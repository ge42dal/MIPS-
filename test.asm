main:
    addi $t0, $zero, 42
    addi $t1, $zero, 10
    add $t2, $t0, $t1
    addi $a0, $t2, 0
    trap 0
    trap 5
