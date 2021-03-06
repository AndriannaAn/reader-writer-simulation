Αναστασοπούλου Ανδριαννα
Α.Μ. 1115201300009

compile:
	gcc -o rw_simulation rw_simulation.c -lm

execution:
	./rw_simulation   #peers #entries #repetitions #percentage of readers
	
example of execution:
	./rw_simulation 20 20 100 50

Το τελευταίο argument πρέπει υποχρεωτικά να είναι μεταξύ του 0 και του 100. Τα υπόλοιπα πρέπει να είναι ακέραιοι μεγαλύτεροι του μηδενός.
Γίνεται  εισαγωγή των παραμέτρων από τη γραμμή εντολών, η μετατροπή τους  απο string σε int, o έλεγχός και η εκτύπωση τους.
Δημιουργία του shared memory με κλειδί το process id του coordinator και εν συνεχεία δημιουργία σημαφόρων για τη shared memory.
Αρχικοποίηση των τιμών των σημαφόρων σε 1, attach τη shared memory στον coordinator και αρχικοποίηση των τιμών των counter που βρίσκονται εκεί σε 0.
Γίνεται fork όσες φορές όσο ο αριθμός των peers.
Στον κώδικα του peer, η srand δέχεται ως seed το time(0)-11*getpid() ώστε ο αριθμός της να είναι διαφορετικός ακόμη και αν δεν προλάβει να αλλάξει η τιμή της time.
Μέσω της rand και με τη χρήση του ποσοστού που έχει δωθει αποφασίζεται αν ο peer θα πραγματοποιήσει read ή write στην προκειμενη επανάληψη. Επίσης με την rand επιλέγεται σε ποιο entry θα γινει στην συγκεκριμενη επαναληψη το read/write.
Υλοποιείται ο αλγόριθμος readers/writers όπως δίδάχθηκε στο μάθημα κρατώντας στη shared memory το συνολικό count των read και write (αναλυτικά η υλοποίηση υπάρχει και στα σχόλια του κώδικα). 
Αφού ο κάθε peer τελειώσει τις επαναλήψεις τότε κάνει detach το shared memory, εκτυπώνει τα στατιστικά του και τερματίζει.
Ο coordinator κάνει wait όλα τα peer, διαβάζει το shared memory και εκτυπώνει τα στατιστικά, κανει και αυτός detach και απελευθερώνει τη shared memory και τους σημαφόρους
και τερματίζει.

ΕΝΔΕΙΚΤΙΚΕΣ ΜΕΤΡΗΣΕΙΣ:

20 20 100 50/ 20 20 500 50
συνολικός μέσος όρος:   ~0.008 sec
μέσος όρος για readers: ~0.004 sec 
μέσος όρος για writers: ~0.012 sec 

20 10 100 50
συνολικός μέσος όρος:   ~0.015 sec
μέσος όρος για readers: ~0.008 sec 
μέσος όρος για writers: ~0.021 sec 

20 40 100 50
συνολικός μέσος όρος:   ~0.004 sec
μέσος όρος για readers: ~0.002 sec 
μέσος όρος για writers: ~0.007 sec 
Ο υποδιπλασιασμός των entries διπλασίασε περίπου τους χρόνους και ο διπλασιασμός των entries υποδιπλασίασε περίπου τους χρόνους. 
Συμπεραίνουμε πως ο αριθμός των entries είναι περιπου αντιστρόφος ανάλογος του χρόνου αναμονής.



40 20 100 50/ 40 20 500 50
συνολικός μέσος όρος:   ~0.015 sec
μέσος όρος για readers: ~0.009 sec 
μέσος όρος για writers: ~0.026 sec 

10 20 100 50
συνολικός μέσος όρος:   ~0.0036 sec
μέσος όρος για readers: ~0.0025 sec 
μέσος όρος για writers: ~0.0055 sec 
Ο αριθμός των peer είναι περίπου ανάλογος του χρόνου αναμονής.

Το πλήθος των επαναλήψεων δεν επηρέασε σημαντικά τους μέσους χρόνους αναμονής αλλα οι πολλές επαναλήψεις μείωσαν την εμφάνιση των ακραίων αποτελεσμάτων.

20 20 100 25
συνολικός μέσος όρος:   ~0.011 sec

20 20 100 75
συνολικός μέσος όρος:   ~0.004 sec

20 20 100 0
συνολικός μέσος όρος:   ~0.013 sec

20 20 100 100
συνολικός μέσος όρος:   ~0.000012 sec
Όσο μεγαλύτερο το ποσοστό των readers τόσο μικρότερος ο συνολικός χρόνος αναμονής.


(Οι μετρήσεις έγιναν στα linux της σχολής και είναι κατα προσέγγιση κυριως λόγω της τυχαιότητας του πλήθους read και write.)

