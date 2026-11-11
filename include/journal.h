#ifndef JOURNAL_H
#define JOURNAL_H

void journal_init(const char *fichier);
void journal_ecrire(const char *fmt, ...);
void journal_arreter(void);

#endif
