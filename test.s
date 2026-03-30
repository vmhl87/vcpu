main:
out "Enter a character: "
in @0 1

set @1 @0 1
sub @1 "a"
comp @5 @1 >=

set @1 @0 1
sub @1 "z"
comp @6 @1 <=

nand @5 @6
cond not_lowercase @5

out "You entered a lowercase letter.\n"
goto printout_end

not_lowercase:

out "You did not enter a lowercase letter.\n"

printout_end:
exit
