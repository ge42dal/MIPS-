main:
    addi $t0, $zero, 10     # Load 10 into $t0
    addi $t1, $zero, 5      # Load 5 into $t1
    add $t2, $t0, $t1       # Add $t0 + $t1, store in $t2
    addi $a0, $t2, 0        # Copy result to $a0 for printing
    addi $v0, $zero, 0      # Load syscall number for print_int
    syscall                 # Print the result
    addi $v0, $zero, 5      # Load syscall number for exit
    syscall                 # Exit program
