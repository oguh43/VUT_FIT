; Autor reseni: hugo bohacsek xbohach00

; Projekt 2 - INP 2024
; Vigenerova sifra na architekture MIPS64

;$8-$15, $24-$25

; DATA SEGMENT
                .data
msg:            .asciiz "hugobohacsek" ; sem doplnte vase "jmenoprijmeni"
cipher:         .space  31 ; misto pro zapis zasifrovaneho textu
key:            .byte 2, 15, 8 ; b o h 




; zde si muzete nadefinovat vlastni promenne ci konstanty,
; napr. hodnoty posuvu pro jednotlive znaky sifrovacho klice

params_sys5:    .space  8 ; misto pro ulozeni adresy pocatku
                          ; retezce pro vypis pomoci syscall 5
                          ; (viz nize "funkce" print_string)

; CODE SEGMENT
                .text

main:           ; ZDE NAHRADTE KOD VASIM RESENIM

                                        ; $t0 - address of msg
                                          ; $t1 - address of cipher
                                          ; $t2 - byte from msg
                                          ; $t3 - address of key
                                          ; $t4 - byte from key
                                          ; $t5 - byte from sign
                                          ; $t6 - general tmp register
                                          ; $t7 - key offset

                ori    $t5, $zero, 0;

                ori    $t7, $zero, 0;

                daddi   $t0, $zero, msg;
               
                daddi $t1, $zero, cipher;

                daddi   $t3, $zero, key;

            loop:

                                          
                                        
                    lb $t2, 0($t0);
                    beq $t2, $zero, done;

                    lb $t4, 0($t3);


                    ; ak sign =0 tak add else sub

                    beq $t5, $zero, addd;
                        sub $t2, $t2, $t4;
                        ori $t5, $zero, 0;
                        j after_addd;
                    addd:
                        add $t2, $t2, $t4;
                        ori $t5, $zero, 1;
                    after_addd:

                    ori $t6, $zero, 97;
                    slt $t6, $t2, $t6;
                    bnez $t6, podtecene;
                    j after_podtecene;
                        podtecene:
                            ori $t6, $zero, 26;
                            add $t2, $t2, $t6;
                    after_podtecene:
                    ori $t6, $zero, 122;
                    slt $t6, $t6, $t2;
                    bnez $t6, nadtecene;
                    j after_nadtecene;
                        nadtecene:
                            ori $t6, $zero, 26;
                            sub $t2, $t2, $t6;
                    after_nadtecene:

                    sb $t2, 0($t1);
                    addi $t0, $t0, 1;
                    addi $t1, $t1, 1;

                    addi $t3, $t3, 1;
                    addi $t7, $t7, 1;

                    ori $t6, $zero, 3;

                    beq $t7, $t6, reset_key;
                    j after_reset_key;
                        reset_key:
                            daddi $t3, $zero, key;
                            ori $t7, $zero, 0;
                    after_reset_key:

                    j loop;




            done:
                sb $zero, 0($t1);

                daddi   r4, r0, cipher ; vozrovy vypis: adresa msg do r4
                jal     print_string ; vypis pomoci print_string - viz nize


; NASLEDUJICI KOD NEMODIFIKUJTE!

                syscall 0   ; halt

print_string:   ; adresa retezce se ocekava v r4
                sw      r4, params_sys5(r0)
                daddi   r14, r0, params_sys5    ; adr pro syscall 5 musi do r14
                syscall 5   ; systemova procedura - vypis retezce na terminal
                jr      r31 ; return - r31 je urcen na return address
