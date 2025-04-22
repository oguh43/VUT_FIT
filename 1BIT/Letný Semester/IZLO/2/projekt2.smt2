(set-logic NIA)

(set-option :produce-models true)
(set-option :incremental true)

; Deklarace promennych pro vstupy
; ===============================

; Parametry
(declare-fun A () Int)
(declare-fun B () Int)
(declare-fun C () Int)
(declare-fun D () Int)
(declare-fun E () Int)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;; START OF SOLUTION ;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Zde doplnte vase reseni

    (assert (and (> D 0) (> E 0)))
    (declare-fun x () Int)
    (assert (= x (* A B 2)))
    (declare-fun y () Int)
    (declare-fun z () Int)
    (assert 
        (=> 
            (< x E)
            (= y (+ x (* 5 B)))
        )
    )
    (assert 
        (=> 
            (>= x E)
            (= y (- x C))
        )
    )
    (assert 
        (=> 
            (< (+ y 2) D)
            (= z (- (* x A) (* y B)))
        )
    )
    (assert 
        (=> 
            (>= (+ y 2) D)
            (= z (+ (* x B) (* y A)))
        )
    )
    (assert 
        (< z (+ E D))
    )
    
    ;(declare-fun xx () Int) PREČO JAAAAAAAAAAAAAAAAAAAAA
    ;(declare-fun yy () Int) PREČO TO NEFUNGOVALO TAKTOOO
    ;(declare-fun zz () Int) CELÉ INC SOM NA TO OBETOVAL
    (assert 
        (not 
            (exists ((ee Int) (dd Int) (xx Int) (yy Int) (zz Int))
                (and 
                    (> dd 0)
                    (> ee 0)
                    (= xx (* A B 2))
                    (=> 
                        (< xx ee)
                        (= yy (+ xx (* 5 B)))
                    )
                    (=> 
                        (>= xx ee)
                        (= yy (- xx C))
                    )
                    (=> 
                        (< (+ yy 2) dd)
                        (= zz (- (* xx A) (* yy B)))
                    )
                    (=> 
                        (>= (+ yy 2) dd)
                        (= zz (+ (* xx B) (* yy A)))
                    )
                    (< zz (+ ee dd))
                    (< (+ ee dd) (+ E D))
                )
            )
        )
    )



    ;(assert
    ;    (forall ((e Int) (d Int)) 
    ;       (< (+ e d) (+ E D)) 
    ;    )
    ;)
    

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;; END OF SOLUTION ;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Testovaci vstupy
; ================

(echo "Test 1 - vstup A=1, B=1, C=3")
(echo "a) Ocekavany vystup je sat a D+E se rovna 2")
(check-sat-assuming (
  (= A 1) (= B 1) (= C 3)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 2, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 1) (= B 1) (= C 3) (distinct (+ D E) 2)
))


(echo "Test 2 - vstup A=5, B=2, C=5")
(echo "a) Ocekavany vystup je sat a D+E se rovna 54")
(check-sat-assuming (
  (= A 5) (= B 2) (= C 5)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 54, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 5) (= B 2) (= C 5) (distinct (+ D E) 54)
))

(echo "Test 3 - vstup A=100, B=15, C=1")
(echo "a) Ocekavany vystup je sat a D+E se rovna 253876")
(check-sat-assuming (
  (= A 100) (= B 15) (= C 1)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 253876, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 100) (= B 15) (= C 1) (distinct (+ D E) 253876)
))

(echo "Test 4 - vstup A=5, B=5, C=3")
(echo "a) Ocekavany vystup je sat a D+E se rovna 51")
(check-sat-assuming (
  (= A 5) (= B 5) (= C 3)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 51, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 5) (= B 5) (= C 3) (distinct (+ D E) 51)
))

(echo "Test 5 - vstup A=2, B=1, C=2")
(echo "a) Ocekavany vystup je sat a D+E se rovna 7")
(check-sat-assuming (
  (= A 2) (= B 1) (= C 2)
))
(get-value (D E (+ D E)))
(echo "b) Neexistuje jine reseni nez 7, ocekavany vystup je unsat")
(check-sat-assuming (
  (= A 2) (= B 1) (= C 2) (distinct (+ D E) 7)
))