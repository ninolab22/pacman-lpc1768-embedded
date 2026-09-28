# Piano di completamento – Pac-Man LandTiger (Extra Point #1)

Analisi del progetto rispetto alle richieste del documento `extrapoint_1.pdf` (consegna 12/01,
target di compilazione **SW_Debug**, emulatore LANDTIGER / LPC1768).

> Questo file è **solo un piano**: elenca cosa c'è, cosa manca e i bug/problemi. Nessuna correzione è stata applicata.

---

## 1. Sintesi rapida

Il progetto è già in buono stato: il labirinto, il movimento, il punteggio, il teleport e le
schermate base esistono. Verificato a livello di codice che **il labirinto contiene esattamente 240
pillole standard** (conteggio sui vettori `linea4..linea29`), e che la "central box" è mantenuta. Il
target `SW_Debug` e la define `SIMULATOR` sono presenti nel `.uvprojx`.

Mancano soprattutto: **avvio in PAUSA**, alcune correzioni su pausa/countdown, una randomness vera per
le power pill, e gli **elementi di consegna non-codice** (PDF compilato + video di 4 minuti).

Nota tecnica importante (non è un bug): la logica colori di collisione si appoggia allo **swap R/B in
lettura** fatto da `LCD_GetPoint` → `LCD_BGR2RGB` (GLCD.c:472). I muri disegnati in `Blue` (0x001F)
vengono riletti come `0xF800` e le power pill disegnate in `Red` vengono rilette come `0x001F`. Le
costanti nel codice di movimento sono quindi coerenti: **non vanno "corrette" ingenuamente**.

---

## 2. Elementi di consegna richiesti dal PDF

| Elemento | Stato | Note |
|---|---|---|
| Cartella progetto zippata con tutte le opzioni di compilazione | ✅ presente | Salvare lo zip dopo aver verificato il salvataggio delle opzioni del target SW_Debug |
| Target di compilazione **SW_Debug** | ✅ presente | Confermato in `sample.uvprojx` (`<TargetName>SW_Debug</TargetName>`) e define `SIMULATOR` |
| Documento PDF compilato (template `extrapoint1`) | ❌ da fare | Va riempito: sezione "Additional Comments" con le variabili/define usate (scaling factor, posizioni, ecc.) e **screenshot della configurazione emulatore** che sostituisce l'immagine nel template |
| Video 4 minuti con audio (.mp4/.avi) | ❌ da fare | Deve mostrare una **sessione di debug software** con tutte le finestre delle periferiche significative aperte, voce che descrive il comportamento (italiano o inglese) |

---

## 3. Stato delle 10 specifiche

| Spec | Descrizione | Stato | Sintesi |
|---|---|---|---|
| 1 | 240 pillole standard | ✅ Completo | Verificato: esattamente 240 pillole disegnate |
| 2 | 6 power pill in posizione/tempo casuali | ⚠️ Parziale | Genera fino a 6 power pill, ma randomness debole/non vera |
| 3 | Movimento joystick continuo fino a muro/nuova direzione | ✅ Completo | Implementato in `movimento()` + RIT |
| 4 | Teleport laterale | ✅ Completo | Solo sx↔dx a y=195 (coerente con la figura) |
| 5 | Mangiare pillole +10 / +50 | ✅ Completo | Logica corretta (con swap colori) |
| 6 | +1 vita ogni 1000 punti (da 1) | ⚠️ Parziale | Funziona ma cap a 4 vite e icona vita iniziale non disegnata |
| 7 | Pausa INT0 + avvio in PAUSA | ⚠️ Parziale | Toggle pausa c'è MA: non parte in pausa, resume non cancella "PAUSE", countdown non congelato |
| 8 | Countdown 60→0 | ⚠️ Da verificare | Timing probabilmente 2s/tick e parte solo al primo movimento |
| 9 | Schermata "Victory!" | ✅/⚠️ Base | Solo testo sovrapposto, niente schermata pulita |
| 10 | Schermata "Game Over!" | ✅/⚠️ Base | Solo testo sovrapposto |
| — | Display Score / Lives / Countdown | ⚠️ Parziale | Score ✅, Countdown ✅, Lives manca icona iniziale |

---

## 4. Cosa c'è (già funzionante)

- **Labirinto e 240 pillole**: `LCD_DrawLevel()` + `DrawGridFromVector()` (GLCD.c). Conteggio `food`
  arriva a 240; central box esclusa correttamente dalle pillole (righe 16–22, colonne 10–13).
- **Movimento**: `movimento()` (GLCD.c:1097) chiamato da `TIMER1_IRQHandler` ogni ~0.2s; direzione
  impostata dal joystick nel `RIT_IRQHandler`. Movimento continuo, stop al muro.
- **Teleport**: gestito nei case left/right di `movimento()` (x=5 ↔ x=235 a y=195).
- **Punteggio**: `aggiornapunteggio()` (GLCD.c:1074), +10 standard / +50 power, display "SCORE".
- **Vita extra**: ogni 1000 punti (`extralifecount`), con disegno icona Pac-Man.
- **Pausa INT0**: gestita in `RIT_IRQHandler` (down_0) con messaggio "PAUSE".
- **Countdown**: Timer0 con variabile `timer=60`, display "GAME OVER IN".
- **Vittoria/Game Over**: testi mostrati quando `food==0` o `timer==-1`.
- **Init periferiche**: BUTTON, RIT, joystick, LCD, TouchPanel inizializzati in `main`.

---

## 5. Bug e problemi rilevati (da risolvere in seguito)

### Priorità ALTA (impattano la conformità alle spec)

1. **Il gioco NON parte in PAUSA** (viola Spec 7). In `sample.c:101` `flag_movimento=1` e
   `flag_pause` di default = 0. Manca il disegno iniziale del messaggio "PAUSE".
2. **Resume dalla pausa non cancella la scritta "PAUSE"**. In `IRQ_RIT.c:47` "PAUSE" è disegnato a
   `(95,205)`; nel resume (`IRQ_RIT.c:52-53`) viene ridisegnata `linea30` alla **riga 13** (y≈130),
   che NON copre la scritta a y≈205 → "PAUSE" rimane a schermo.
3. **Il countdown non si ferma durante la pausa**. `TIMER0_IRQHandler` (IRQ_timer.c:40) non controlla
   `flag_pause`/`flag_movimento`, quindi il timer continua a scendere anche in pausa.
4. **Timing del countdown probabilmente errato (~2s invece di 1s)**. `sample.c:112`
   `init_timer(0, 1, 0, 3, 0x17D7840)`: prescaler=1 → il TC incrementa ogni 2 cicli PCLK → con
   MR0=25.000.000 e PCLK≈25MHz risulta ~2s per tick (60 → ~120s reali). **Da verificare in emulatore.**
5. **Randomness power pill debole** (viola lo spirito di Spec 2). La variabile `random`
   (`sample.c:37`) non è mai assegnata (vale 0) e non c'è `srand()`: `rand()` produce sempre la stessa
   sequenza → posizioni e tempi NON realmente casuali tra esecuzioni.

### Priorità MEDIA (robustezza / correttezza logica)

6. **`SetPowerPills` può generare power pill su celle già mangiate o sotto Pac-Man**.
   `DrawpowerPills` (GLCD.c:708) controlla solo la matrice statica (`vector[col]==0`), non lo stato
   attuale del pixel → può "ricreare" pillole su celle già svuotate (corrompe `food` e la condizione
   di vittoria) o disegnarle sopra Pac-Man.
7. **Icona vita iniziale non disegnata**. `life=1` ma nessuna icona a inizio gioco
   (`sample.c:100` è commentato). Le icone compaiono solo con le vite extra.
8. **Cap vite a 4** (`GLCD.c:1079`, `life<4`): non previsto dalla spec. Da decidere se rimuovere.
9. **`powerPills` non decrementato** quando una power pill viene mangiata: raggiunte 6, non se ne
   generano altre. Coerente con "6 totali", ma da confermare che sia il comportamento voluto.
10. **Vittoria/Game Over solo testuali** (Spec 9/10): "Victory!" / "GAME OVER" scritti sopra il
    labirinto senza pulire lo schermo. Funzionale ma minimale rispetto alle "schermate" della figura.
11. **Possibile ri-pausa dopo fine partita**: con `flag_movimento` già 0, premere INT0 ridisegna
    comunque "PAUSE" sopra la schermata di fine.

### Priorità BASSA (pulizia / warning)

12. **Codice KEY3 errato e morto** (`IRQ_RIT.c:112-128`): usa lo stesso pin di KEY1 (`1<<11`),
    `down_3` non viene mai impostato (BUTTON_init abilita solo EINT0/1/2). Da rimuovere.
13. **`extern` senza tipo (implicit int)**: es. `GLCD.c:40 extern random,powerPills;`,
    `IRQ_timer.c:35 extern timer;`, vari `extern flag_movimento;`. Generano warning e sono cattiva
    pratica; meglio dichiararli in un header con tipo esplicito.
14. **Ordine `enable_timer(3)` prima di `init_timer(3,...)`** (`sample.c:114-115`): illogico (funziona,
    ma da sistemare).
15. **Codice non utilizzato**: `ADC_init`/`LED_init` commentati, calibrazione TouchPanel commentata,
    file LED/ADC/joystick "funct" non necessari per la spec → candidati a pulizia.
16. **`first_time` fa partire il countdown solo al primo movimento** (`movimento()`): da decidere se
    il countdown debba invece partire all'uscita dalla pausa iniziale (collegato a Spec 7/8).

---

## 6. Dipendenze/dettagli da non rompere durante le correzioni

- **Swap colori in lettura**: la collisione muri (`!=0xF800`) e lo scoring power pill (`==0x001F`)
  funzionano SOLO grazie a `LCD_BGR2RGB` in `LCD_GetPoint`. Non modificare i colori di disegno o le
  costanti di confronto senza tenerne conto.
- **Geometria a passi di 10px**: Pac-Man si muove di 10px e le pillole sono centrate nelle celle
  10×10. Cambi di dimensione richiedono di riallineare detection pillole/muri e teleport (y=195).
- **240 pillole**: qualsiasi modifica al labirinto deve mantenere il conteggio a 240 (Spec 1).

---

## 7. Strategia di correzione (approccio per ciascun bug)

> Decisioni di default prese (modificabili): countdown parte al primo movimento dopo l'uscita dalla
> pausa iniziale; schermate finali a tutto schermo gestite nel `main` (no lavoro pesante in ISR);
> cap vite rimosso (col punteggio massimo si arriva comunque a ~3 vite); 6 power pill totali nella
> partita (non rigenerate dopo essere mangiate). Seed `rand()` preso da un contatore incrementato nel
> RIT durante la pausa iniziale (entropia dal tempo di reazione umano).

1. **(bug 1) Avvio in PAUSA** — In `sample.c` impostare `flag_movimento=0` e `flag_pause=1` all'avvio
   e disegnare "PAUSE". Così la prima pressione di INT0 entra nel ramo "resume".
2. **(bug 2) Erase PAUSE** — Spostare il testo "PAUSE" dentro la central box (cols 10–13, righe 16–22,
   zona senza pillole) a `(100,192)`. Nuova funzione `erasePause()` che ridisegna SOLO quella regione
   (muro→Blue, vuoto→Black) senza toccare/contare pillole. Sostituisce il vecchio `linea30`.
3. **(bug 3) Countdown in pausa** — In `TIMER0_IRQHandler` decrementare solo se `flag_movimento==1`.
4. **(bug 4) Timing 1s** — In `sample.c` cambiare il prescaler del timer0 da `1` a `0`
   (`init_timer(0, 0, 0, 3, 0x17D7840)`), così 25M conteggi @25MHz = 1s.
5. **(bug 5) Randomness** — `srand(seed_ctr)` una volta (seed_ctr incrementato nel RIT); spawn con
   probabilità casuale per tick (tempo) e cella casuale (posizione).
6. **(bug 6) Spawn solo su pillole esistenti** — Riscrivere `SetPowerPills()`: scegliere una cella a
   caso e creare la power pill solo se `LCD_GetPoint()==White` (pillola standard presente). Rimuove
   `DrawpowerPills()` (codice morto che usava `random`).
7. **(bug 7) Icona vita iniziale** — Nuova `draw_first_life()` in GLCD.c (accede a `x_life`),
   chiamata nel `main` dopo il testo "LIVES".
8. **(bug 8) Cap vite** — Rimuovere la condizione `life<4` in `aggiornapunteggio()`.
9. **(bug 9) 6 power pill totali** — Mantenuto come da spec ("Generate 6"); nessuna modifica.
10. **(bug 10/11) Schermate finali + no ri-pausa** — Variabile `game_state` (0/1/2). Vittoria e
    Game Over impostano `game_state` e fermano i timer; il `main` (dopo `wfi`) fa `LCD_Clear` e mostra
    la schermata una sola volta. INT0 ignora la pausa se `game_state!=0`.
11. **(bug 12) KEY3 morto** — Rimuovere il blocco KEY3 in `IRQ_RIT.c`.
12. **(bug 13) extern tipizzati** — Aggiungere `int` agli `extern` impliciti; rinominare la variabile
    globale `random` (potenziale conflitto con `random()` di stdlib) — qui semplicemente rimossa
    perché non più usata.
13. **(bug 14) Ordine timer3** — In `sample.c` invertire in `init_timer(3,...)` poi `enable_timer(3)`.
14. **(bug 15/16) Pulizia/avvio countdown** — Lasciati i file inutilizzati (rimozione opzionale);
    countdown parte al primo movimento (decisione documentata).

## 8. Stato delle correzioni

Tutte le correzioni 1–14 sopra sono state **applicate** al codice (`sample.c`, `IRQ_RIT.c`,
`IRQ_timer.c`, `GLCD.c`, `GLCD.h`). Restano da verificare in Keil/emulatore (build + run) e da
completare gli elementi di consegna non-codice.

## 9. Da fare manualmente (non codice)

- Compilare il PDF `extrapoint1`: sezione "Additional Comments" con variabili/define usate
  (es. `POWERPILL_SPAWN_PROB`, posizioni testo, passo 10px) + screenshot della configurazione emulatore.
- Registrare il video di 4 minuti con audio e sessione di debug (finestre periferiche aperte).
- Verificare la build del target **SW_Debug** in Keil e testare il gameplay nell'emulatore.
