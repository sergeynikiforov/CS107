#!r6rs
(import (rnrs))

(display "hello\n")

(define (average num-list)
  (/ (apply + num-list) (length num-list)))

(define (depth seq)
  (if (or (not (list? seq)) (= (length seq) 0)) 0
      (+ 1 (apply max (map depth seq)))))

(define (translate seq offset)
  (map (lambda (n) (+ n offset)) seq))

(define (ps set)
  (if (null? set) '(())
      (let ((ps-rest (ps (cdr set))))
        (append ps-rest
                (map (lambda (subset)
                       (cons (car set) subset))
                     ps-rest)))))

(define (remove seq elem)
  (cond ((null? seq) '())
        ((equal? (car seq) elem) (remove (cdr seq) elem))
        (else (cons (car seq) (remove (cdr seq) elem)))))

(define (permute items)
  (if (null? items) '(())
      (apply append
             (map (lambda (elem)
                    (map (lambda (permutation)
                           (cons elem permutation))
                         (permute (remove items elem))))
                  items))))
