main:
    addi $t0, $zero, 10     # Load 10 into $t0
    addi $t1, $zero, 5      # Load 5 into $t1
    add $t2, $t0, $t1       # Add $t0 + $t1, store in $t2
    addi $a0, $t2, 0        # Copy result to $a0 for printing
    trap 0                  # Print integer syscall
    trap 5                  # Exit program syscall
