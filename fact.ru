(func uint fact(uint n) (
    (if (== n 1) (
	(return 1)))
    (return (* n (fact (- n 1))))))

(func void main() (
    (fact 6)))
